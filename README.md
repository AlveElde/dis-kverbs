# dis-kverbs #

This is the main implementation of RoPCIe, an RDMA transport over PCI Express. RoPCIe acts as a translation layer in between RDMA operations and PCIe Non Transparent Bridge(NTB) operations. User-space to user-space RDMA is supported through a Libibverbs plugin in [dis-uverbs](https://www.github.com/AlveElde/dis-uverbs).

The implementation consists four kernel modules: a bus, a device driver, a device and a Dolphin SCI Library Interface. When modules are built built and loaded into the kernel, a virtual device representing a RoPCIe endpoint will be registered with the RDMA subsystem in the kernel. The virtual device will behave like a physical RDMA adapter, while translating RDMA commands into NTB commands behind the scenes. 

# How to build #

## Dependencies ##
- Kernel version 5.5.x (May also work on other kernels of similar age)
- Dolphin eXpressWare for Linux

## Build ##
From project root: 
~~~sh
make
~~~

## Result ##
A successful make results in four kernel modules:
- dis_bus_mod.ko
- dis_driver_mod.ko
- dis_device_mod.ko
- dis_sci_if_mod.ko

# How to use #

## Load modules ##
***WARNING: Proceed at own risk!***

Requester side:
~~~sh
make req
~~~

Responder side:
~~~sh
make res
~~~

## Remove modules ##
Both sides:
~~~sh
make rm
~~~

# About #

This project was developed by me as a part of my master thesis, in collaboration with University of Oslo, Simula Research Laboratories, and Dolphin Interconnect Solutions. The project has achieved its goals to a large extent and is a functional proof of concept. If you choose to use any part of it, know that this code executes inside the kernel with (probably) unsafe mechanisms for directly reading and writing to physical memory. 