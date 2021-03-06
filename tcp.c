/*
 * Copyright 2022 Jacob Adams <jacob@tookmund.com>
 * Copyright 2009 Rahul Murmuria <rahul@murmuria.in>
 * This file may be redistributed under the terms of the GNU GPL.
 */

#define __NO_VERSION__

#include "net.h"
#include "tcp.h"

/*
 * Create a new connection folder when reading the clone file
 */
static ssize_t tcp_read_clone(struct file *filp, char *userdata,
		size_t count, loff_t *offset) {
	struct dentry *condir;
	int ret;
	struct super_block *sb = filp->f_path.mnt->mnt_sb;
	struct dentry *parent = filp->f_path.dentry->d_parent;
	char* dirname = "CLONETEST";
	condir = netfs_create_dir(sb, parent, dirname);
	if (condir)
	{
		netfs_create_file(sb, condir, "data", &tcp_data_ops);
		netfs_create_file(sb, condir, "ctl", &tcp_ctl_ops);
		ret = copy_to_user(userdata, dirname, strlen(dirname));
		return ret;
	}
	return -EIO;
}

/*
 * Read data from the connection
 */
static ssize_t tcp_read_data(struct file *filp, char *userdata,
		size_t count, loff_t *offset)
{
	return -EIO;
}

/*
 * Write data to the connection
 */
static ssize_t tcp_write_data(struct file *filp, const char *userdata,
		size_t count, loff_t *offset)
{
	return -EIO;
}

static ssize_t tcp_read_ctl(struct file *filp, char *userdata,
		size_t count, loff_t *offset)
{
	return -EIO;
}

static ssize_t tcp_write_ctl(struct file *filp, const char *userdata,
		size_t count, loff_t *offset)
{
	return -EIO;
}

void tcp_create_files (struct super_block *sb, struct dentry *root)
{
	struct dentry *subdir;
	subdir = netfs_create_dir(sb, root, "tcp");
	if (subdir) {
		netfs_create_file(sb, subdir, "clone", &tcp_clone_ops);
	}
}
