/*
*
*	@Description: MCP9808 I2C sensor's linux device driver
*	@Author: BenAli Samar
*	@Date: Aug/2022
*
*/

#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/kernel.h>
#include<linux/version.h>
#include<linux/errno.h>
#include<linux/device.h>
#include<linux/mutex.h>
#include<linux/i2c.h>
#include<linux/delay.h>
#include<linux/uaccess.h>

#define DEVICE_NAME	"mcp9808"



static unsigned int mcp_major;
static unsigned int mcp_minor;

/* Sensor's data struct*/
struct mcp_data {
    unsigned char *data;
    struct i2c_client *client;
    u8 readtmp;
    struct cdev dev;
    };


	/*creating device class */
struct class *mcp_class = NULL;

	/* open methode for kernel access to the device file*/
int mcp_open(struct inode *inode,struct file *filp)
{
    struct mcp_data *mcp = NULL;

    mcp = container_of(inode->i_cdev,struct mcp_data,dev);

    if(mcp == NULL){
	pr_err("Container_of did not found valid data\n");
	return -ENODEV;
    }
    
    filp->private_data = mcp;
    mcp->data = kmalloc(2,GFP_KERNEL);
    if(mcp->data == NULL){
	pr_err("Error allocating memory\n");
	return -ENOMEM;
    }
    return 0;

}

	/* release methode for freeeing the memory after acceding to the file*/
int mcp_release(struct inode *inode,struct file *filp)
{
    struct mcp_data *mcp = filp->private_data;
    if(mcp->data != NULL){
	kfree(mcp->data);
	mcp->data = NULL;
    }
    return 0;
}


	/*Read data function*/
ssize_t mcp_read(struct file *filp,char __user *buf,size_t count,loff_t *f_pos)
{
   u8 reg_addr;
   struct i2c_msg msg[2];
   struct mcp_data *mcp_data = filp->private_data;
   u8 rawtmp[2];
   
   
	/* Temperature raw value register address */
    reg_addr = 0x05;
    msg[0].addr = mcp_data->client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &reg_addr;

    msg[1].addr = mcp_data->client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = 2;
    msg[1].buf = rawtmp;


    if(i2c_transfer(mcp_data->client->adapter,msg,2) < 0){
	printk("i2c_transfer fail \n");
    }

   	/* conversion tmp */
  mcp_data->data[0] = ((rawtmp[0]>> 4)& 0x0f);
  mcp_data->data[1] = (rawtmp[1]& 0x0f); 

  printk("Temprature value = %d%d Celsius degree\n",mcp_data->data[0],mcp_data->data[1]);

	/* copy from kernel to user space */
    if(copy_to_user(buf,mcp_data->data,2) != 0){
	pr_err("copy_to_user fail\n");
	return -EIO;
    };
    return 2;
}

   
	/* file operation */
static struct file_operations mcp_fops = {
    .owner = THIS_MODULE,
    .open = mcp_open,
    .release = mcp_release,
    .read = mcp_read,
}; 

	/*Probe function*/
static int mcp9808_probe(struct i2c_client *client, const struct i2c_device_id *id) {
 
  struct mcp_data *mcp_data;
  struct i2c_msg msg[2];
  struct device *mcp_device=NULL;
  unsigned char reg_addr = 0x06;
  u8 data[2];
  int err;
  dev_t dev_no;


	/*check functionnality*/
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
      printk( "i2c_check_functionality failed ! \n");
       return -ENODEV;
    }
printk("Functinallity checked succefully\n");  
  	
 	/* check id*/
 msg[0].addr =client->addr;
 msg[0].flags=0;
 msg[0].len =1;
 msg[0].buf =&reg_addr;
 
 msg[1].addr =client -> addr;
 msg[1].flags = I2C_M_RD;
 msg[1].len =2;
 msg[1].buf =data;
 
 if (i2c_transfer(client->adapter,msg,2)<0){
 	printk("i2c_transfer fail \n MCP9808 not found\n");
 	}
 printk("MCP9808 Manufacture ID = %x\n", data [0]<<8| data [1]);
	
    
    /* Allocate the major and minor number */
err = alloc_chrdev_region(&dev_no,0,1,DEVICE_NAME);
  if(err < 0){
  printk("alloc_chrdev_region fail\n");
  return err;
  }
  mcp_major = MAJOR(dev_no);
  mcp_minor = MINOR(dev_no);
  printk("%s major num = %d -- minor num = %d\n",DEVICE_NAME,mcp_major,mcp_minor);

    /* Create the sysfs class */
mcp_class = class_create(THIS_MODULE,DEVICE_NAME);
if(IS_ERR(mcp_class)){
err = PTR_ERR(mcp_class);
}

	/* allocating the memory for device data */
mcp_data = kzalloc(sizeof (struct mcp_data),GFP_KERNEL); 
if (mcp_data== NULL) {
	printk("Memory allocation failed! \n");
	return -ENOMEM;
	}
printk("Memory allocated succefully\n");
		
    
mcp_data->data = NULL;
mcp_data->client = client;
printk ("Starting the registration of char device \n");
    
	/* Initilize and register character device with kernel */
cdev_init(&mcp_data->dev,&mcp_fops);
mcp_data->dev.owner = THIS_MODULE;
err = cdev_add(&mcp_data->dev,dev_no,1);

    if(err){
	printk("Adding cdev fail\n");
    }
printk("Device registred \n");

	/* create the device so that user can access the slave device from user space */
mcp_device = device_create(mcp_class,
		NULL,
		dev_no,
		NULL,
		DEVICE_NAME);

    if(IS_ERR(mcp_device)) {
	err = PTR_ERR(mcp_device);
	printk("Device creation fail\n");
	cdev_del(&mcp_data->dev);
    }
    printk ("Device created succeffully\n");
    i2c_set_clientdata(client,mcp_data);
   return 0; 
return 0;
};

	/*Remove function*/
static int mcp9808_remove(struct i2c_client *client)
 {
 	struct mcp_data *mcp_data = i2c_get_clientdata(client);
 	kfree(mcp_data) ;
 	unregister_chrdev_region(MKDEV(mcp_major,mcp_minor),1);
 	printk("MCP9808's module removed successfully\n");
 	return 0;
 }
 
	/* for the device tree stuff plus the compatible one ..*/
static const struct of_device_id mcp98xx_of[] = {
    {.compatible = "microchip,mcp9808","brcm,bcm2708",},
    { },
};

static const struct i2c_device_id mcp98xx_id[] = {
    {"mcp9808",0},
    {},
};
MODULE_DEVICE_TABLE(i2c, mcp98xx_id);	

static struct i2c_driver mcp9808_driver = {
    .driver = {
	.owner = THIS_MODULE,
	.name = "mcp9808",
	.of_match_table = of_match_ptr(mcp98xx_of),

    },
    .probe = mcp9808_probe,
    .remove = mcp9808_remove,
    .id_table = mcp98xx_id,
};

module_i2c_driver(mcp9808_driver);

MODULE_AUTHOR("SAMAR_BENALI");
MODULE_LICENSE ("GPL");
 
