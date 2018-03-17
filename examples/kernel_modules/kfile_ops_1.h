#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/syscalls.h> 
#include <linux/file.h> 
#include <linux/fs.h>
#include <linux/fcntl.h> 
#include <asm/uaccess.h> 

static struct file *file_open(const char *path, int flags, int rights) 
{
	struct file *filp = NULL;
	mm_segment_t oldfs;
	int err = 0;
	int fd;

	oldfs = get_fs();
	set_fs(get_ds());
	fd = sys_open(path, flags, rights);
	set_fs(oldfs);
	if (fd >= 0) {
		filp = fget(fd);
		if (IS_ERR(filp)) {
			err = PTR_ERR(filp);
			return NULL;
		}
	}
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
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}   

//Writing data to a file (similar to pwrite):
static int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

//Syncing changes a file (similar to fsync):
static int file_sync(struct file *file) 
{
    vfs_fsync(file, 0);
    return 0;
}
