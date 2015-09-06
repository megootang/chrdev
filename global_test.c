#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/workqueue.h>

#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/errno.h>


MODULE_LICENSE("GPL");

#define init_MUTEX(LOCKNAME) sema_init(LOCKNAME,1)

#define DEVICE_NAME "CDEV_TANG"

static struct class *cdev_class;

struct cdev dev_c;

dev_t dev;

static ssize_t globalvar_read(struct file *, char *,size_t, loff_t*);
static ssize_t globalvar_write(struct file *, const char *,size_t,loff_t*);


static const struct file_operations globalvar_fops = {
	.read	= globalvar_read,
	.write	= globalvar_write,
};
static struct semaphore sem;
static wait_queue_head_t wq_test;
static int flag = 0;
static int global_var = 0;
static int __init globalvar_init(void) {
	int ret,err;

	ret = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
	if(ret) {
		printk("globalvar register failure ret=%d \n",ret);
		return ret ;
	}

	cdev_init(&dev_c, &globalvar_fops);

	err = cdev_add(&dev_c, dev, 1);
	if(err) {
		printk(KERN_EMERG "add cdev err %d \n",err);
		unregister_chrdev_region(dev,1);
		return err;
	}
	printk(KERN_EMERG "device register success! \n");

	cdev_class = class_create(THIS_MODULE,DEVICE_NAME);
	if(IS_ERR(cdev_class)) {
		printk(KERN_EMERG "ERR:can not create a cdev class\n");
		unregister_chrdev_region(dev, 1);
		return -1 ;
	}
	device_create(cdev_class, NULL, dev, 0, DEVICE_NAME);

	init_MUTEX(&sem);
	init_waitqueue_head(&wq_test);

	return 0;
}

static void __exit globalvar_exit(void) {
	device_destroy(cdev_class,dev);
	class_destroy(cdev_class);
	unregister_chrdev_region(dev, 1);
	printk(KERN_EMERG " globalvar exit \n");
}

static ssize_t globalvar_read(struct file *flip, char * buf, size_t len, loff_t *off) {
	
	printk(KERN_EMERG "start global read!\n");
	if(wait_event_interruptible(wq_test,flag != 0)) {
		printk(KERN_EMERG "current flag not equal \n");
		return -ERESTARTSYS;
	}

	if(down_interruptible(&sem)) {
		return -ERESTARTSYS;
	}
	printk(KERN_EMERG "wake up start read\n");
	flag = 0;
	if(copy_to_user(buf,&global_var, sizeof(int))) {
		up(&sem);
		return -EFAULT ;
	}
	up(&sem);
	return sizeof(int);
}

static ssize_t globalvar_write(struct file *filp, const char *buf, size_t len, loff_t *off) {
	printk(KERN_EMERG "wake up write!\n");
	if(down_interruptible(&sem)) {
		return -ERESTARTSYS;
	}

	if(copy_from_user(&global_var, buf, sizeof(int))) {
		up(&sem);
		return -ERESTARTSYS;
	}
	up(&sem);
	flag = 1;
	printk(KERN_EMERG "write buf down\n");
	wake_up_interruptible(&wq_test);
	printk(KERN_EMERG "wake up write thread \n");

	return sizeof(int);
}
module_init(globalvar_init);
module_exit(globalvar_exit);
