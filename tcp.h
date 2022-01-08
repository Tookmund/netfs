/*
 * Copyright 2022 Jacob Adams <jacob@tookmund.com>
 * Copyright 2009 Rahul Murmuria <rahul@murmuria.in>
 * This file may be redistributed under the terms of the GNU GPL.
 */

#define __NO_VERSION__

/*
 * Create a new connection folder when reading the clone file
 */
static ssize_t tcp_read_clone(struct file *filp, char *userdata,
		size_t count, loff_t *offset);

static struct file_operations tcp_clone_ops = {
	.open	= netfs_open,
	.read 	= tcp_read_clone,
};

/*
 * Read/write data from the connection
 */
static ssize_t tcp_read_data(struct file *filp, char *userdata,
		size_t count, loff_t *offset);

static ssize_t tcp_write_data(struct file *filp, const char *userdata,
		size_t count, loff_t *offset);

static struct file_operations tcp_data_ops = {
	.open	= netfs_open,
	.read 	= tcp_read_data,
	.write  = tcp_write_data,
};

/*
 * Read/write connection locaion
 */
static ssize_t tcp_read_ctl(struct file *filp, char *userdata,
		size_t count, loff_t *offset);

static ssize_t tcp_write_ctl(struct file *filp, const char *userdata,
		size_t count, loff_t *offset);

static struct file_operations tcp_ctl_ops = {
	.open	= netfs_open,
	.read 	= tcp_read_ctl,
	.write  = tcp_write_ctl,
};
