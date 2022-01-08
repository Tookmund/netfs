/*
 * Copyright 2009 Rahul Murmuria <rahul@murmuria.in>
 * This file may be redistributed under the terms of the GNU GPL.
 */

#define __NO_VERSION__

#include "net.h"
/*
 * Create the files that we export.
 */

void tcp_create_files (struct super_block *sb, struct dentry *root)
{
	struct dentry *subdir;
	subdir = netfs_create_dir(sb, root, "tcp");
	if (subdir) {
		netfs_create_file(sb, subdir, "clone");
		netfs_create_file(sb, subdir, "stats");
	}
}



