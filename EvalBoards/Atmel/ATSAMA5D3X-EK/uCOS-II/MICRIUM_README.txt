*******************************************************************************************************
*                                            uC/OS-II DEMO
*                                               on the
*                                ATMEL ATSAMA5D3X-EK Evaluation Board
*                                                 for
*                                              IAR EWARM
*******************************************************************************************************

This is an example IAR project running uCOS-II on the ATSAMA5 development board.

Supported Computer Modules (CM)

 - SAMA5D31-CM
 - SAMA5D33-CM
 - SAMA5D34-CM
 - SAMA5D35-CM


Example project features :

 - uCOS-II setup and configuration
 - uCOS-II & uC-CPU port for the Cortex A5 with VFP support
 - User LEDs
 - Basic interrupt setup and handling
 - Cortex A5 startup, MMU & caches configuration
 
 
-- Project startup and configuration --

Target initialisation is done by a C-SPY script file as distributed by IAR

A basic MMU configuration is included where most of the address space is 
mapped as device memory and external DRAM as cacheable.

