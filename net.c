/*
 * Copyright 2009 Rahul Murmuria <rahul@murmuria.in>
 * This file may be redistributed under the terms of the GNU GPL.
 * Most of this program has been adapted from the design of lwnfs
 * from http://lwn.net which is a sample implementation over libfs.
 */

#include "net.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jacob Adams <jacob@tookmund.com>");
MODULE_AUTHOR("Rahul Murmuria <rahul@murmuria.in>");
MODULE_AUTHOR("Jonathan Corbet <corbet@lwn.net>");

inline unsigned int blksize_bits(unsigned int size)
{
    unsigned int bits = 8;
    do {
        bits++;
        size >>= 1;
    } while (size > 256);
    return bits;
}

static struct inode *netfs_make_inode(struct super_block *sb, int mode)
{
	struct inode *ret = new_inode(sb);

	if (ret) {
		ret->i_mode = mode;
		ret->i_uid = KUIDT_INIT(0);
		ret->i_gid = KGIDT_INIT(0);
		ret->i_blkbits = blksize_bits(PAGE_SIZE);
		ret->i_blocks = 0;
		ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
	}
	return ret;
}


/*
 * The operations on our "files".
 */

/*
 * Open a file.
 */
static int netfs_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * Read a file.
 */
static ssize_t netfs_read_file(struct file *filp, char *dnsquery,
		size_t count, loff_t *offset)
{
	char *buffer = (char *)(filp->f_inode->i_private);
	int len, retval;
	len = strlen(buffer);
	if (*offset > len)
		return 0;
	if (count > len - *offset)
		count = len - *offset;

	retval = copy_to_user(dnsquery, buffer + *offset, count);

	if (retval)
		return -EFAULT;

	*offset += count;
	return count;
}

/*
 * Write a file.
 */
static ssize_t netfs_write_file(struct file *filp, const char *dnsquery,
		size_t count, loff_t *offset)
{
	char tmp[TMPSIZE];
	char *buffer = (char *)(filp->f_inode->i_private);

	if (*offset != 0)
		return -EINVAL;

	memset(tmp, 0, TMPSIZE);
	if(copy_from_user(tmp, dnsquery, count))
		return -EFAULT;
	tmp[count-1] = '\0';
	memcpy (buffer, tmp, count);
	return count;
}


/*
 * Now we can put together our file operations structure.
 */
static struct file_operations netfs_file_ops = {
	.open	= netfs_open,
	.read 	= netfs_read_file,
	.write  = netfs_write_file,
};


/*
 * Create a file.
 */
struct dentry *netfs_create_file (struct super_block *sb,
		struct dentry *dir, const char *name, struct file_operations *fops)
{
	struct dentry *dentry;
	struct inode *inode;
	struct qstr qname = QSTR_INIT(name, strlen(name));
/*
 * Make a hashed version of the name to go with the dentry.
 * Must include the salt of the parent directory dentry to work correctly
 */
	qname.hash = full_name_hash(dir, qname.name, qname.len);
/*
 * Now we can create our dentry and the inode to go with it.
 */
	dentry = d_alloc(dir, &qname);
	if (! dentry)
		goto out;
	inode = netfs_make_inode(sb, S_IFREG | 0644);
	if (! inode)
		goto out_dput;
	if (!fops)
	{
		inode->i_fop = &netfs_file_ops;
		inode->i_private = kmalloc(TMPSIZE, GFP_KERNEL);
		memset(inode->i_private, 0, TMPSIZE);
		memcpy(inode->i_private, qname.name, qname.len);
	}
	else inode->i_fop = fops;


/*
 * Put it all into the dentry cache and we're done.
 */
	d_add(dentry, inode);
	return dentry;
/*
 * Then again, maybe it didn't work.
 */
  out_dput:
	dput(dentry);
  out:
	return 0;
}


/*
 * Create a directory which can be used to hold files.  This code is
 * almost identical to the "create file" logic, except that we create
 * the inode with a different mode, and use the libfs "simple" operations.
 */
struct dentry *netfs_create_dir (struct super_block *sb,
		struct dentry *parent, const char *name)
{
	struct dentry *dentry;
	struct inode *inode;
	struct qstr qname = QSTR_INIT(name, strlen(name));
	qname.hash = full_name_hash(parent, qname.name, qname.len);
	dentry = d_alloc(parent, &qname);
	if (! dentry)
		goto out;

	inode = netfs_make_inode(sb, S_IFDIR | 0644);
	if (! inode)
		goto out_dput;
	inode->i_op = &simple_dir_inode_operations;
	inode->i_fop = &simple_dir_operations;

	d_add(dentry, inode);
	return dentry;

  out_dput:
	dput(dentry);
  out:
	return 0;
}



/*
 * Create the files that we export.
 */

static void netfs_create_files (struct super_block *sb, struct dentry *root)
{
	tcp_create_files (sb, root);
}




/*
 * Superblock stuff.  This is all boilerplate to give the vfs something
 * that looks like a filesystem to work with.
 */

/*
 * Our superblock operations, both of which are generic kernel ops
 * that we don't have to write ourselves.
 */
static struct super_operations netfs_s_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
};

/*
 * "Fill" a superblock with mundane stuff.
 */
static int netfs_fill_super (struct super_block *sb, void *data, int silent)
{
	struct inode *root;
	struct dentry *root_dentry;
/*
 * Basic parameters.
 */
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_magic = NET_MAGIC;
	sb->s_op = &netfs_s_ops;
/*
 * We need to conjure up an inode to represent the root directory
 * of this filesystem.  Its operations all come from libfs, so we
 * don't have to mess with actually *doing* things inside this
 * directory.
 */
	root = netfs_make_inode (sb, S_IFDIR | 0755);
	if (! root)
		goto out;
	root->i_op = &simple_dir_inode_operations;
	root->i_fop = &simple_dir_operations;
/*
 * Get a dentry to represent the directory in core.
 */
	root_dentry = d_make_root(root);
	if (! root_dentry)
		goto out_iput;
	sb->s_root = root_dentry;
/*
 * Make up the files which will be in this filesystem, and we're done.
 */
	netfs_create_files (sb, root_dentry);
	return 0;
	
  out_iput:
	iput(root);
  out:
	return -ENOMEM;
}


/*
 * Stuff to pass in when registering the filesystem.
 */
static struct dentry *netfs_get_super(struct file_system_type *fst,
		int flags, const char *devname, void *data)
{
	return mount_single(fst, flags, data, netfs_fill_super);
}

static struct file_system_type netfs_type = {
	.owner 		= THIS_MODULE,
	.name		= "netfs",
	.mount		= netfs_get_super,
	.kill_sb	= kill_litter_super,
};




/*
 * Get things set up.
 */
static int __init netfs_init(void)
{
	return register_filesystem(&netfs_type);
}

static void __exit netfs_exit(void)
{
	unregister_filesystem(&netfs_type);
}

module_init(netfs_init);
module_exit(netfs_exit);

