#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>


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
	printk(KERN_ERR "[%s]: filp:%p, read:%p, write:%p, fsync:%p\n",
			__FUNCTION__, filp, filp->f_op->read, filp->f_op->write, filp->f_op->fsync);
    return filp;
}

//Close a file (similar to close):
static void file_close(struct file *file) 
{
	if (file)
		filp_close(file, NULL);
}

//Syncing changes a file (similar to fsync):
static int file_sync(struct file *file) 
{
    vfs_fsync(file, 0);
    return 0;
}

//Reading data from a file (similar to pread):
static int file_write(struct file *fp, unsigned long long offset, unsigned char *data, unsigned int size)
{
#if 0
	mm_segment_t oldfs;
    char buffer[512] __attribute__((aligned(sizeof(long))));
	struct timespec ts;
	if (!fp) return 1;

    oldfs = get_fs();
    set_fs(get_ds());

	getnstimeofday(&ts);
	sprintf(buffer, "[%ld.%ld]", ts.tv_sec, ts.tv_nsec);
	strncat(buffer, data, size);
	
	fp->f_op->write(fp, buffer, strlen(buffer), &fp->f_pos);
	set_fs(oldfs);

	return 1;
#endif
	loff_t pos = 0;
	int ret = 0;
	mm_segment_t old_fs = get_fs();

	set_fs(KERNEL_DS);
	if (fp && fp->f_op->write && fp->f_op->fsync)
	{
		ret = __kernel_write(fp, (char*)data, size, &pos);
		if (ret != size)
			printk(KERN_ERR "Failed to write to hslk trace\n");
		else {
			fp->f_op->fsync(fp, 0, LLONG_MAX, 1);
		}
	}
	set_fs(old_fs);
	return 1;
}
