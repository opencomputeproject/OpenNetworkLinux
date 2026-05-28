// SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0
/*
 * Copyright 2024 Accton Technology Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * Description:
 *  IPMI driver related interface declarations
 */

#ifndef ACCTON_IPMI_INTF_H
#define ACCTON_IPMI_INTF_H

#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>

/* Structure to hold IPMI (Intelligent Platform Management Interface) data */
struct ipmi_data {
    struct completion read_complete;         // Synchronization primitive for signaling message read completion
    struct ipmi_addr address;                // Structure to store the IPMI system interface address
    struct ipmi_user *user;                  // Pointer to IPMI user created by the kernel
    int interface;                           // Interface identifier for the IPMI system

    struct kernel_ipmi_msg tx_message;       // Message structure for sending IPMI commands
    long tx_msgid;                           // Message ID for tracking IPMI message transactions

    void *rx_msg_data;                       // Pointer to buffer for storing received IPMI message data
    unsigned short rx_msg_len;               // Length of the received IPMI message
    unsigned char rx_result;                 // Result code from the received IPMI message
    int rx_recv_type;                        // Type of the received message (e.g., system interface, LAN, etc.)

    struct ipmi_user_hndl ipmi_hndlrs;       // IPMI handler structure for handling incoming IPMI messages
    struct device *dev;                      // Device structure for logging errors
};

/* Function declarations */

/* 
 * Initialize IPMI data structure and create a user interface for communication.
 * 
 * @param ipmi: Pointer to ipmi_data structure to be initialized.
 * @param iface: IPMI interface identifier.
 * @param dev: Device structure for logging errors.
 * @return 0 on success, or an error code on failure.
 */
extern int init_ipmi_data(struct ipmi_data *ipmi, int iface, struct device *dev);

/* 
 * Send an IPMI command to the IPMI device and receive the response.
 * 
 * @param ipmi: Pointer to ipmi_data structure containing IPMI communication information.
 * @param cmd: IPMI command byte.
 * @param tx_data: Pointer to data buffer for the command payload.
 * @param tx_len: Length of the command payload data.
 * @param rx_data: Pointer to buffer for storing the response data.
 * @param rx_len: Length of the response data buffer.
 * @return 0 on success, or an error code on failure.
 */
extern int ipmi_send_message(struct ipmi_data *ipmi, unsigned char cmd,
                             unsigned char *tx_data, unsigned short tx_len,
                             unsigned char *rx_data, unsigned short rx_len);

#endif /* ACCTON_IPMI_INTF_H */
