/* ----------------------------------------------------------------------------
 *         SAM Software Package License 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2011, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */


/** \file
 * \addtogroup usbd_cdc
 *@{
 */

/*------------------------------------------------------------------------------
 *         Headers
 *------------------------------------------------------------------------------*/

#include "board.h"
#include "include/USBD_Config.h"
#include "CDCDSerialDriver.h"

/*------------------------------------------------------------------------------
 *         Definitions
 *------------------------------------------------------------------------------*/

/** \addtogroup usbd_cdc_serial_device_ids CDC Serial Device IDs
 *      @{
 * This page lists the IDs used in the CDC Serial Device Descriptor.
 *
 * \section IDs
 * - CDCDSerialDriverDescriptors_PRODUCTID
 * - CDCDSerialDriverDescriptors_VENDORID
 * - CDCDSerialDriverDescriptors_RELEASE
 */

/** Device product ID. */
#define CDCDSerialDriverDescriptors_PRODUCTID       0x9700
/** Device vendor ID (Atmel). */
#define CDCDSerialDriverDescriptors_VENDORID        0x03EB
/** Device release number. */
#define CDCDSerialDriverDescriptors_RELEASE         0x0100
/**      @}*/

/*------------------------------------------------------------------------------
 *         Macros
 *------------------------------------------------------------------------------*/

/** Returns the minimum between two values. */
#define MIN(a, b)       ((a < b) ? a : b)

/*------------------------------------------------------------------------------
 *         Exported variables
 *------------------------------------------------------------------------------*/

/** Standard USB device descriptor for the CDC serial driver */
const USBDeviceDescriptor deviceDescriptor = {

    sizeof(USBDeviceDescriptor),
    USBGenericDescriptor_DEVICE,                     //1
    USBDeviceDescriptor_USB2_00,                     //0x200
    CDCDeviceDescriptor_CLASS,                       //2
    CDCDeviceDescriptor_SUBCLASS,                    //0
    CDCDeviceDescriptor_PROTOCOL,                    //0
    CHIP_USB_ENDPOINTS_MAXPACKETSIZE(0),             //64
    CDCDSerialDriverDescriptors_VENDORID,
    CDCDSerialDriverDescriptors_PRODUCTID,
    CDCDSerialDriverDescriptors_RELEASE,
    1, // No string descriptor for manufacturer  ----changed form 0-->1
    2, // Index of product string descriptor is #1  --->changed form 1-->2
    3, // No string descriptor for serial number ------>change from 1 -->3 
    1 // Device has 1 possible configuration  
};

//* Standard USB configuration descriptor for the CDC serial driver  
const CDCDSerialDriverConfigurationDescriptors configurationDescriptorsFS = {

    // Standard configuration descriptor  
    {
        sizeof(USBConfigurationDescriptor),
        USBGenericDescriptor_CONFIGURATION,
        sizeof(CDCDSerialDriverConfigurationDescriptors),
        1, // There are two interfaces in this configuration  --reduce interface by leo
        1, // This is configuration #1  
        0, // No string descriptor for this configuration  
        USBD_BMATTRIBUTES,
        USBConfigurationDescriptor_POWER(100)
    },
#ifndef CUSTOM_USB
    // Communication class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #0  
        0, // This is alternate setting #0 for this interface  
        1, // This interface uses 1 endpoint  
        CDCCommunicationInterfaceDescriptor_CLASS,
        CDCCommunicationInterfaceDescriptor_ABSTRACTCONTROLMODEL,
        CDCCommunicationInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Class-specific header functional descriptor  
    {
        sizeof(CDCHeaderDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_HEADER,
        CDCGenericDescriptor_CDC1_10
    },
    // Class-specific call management functional descriptor  
    {
        sizeof(CDCCallManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_CALLMANAGEMENT,
        CDCCallManagementDescriptor_SELFCALLMANAGEMENT,
        0 // No associated data interface  
    },
    // Class-specific abstract control management functional descriptor  
    {
        sizeof(CDCAbstractControlManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_ABSTRACTCONTROLMANAGEMENT,
        CDCAbstractControlManagementDescriptor_LINE
    },
    // Class-specific union functional descriptor with one slave interface  
    {
        sizeof(CDCUnionDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_UNION,
        0, // Number of master interface is #0  
        1 // First slave interface is #1  
    },
    // Notification endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_NOTIFICATION),
        USBEndpointDescriptor_INTERRUPT,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_NOTIFICATION),
            USBEndpointDescriptor_MAXINTERRUPTSIZE_FS),
        10 // Endpoint is polled every 10ms  
    },
#endif
    // Data class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #1  ----changed from 1--->0 by leo after reducing interface
        0, // This is alternate setting #0 for this interface  
        6, // This interface uses 2 endpoints  ----uif will use 6 endpoints,so changed from 2 -->6
        0xFF, //CDCDataInterfaceDescriptor_CLASS,
        0xFF, //CDCDataInterfaceDescriptor_SUBCLASS,
        0xFF, //CDCDataInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT ),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_FS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_FS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
        // Bulk-OUT CMD endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_CMDDATAOUT ),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_FS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN CMD endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_CMDDATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_FS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT1 ),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT1),
            USBEndpointDescriptor_MAXBULKSIZE_FS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN1),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN1),
            USBEndpointDescriptor_MAXBULKSIZE_FS),
        0 // Must be 0 for full-speed bulk endpoints  
    }
    
};

/// Other-speed configuration descriptor (when in full-speed).  
const CDCDSerialDriverConfigurationDescriptors otherSpeedDescriptorsFS = {

    // Standard configuration descriptor  
    {
        sizeof(USBConfigurationDescriptor),
        USBGenericDescriptor_OTHERSPEEDCONFIGURATION,
        sizeof(CDCDSerialDriverConfigurationDescriptors),
        1, // There are two interfaces in this configuration  
        1, // This is configuration #1  
        0, // No string descriptor for this configuration  
        BOARD_USB_BMATTRIBUTES,
        USBConfigurationDescriptor_POWER(100)
    },
#ifndef CUSTOM_USB
    // Communication class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #0  
        0, // This is alternate setting #0 for this interface  
        1, // This interface uses 1 endpoint  
        CDCCommunicationInterfaceDescriptor_CLASS,
        CDCCommunicationInterfaceDescriptor_ABSTRACTCONTROLMODEL,
        CDCCommunicationInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Class-specific header functional descriptor  
    {
        sizeof(CDCHeaderDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_HEADER,
        CDCGenericDescriptor_CDC1_10
    },
    // Class-specific call management functional descriptor  
    {
        sizeof(CDCCallManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_CALLMANAGEMENT,
        CDCCallManagementDescriptor_SELFCALLMANAGEMENT,
        0 // No associated data interface  
    },
    // Class-specific abstract control management functional descriptor  
    {
        sizeof(CDCAbstractControlManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_ABSTRACTCONTROLMANAGEMENT,
        CDCAbstractControlManagementDescriptor_LINE
    },
    // Class-specific union functional descriptor with one slave interface  
    {
        sizeof(CDCUnionDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_UNION,
        0, // Number of master interface is #0  
        1 // First slave interface is #1  
    },
    // Notification endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_NOTIFICATION),
        USBEndpointDescriptor_INTERRUPT,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_NOTIFICATION),
            USBEndpointDescriptor_MAXINTERRUPTSIZE_FS),
        8 // Endpoint is polled every 16ms  
    },
#endif
    // Data class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #1  
        0, // This is alternate setting #0 for this interface  
        6, // This interface uses 2 endpoints  
        0xFF, //CDCDataInterfaceDescriptor_CLASS,
        0xFF, //CDCDataInterfaceDescriptor_SUBCLASS,
        0xFF, //CDCDataInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    /*---------------------------------add custom endpoint--------------------*/
     // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_CMDDATAOUT),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_CMDDATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT1),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT1),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN1),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN1),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
};

/// Configuration descriptor (when in high-speed).  
const CDCDSerialDriverConfigurationDescriptors configurationDescriptorsHS = {

    // Standard configuration descriptor  
    {
        sizeof(USBConfigurationDescriptor),
        USBGenericDescriptor_CONFIGURATION,
        sizeof(CDCDSerialDriverConfigurationDescriptors),
        1, // There are two interfaces in this configuration  
        1, // This is configuration #1  
        0, // No string descriptor for this configuration  
        BOARD_USB_BMATTRIBUTES,
        USBConfigurationDescriptor_POWER(100)
    },
#ifndef CUSTOM_USB
    // Communication class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #0  
        0, // This is alternate setting #0 for this interface  
        1, // This interface uses 1 endpoint  
        CDCCommunicationInterfaceDescriptor_CLASS,
        CDCCommunicationInterfaceDescriptor_ABSTRACTCONTROLMODEL,
        CDCCommunicationInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Class-specific header functional descriptor  
    {
        sizeof(CDCHeaderDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_HEADER,
        CDCGenericDescriptor_CDC1_10
    },
    // Class-specific call management functional descriptor  
    {
        sizeof(CDCCallManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_CALLMANAGEMENT,
        CDCCallManagementDescriptor_SELFCALLMANAGEMENT,
        0 // No associated data interface  
    },
    // Class-specific abstract control management functional descriptor  
    {
        sizeof(CDCAbstractControlManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_ABSTRACTCONTROLMANAGEMENT,
        CDCAbstractControlManagementDescriptor_LINE
    },
    // Class-specific union functional descriptor with one slave interface  
    {
        sizeof(CDCUnionDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_UNION,
        0, // Number of master interface is #0  
        1 // First slave interface is #1  
    },
    // Notification endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_NOTIFICATION),
        USBEndpointDescriptor_INTERRUPT,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_NOTIFICATION),
            USBEndpointDescriptor_MAXINTERRUPTSIZE_FS),
        8  // Endpoint is polled every 16ms  
    },
#endif
    // Data class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #1  
        0, // This is alternate setting #0 for this interface  
        6, // This interface uses 2 endpoints  
        0xFF, //CDCDataInterfaceDescriptor_CLASS,
        0xFF, //CDCDataInterfaceDescriptor_SUBCLASS,
        0xFF, //CDCDataInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_HS),//USBEndpointDescriptor_MAXBULKSIZE_HS
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_HS),//USBEndpointDescriptor_MAXBULKSIZE_HS
        0 // Must be 0 for full-speed bulk endpoints  
    },
    
    /*---------------------------add custom endpoint--------------------------*/
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_CMDDATAOUT),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_CMDDATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT1),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT1),
            USBEndpointDescriptor_MAXBULKSIZE_HS),//USBEndpointDescriptor_MAXBULKSIZE_HS
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN1),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN1),
            USBEndpointDescriptor_MAXBULKSIZE_HS),//USBEndpointDescriptor_MAXBULKSIZE_HS
        0 // Must be 0 for full-speed bulk endpoints  
    }    
};

/// Other-speed configuration descriptor (when in high-speed).  
const CDCDSerialDriverConfigurationDescriptors otherSpeedDescriptorsHS = {

    // Standard configuration descriptor  
    {
        sizeof(USBConfigurationDescriptor),
        USBGenericDescriptor_OTHERSPEEDCONFIGURATION,
        sizeof(CDCDSerialDriverConfigurationDescriptors),
        1, // There are two interfaces in this configuration  
        1, // This is configuration #1  
        0, // No string descriptor for this configuration  
        BOARD_USB_BMATTRIBUTES,
        USBConfigurationDescriptor_POWER(100)
    },
#ifndef CUSTOM_USB
    // Communication class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #0  
        0, // This is alternate setting #0 for this interface  
        1, // This interface uses 1 endpoint  
        CDCCommunicationInterfaceDescriptor_CLASS,
        CDCCommunicationInterfaceDescriptor_ABSTRACTCONTROLMODEL,
        CDCCommunicationInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Class-specific header functional descriptor  
    {
        sizeof(CDCHeaderDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_HEADER,
        CDCGenericDescriptor_CDC1_10
    },
    // Class-specific call management functional descriptor  
    {
        sizeof(CDCCallManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_CALLMANAGEMENT,
        CDCCallManagementDescriptor_SELFCALLMANAGEMENT,
        0 // No associated data interface  
    },
    // Class-specific abstract control management functional descriptor  
    {
        sizeof(CDCAbstractControlManagementDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_ABSTRACTCONTROLMANAGEMENT,
        CDCAbstractControlManagementDescriptor_LINE
    },
    // Class-specific union functional descriptor with one slave interface  
    {
        sizeof(CDCUnionDescriptor),
        CDCGenericDescriptor_INTERFACE,
        CDCGenericDescriptor_UNION,
        0, // Number of master interface is #0  
        1 // First slave interface is #1  
    },
    // Notification endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_NOTIFICATION),
        USBEndpointDescriptor_INTERRUPT,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_NOTIFICATION),
            USBEndpointDescriptor_MAXINTERRUPTSIZE_FS),
        10 // Endpoint is polled every 10ms  
    },
#endif
    // Data class interface standard descriptor  
    {
        sizeof(USBInterfaceDescriptor),
        USBGenericDescriptor_INTERFACE,
        0, // This is interface #1  
        0, // This is alternate setting #0 for this interface  
        6, // This interface uses 2 endpoints  
        0xFF, //CDCDataInterfaceDescriptor_CLASS,
        0xFF, //CDCDataInterfaceDescriptor_SUBCLASS,
        0xFF, //CDCDataInterfaceDescriptor_NOPROTOCOL,
        0  // No string descriptor for this interface  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    /*---------------------------add custom endpoint--------------------------*/
        // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_CMDDATAOUT),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAOUT),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_CMDDATAIN),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_CMDDATAIN),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-OUT endpoint standard descriptor  
    {
        sizeof(USBEndpointDescriptor), 
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_OUT,
                                      CDCDSerialDriverDescriptors_AUDIODATAOUT1),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAOUT1),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    },
    // Bulk-IN endpoint descriptor  
    {
        sizeof(USBEndpointDescriptor),
        USBGenericDescriptor_ENDPOINT,
        USBEndpointDescriptor_ADDRESS(USBEndpointDescriptor_IN,
                                      CDCDSerialDriverDescriptors_AUDIODATAIN1),
        USBEndpointDescriptor_BULK,
        MIN(CHIP_USB_ENDPOINTS_MAXPACKETSIZE(CDCDSerialDriverDescriptors_AUDIODATAIN1),
            USBEndpointDescriptor_MAXBULKSIZE_HS),
        0 // Must be 0 for full-speed bulk endpoints  
    }    
};

/// Language ID string descriptor  
const unsigned char languageIdStringDescriptor[] = {

    USBStringDescriptor_LENGTH(1),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_ENGLISH_US
};

/// Manufacturer name.
const unsigned char manufacturerDescriptor[] = {

    USBStringDescriptor_LENGTH(15),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_UNICODE('F'),
    USBStringDescriptor_UNICODE('o'),
    USBStringDescriptor_UNICODE('r'),
    USBStringDescriptor_UNICODE('t'),
    USBStringDescriptor_UNICODE('e'),
    USBStringDescriptor_UNICODE('m'),
    USBStringDescriptor_UNICODE('e'),
    USBStringDescriptor_UNICODE('d'),
    USBStringDescriptor_UNICODE('i'),
    USBStringDescriptor_UNICODE('a'),
    USBStringDescriptor_UNICODE(' '),  
    USBStringDescriptor_UNICODE('I'),
    USBStringDescriptor_UNICODE('n'),
    USBStringDescriptor_UNICODE('c'),
    USBStringDescriptor_UNICODE('.')
};


/// Product string descriptor  
const unsigned char productStringDescriptor[] = {

    USBStringDescriptor_LENGTH(10),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_UNICODE('A'),
    USBStringDescriptor_UNICODE('B'),
    USBStringDescriptor_UNICODE('0'),
    USBStringDescriptor_UNICODE('4'),    
    USBStringDescriptor_UNICODE(' '),
    USBStringDescriptor_UNICODE('B'),
    USBStringDescriptor_UNICODE('o'),
    USBStringDescriptor_UNICODE('a'),
    USBStringDescriptor_UNICODE('r'),
    USBStringDescriptor_UNICODE('d'),
};

// Product serial number.
const unsigned char serialNumberDescriptor[] = {

    USBStringDescriptor_LENGTH(16),
    USBGenericDescriptor_STRING,
    USBStringDescriptor_UNICODE('+'),
    USBStringDescriptor_UNICODE('8'),
    USBStringDescriptor_UNICODE('6'),
    USBStringDescriptor_UNICODE('-'),
    USBStringDescriptor_UNICODE('0'),
    USBStringDescriptor_UNICODE('2'),
    USBStringDescriptor_UNICODE('5'),
    USBStringDescriptor_UNICODE('-'),
    USBStringDescriptor_UNICODE('8'),
    USBStringDescriptor_UNICODE('3'),
    USBStringDescriptor_UNICODE('1'),
    USBStringDescriptor_UNICODE('9'),
    USBStringDescriptor_UNICODE('9'),
    USBStringDescriptor_UNICODE('9'),
    USBStringDescriptor_UNICODE('9'),
    USBStringDescriptor_UNICODE('7')
};

// MS OS Descriptor for Win8.1
const unsigned char OSStringDescriptor[] = {

    0x12,
    USBGenericDescriptor_STRING,
    USBStringDescriptor_UNICODE('M'),
    USBStringDescriptor_UNICODE('S'),
    USBStringDescriptor_UNICODE('F'),
    USBStringDescriptor_UNICODE('T'),
    USBStringDescriptor_UNICODE('1'),
    USBStringDescriptor_UNICODE('0'),
    USBStringDescriptor_UNICODE('0'),   
    USBGenericRequest_GET_MS_DESCRIPTOR, //bMS_VendorCode
    0
};


// MS OS Extended Compat ID OS Feature Descriptor
const unsigned char OSExCompIDDescriptor[] = {

    0x28,0x00,0x00,0x00,  //length
    0x00, 0x01,//0x0100,  //bcd Version  1.0
    0x04, 0x00,//0x0004,  //Extened compat ID descriptor
    0x01,      //number of function section
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //reserved
    
    0x00,  //interface number
    0x01,  // reserved  
    ('W'), //compatible ID
    ('I'),
    ('N'),   
    ('U'),
    ('S'),
    ('B'), 
    0,     //pad
    0,     //pad
    
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //Sub compatible ID
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00               //reserved
    
};


// MS OS Extended Property Descriptor
const unsigned char OSExPropertyDescriptor[] = {

    0x8E,0x00,0x00,0x00,  //length of descriptor
    0x00, 0x01,//0x0100,  //bcd Version  1.0
    0x05, 0x00,//0x0005,  //Extened property descriptor
    0x01, 0x00,//number of customer propertity section
       
    0x84,0x00,0x00,0x00,  //length of this property
    0x01,0x00,0x00,0x00,
    0x28,0x00, //Length of the property name string is 40 bytes.
    
    //Property name is ��DeviceInterfaceGUID��.
    0x44, 0x00 ,0x65 ,0x00 ,0x76 ,0x00 ,0x69 ,0x00 ,0x63 ,0x00 ,
    0x65 ,0x00 ,0x49 ,0x00 ,0x6E ,0x00 ,0x74 ,0x00 ,0x65 ,0x00 ,
    0x72 ,0x00 ,0x66 ,0x00 ,0x61 ,0x00 ,0x63 ,0x00 ,0x65 ,0x00 ,
    0x47 ,0x00 ,0x55 ,0x00 ,0x49 ,0x00 ,0x44 ,0x00 ,0x00 ,0x00 , 
        
    0x4E ,0x00,0x00,0x00, //Length of the property value string is 78 bytes.
    //"{cc22e4b4-7985-426a-87ea-6ee58f202136}"
    USBStringDescriptor_UNICODE('{'),
    USBStringDescriptor_UNICODE('c'),
    USBStringDescriptor_UNICODE('c'),
    USBStringDescriptor_UNICODE('2'),
    USBStringDescriptor_UNICODE('2'),
    USBStringDescriptor_UNICODE('e'),
    USBStringDescriptor_UNICODE('4'),
    USBStringDescriptor_UNICODE('b'),
    USBStringDescriptor_UNICODE('4'),
    USBStringDescriptor_UNICODE('-'),
    USBStringDescriptor_UNICODE('7'),
    USBStringDescriptor_UNICODE('9'),
    USBStringDescriptor_UNICODE('8'),
    USBStringDescriptor_UNICODE('5'),
    USBStringDescriptor_UNICODE('-'),
    USBStringDescriptor_UNICODE('4'),
    USBStringDescriptor_UNICODE('2'),
    USBStringDescriptor_UNICODE('6'),
    USBStringDescriptor_UNICODE('a'),
    USBStringDescriptor_UNICODE('-'),
    USBStringDescriptor_UNICODE('8'),
    USBStringDescriptor_UNICODE('7'),
    USBStringDescriptor_UNICODE('e'),
    USBStringDescriptor_UNICODE('a'),
    USBStringDescriptor_UNICODE('-'),
    USBStringDescriptor_UNICODE('6'),
    USBStringDescriptor_UNICODE('e'),
    USBStringDescriptor_UNICODE('e'),
    USBStringDescriptor_UNICODE('5'),
    USBStringDescriptor_UNICODE('8'),
    USBStringDescriptor_UNICODE('f'),
    USBStringDescriptor_UNICODE('2'),
    USBStringDescriptor_UNICODE('0'),
    USBStringDescriptor_UNICODE('2'),
    USBStringDescriptor_UNICODE('1'),
    USBStringDescriptor_UNICODE('3'),
    USBStringDescriptor_UNICODE('6'),
    USBStringDescriptor_UNICODE('}'),
    0, //added 2 more 0 in case of the issue of messy code after GUID in registry: 
    0 
    
};



/// List of string descriptors used by the device  
const unsigned char *stringDescriptors[] = {
    
    languageIdStringDescriptor,
    manufacturerDescriptor,
    productStringDescriptor,
    serialNumberDescriptor,
    OSStringDescriptor,
    OSExCompIDDescriptor,
    OSExPropertyDescriptor
 };

/// List of standard descriptors for the serial driver.  
const USBDDriverDescriptors cdcdSerialDriverDescriptors = {

    &deviceDescriptor,
    (USBConfigurationDescriptor *) &(configurationDescriptorsFS),
    0, // No full-speed device qualifier descriptor  
    (USBConfigurationDescriptor *) &(otherSpeedDescriptorsFS),
    0, // No high-speed device descriptor (uses FS one)  
    (USBConfigurationDescriptor *) &(configurationDescriptorsHS),
    0, // No high-speed device qualifier descriptor  
    (USBConfigurationDescriptor *) &(otherSpeedDescriptorsHS),
    stringDescriptors,
    6 // 6 string descriptors in list  
};

/**@} */
