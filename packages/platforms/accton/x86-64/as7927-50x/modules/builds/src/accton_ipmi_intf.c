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
 *  IPMI driver related interface implementation
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string_helpers.h>
#include "accton_ipmi_intf.h"

#define ACCTON_IPMI_NETFN 0x34
#define IPMI_TIMEOUT (5 * HZ)
#define IPMI_ERR_RETRY_TIMES 1
#define RAW_CMD_BUF_SIZE 32

static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data);

/* Functions to talk to the IPMI layer */

/* Initialize IPMI data structure and create a user interface for communication */
int init_ipmi_data(struct ipmi_data *ipmi, int iface, struct device *dev)
{
	int err;

	if (!ipmi || !dev)
		return -EINVAL;

	init_completion(&ipmi->read_complete);

	// Initialize IPMI address
	ipmi->address.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
	ipmi->address.channel = IPMI_BMC_CHANNEL;
	ipmi->address.data[0] = 0;
	ipmi->interface = iface;
	ipmi->dev = dev;	// Storing the device for future reference

	// Initialize message buffers
	ipmi->tx_msgid = 0;
	ipmi->tx_message.netfn = ACCTON_IPMI_NETFN;

	// Assign the message handler
	ipmi->ipmi_hndlrs.ipmi_recv_hndl = ipmi_msg_handler;

	// Create IPMI messaging interface user
	err = ipmi_create_user(ipmi->interface, &ipmi->ipmi_hndlrs,
			       ipmi, &ipmi->user);
	if (err < 0) {
		dev_err(dev,
			"Unable to register user with IPMI interface %d, err: %d\n",
			ipmi->interface, err);
		return err;
	}

	return 0;
}
EXPORT_SYMBOL(init_ipmi_data);

/* Handler function for receiving IPMI messages */
static void ipmi_msg_handler(struct ipmi_recv_msg *msg, void *user_msg_data)
{
	unsigned short rx_len;
	struct ipmi_data *ipmi = user_msg_data;

	// Check for message ID mismatch
	if (msg->msgid != ipmi->tx_msgid) {
		dev_err(ipmi->dev, "Mismatch between received msgid "
			"(%02x) and transmitted msgid (%02x)!\n",
			(int)msg->msgid, (int)ipmi->tx_msgid);
		ipmi_free_recv_msg(msg);
		return;
	}

	// Handle received message type
	ipmi->rx_recv_type = msg->recv_type;

	// Parse message data
	if (msg->msg.data_len > 0)
		ipmi->rx_result = msg->msg.data[0];
	else
		ipmi->rx_result = IPMI_UNKNOWN_ERR_COMPLETION_CODE;

	// Copy remaining message data if available
	if (msg->msg.data_len > 1) {
		rx_len = msg->msg.data_len - 1;
		if (ipmi->rx_msg_len < rx_len)
			rx_len = ipmi->rx_msg_len;

		ipmi->rx_msg_len = rx_len;
		memcpy(ipmi->rx_msg_data, msg->msg.data + 1, ipmi->rx_msg_len);
	} else {
		ipmi->rx_msg_len = 0;
	}

	// Free the received message and signal completion
	ipmi_free_recv_msg(msg);
	complete(&ipmi->read_complete);
}

static void _ipmi_log_error(struct ipmi_data *ipmi, unsigned char cmd,
			    unsigned char *tx_data, unsigned short tx_len,
			    int status, int retry)
{
	int i, pos;
	char *cmdline = NULL;
	char raw_cmd[RAW_CMD_BUF_SIZE] = { 0 };

	// Format the command and data into a raw command string
	pos = snprintf(raw_cmd, sizeof(raw_cmd), "0x%02x", cmd);
	for (i = 0; i < tx_len && pos < sizeof(raw_cmd); i++) {
		pos += snprintf(raw_cmd + pos, sizeof(raw_cmd) - pos,
				     " 0x%02x", tx_data[i]);
	}

	// Log the error message
	cmdline = kstrdup_quotable_cmdline(current, GFP_KERNEL);
	dev_err(ipmi->dev,
		"ipmi_send_message: retry(%d), error(%d), cmd(%s) raw_cmd=[%s]\r\n",
		retry, status, cmdline ? cmdline : "", raw_cmd);

	if (cmdline) {
		kfree(cmdline);
	}
}

/* Send an IPMI command */
static int _ipmi_send_message(struct ipmi_data *ipmi, unsigned char cmd,
			      unsigned char *tx_data, unsigned short tx_len,
			      unsigned char *rx_data, unsigned short rx_len)
{
	int err;

	// Validate the input parameters
	if ((tx_len && !tx_data) || (rx_len && !rx_data)) {
		return -EINVAL;
	}

	// Initialize IPMI message
	ipmi->tx_message.cmd = cmd;
	ipmi->tx_message.data = tx_len ? tx_data : NULL;
	ipmi->tx_message.data_len = tx_len;
	ipmi->rx_msg_data = rx_len ? rx_data : NULL;
	ipmi->rx_msg_len = rx_len;

	// Validate the IPMI address
	err = ipmi_validate_addr(&ipmi->address, sizeof(ipmi->address));
	if (err) {
		dev_err(ipmi->dev, "Invalid IPMI address: %x\n", err);
		return err;
	}

	// Increment message ID and send the request
	ipmi->tx_msgid++;
	err = ipmi_request_settime(ipmi->user, &ipmi->address, ipmi->tx_msgid,
				   &ipmi->tx_message, ipmi, 0, 0, 0);
	if (err) {
		dev_err(ipmi->dev, "IPMI request_settime failed: %x\n", err);
		return err;
	}

	// Wait for the message to complete
	err = wait_for_completion_timeout(&ipmi->read_complete, IPMI_TIMEOUT);
	if (!err) {
		dev_err(ipmi->dev, "IPMI command timeout\n");
		return -ETIMEDOUT;
	}

	return 0;
}

/* Send an IPMI command to the IPMI device and receive the response */
int ipmi_send_message(struct ipmi_data *ipmi, unsigned char cmd,
		      unsigned char *tx_data, unsigned short tx_len,
		      unsigned char *rx_data, unsigned short rx_len)
{
	int status = 0, retry = 0;

	// Validate the input parameters
	if ((tx_len && !tx_data) || (rx_len && !rx_data)) {
		return -EINVAL;
	}

	for (retry = 0; retry <= IPMI_ERR_RETRY_TIMES; retry++) {
		status = _ipmi_send_message(ipmi, cmd, tx_data, tx_len, rx_data, rx_len);
		if (unlikely(status != 0)) {
			_ipmi_log_error(ipmi, cmd, tx_data, tx_len, status, retry);
			continue;
		}

		if (unlikely(ipmi->rx_result != 0)) {
			_ipmi_log_error(ipmi, cmd, tx_data, tx_len, status, retry);
			continue;
		}

		// Success, exit the retry loop
		break;
	}

	return status;
}

EXPORT_SYMBOL(ipmi_send_message);

static int __init ipmi_module_init(void)
{
	printk(KERN_INFO "Accton IPMI Module loaded\n");
	return 0;
}

static void __exit ipmi_module_exit(void)
{
	printk(KERN_INFO "Accton IPMI Module unloaded\n");
}

module_init(ipmi_module_init);
module_exit(ipmi_module_exit);

MODULE_AUTHOR("Brandon Chuang <brandon_chuang@accton.com.tw>");
MODULE_DESCRIPTION("Accton IPMI messaging module");
MODULE_LICENSE("GPL");
