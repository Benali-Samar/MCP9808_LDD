
# Overview

A custom character linux device driver for the  I2C temperatur sensor "MCP9808".

The MCP9808 sensor is from the MCPXX sensors family that is included as a tristate modules in the linux kernel "SENSORS_JC42", for more details see:"https://elixir.bootlin.com/linux/latest/source/drivers/hwmon/Kconfig" but the sensor itself doesn't have its own driver.

So in this repository we'll be discusting the I2C protocol, I2C susbsystem in linux, the I2C client driver and the main structs used in this driver and finally how to compile and load it manually.

# What is I2C ?

The Inter-Integrated Circuit (I2C) is so simple, master slave protocol. 


![alt_text](https://ai.thestempedia.com/wp-content/uploads/2022/08/I2C-Communication-How-It-Works.png)



I2C is a half-duplex bidirectional synchronous serial bus, where several equipment, masters or slaves, can be connected to the bus.

Exchanges always take place between a single master and one (or all) slave(s), always at the initiative of the master (never from master to master or from slave to slave). However, nothing prevents a component from passing from the status of master to slave and vice versa.

The connection is made by means of two lines:

    SDA (Serial Data Line): bidirectional data line,
    SCL (Serial Clock Line): bidirectional synchronization clock line.

# What is the I2C linux subsystem ?

The Linux kernel implements a number of important architectural attributes. At a high level, and a lower levels, the kernel is layered into a number of distinct subsystems.

Each subsystem typically focuses on a distinct area of system management. Here are a few common Linux subsystems:

    Core subsystems ( memeory management, scheduler, timers, ...)
    Human interfaces (HID, sound subsystem, ...)
    Networking interfaces (networking)
    Storage interfaces (file systems, I2C, IIO, 1-wire, hwmon, usb, pci, ...)
    
The I2C subsystem in Linux provides support for the I2C bus protocol, allowing Linux to interact with I2C devices. 

![alt text](https://anantobc.files.wordpress.com/2017/03/linux_i2c_subsystem.png?w=551&h=441)

The I2C core is responsible for low-level I2C bus communication, the I2C client represents individual devices on the bus, and the I2C dev interface provides user-space access to these devices through device files. Together, they form the framework for managing and interacting with I2C devices in the Linux kernel.

For more details : "https://docs.kernel.org/i2c/index.html"

# The MCP9808 client driver

The MCP9808's I2C client driver is a character driver that will sends its data under /dev/* 

Common structs used in this driver:

    - struct i2c_client (for the i2c slave device )
    - struct cdev (for the char driver routines)
    - struct i2c_driver (for the i2c device driver itself)
    - struct for the driver's data (driver's data)
    - open/release methodes (for the kernel access to the device file)
    - probe/remove functions (the entry point in kernel, unloading module function)
    - read/write functions (for data read/write)

# Compile and load

The MCP908's driver is a kernel module that can be compiled with the makefile present in this repository, but keep in mind to give it the correct kernel headers version.

To compile just type "make"

This will give you a ".ko" file you can load/unload it manually by:

    $ sudo insmod mcp9808.ko 
    $ sudo rmmod mcp9808.ko 

You can see the kernel logs with "sudo dmesg"



For further prospects, the driver can be implemented with the "iio subsystem" to enhance performance and data speed transfer.
