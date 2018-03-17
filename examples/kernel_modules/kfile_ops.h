#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

static DEFINE_SPINLOCK(logfile_lock);

static struct file *file_open(const char *path, int flags, int rights) 
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
	printk(KERN_ERR "[%s]: filp:%p, read:%p, write:%p, fsync:%p pos: %lld\n",
			__FUNCTION__, filp, filp->f_op->read, filp->f_op->write, filp->f_op->fsync, filp->f_pos);
    return filp;
}

//Close a file (similar to close):
static void file_close(struct file *file) 
{
    filp_close(file, NULL);
}

//Reading data from a file (similar to pread):
static int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
	int ret = 0;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}   

//Writing data to a file (similar to pwrite):
static int file_write(struct file *file, unsigned long long offset, char *data, unsigned int size) 
{
    mm_segment_t oldfs;
	unsigned long long pos;
    int ret = 0;
	if (!file)
		return ret;


	spin_lock(&logfile_lock);
	pos = file->f_pos;
	spin_unlock(&logfile_lock);

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &pos);
	if (ret == size)
	{
		pr_info("Write successful size:%d pos: %lld\n", size, pos);
		spin_lock(&logfile_lock);
		file->f_pos += size;
		spin_unlock(&logfile_lock);
	} else {
		printk(KERN_ERR "Write failed to file\n");
	}

    set_fs(oldfs);
    return ret;
}

//Syncing changes a file (similar to fsync):
static int file_sync(struct file *file) 
{
    vfs_fsync(file, 0);
    return 0;
}

static inline unsigned int file_pos_read(struct file *file)
{
	return file->f_pos;
}

static inline void file_pos_write(struct file *file, unsigned int pos)
{
	file->f_pos = pos;
}
