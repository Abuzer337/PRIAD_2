#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/rwlock.h>
#include <linux/device.h>
#include <linux/path.h>
#include <linux/dcache.h>
#include <linux/namei.h>
#include <linux/version.h>

#define MY_DEVICE "invisible"
#define NAME_SIZE 256
#define TARGET_DIR "/tmp"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("LAVRENTEV YFIM");
MODULE_DESCRIPTION("INVISIBLE - VFS Interception Driver for File Hiding");
MODULE_VERSION("1.0");

struct file_to_hide {
    struct list_head list;
    char file_name[NAME_SIZE];
};

/* VFS interception structures */
static const struct file_operations *original_fops = NULL;
static int (*original_iterate_shared)(struct file *, struct dir_context *) = NULL;
static struct inode *target_inode = NULL;

/* Global variables */
static LIST_HEAD(hidden_list);
static DEFINE_RWLOCK(list_lock);
static int major_num;
static struct class *my_class;

/* Helper structure for passing data between iterate and filldir */
struct my_dir_context {
    struct dir_context ctx;
    struct dir_context *original_ctx;
};

/* Custom filldir function - filters hidden files */
static bool my_filldir(struct dir_context *ctx, const char *name,
                      int namelen, loff_t offset, u64 ino, unsigned d_type)
{
    struct file_to_hide *item;
    struct my_dir_context *my_ctx = container_of(ctx, struct my_dir_context, ctx);
    int is_hidden = 0;
    
    /* Check if file is in hidden list */
    read_lock(&list_lock);
    list_for_each_entry(item, &hidden_list, list) {
        if (strncmp(item->file_name, name, namelen) == 0 && 
            strlen(item->file_name) == namelen) {
            is_hidden = 1;
            break;
        }
    }
    read_unlock(&list_lock);
    
    /* Skip hidden files */
    if (is_hidden) {
        return true; /* Continue iteration but skip this file */
    }
    
    /* Call original filldir for normal files */
    return my_ctx->original_ctx->actor(my_ctx->original_ctx, name, namelen, offset, ino, d_type);
}

/* Custom iterate_shared function */
static int my_iterate_shared(struct file *file, struct dir_context *ctx)
{
    struct my_dir_context my_ctx;
    int result;
    
    /* Setup our context */
    my_ctx.original_ctx = ctx;
    my_ctx.ctx.actor = my_filldir;
    my_ctx.ctx.pos = ctx->pos;
    
    /* Call original iterate_shared with our context */
    result = original_iterate_shared(file, &my_ctx.ctx);
    
    /* Restore position */
    ctx->pos = my_ctx.ctx.pos;
    
    return result;
}

/* Setup VFS interception */
static int setup_vfs_interception(void)
{
    struct path path;
    int ret;
    struct file_operations *new_fops;
    
    printk(KERN_INFO "Setting up VFS interception for %s\n", TARGET_DIR);
    
    /* Get path to target directory */
    ret = kern_path(TARGET_DIR, LOOKUP_FOLLOW, &path);
    if (ret) {
        printk(KERN_ERR "Failed to get path for %s: %d\n", TARGET_DIR, ret);
        return ret;
    }
    
    target_inode = path.dentry->d_inode;
    
    /* Save original operations */
    original_fops = target_inode->i_fop;
    if (!original_fops) {
        printk(KERN_ERR "No file operations for target directory\n");
        return -ENOENT;
    }
    
    original_iterate_shared = original_fops->iterate_shared;
    if (!original_iterate_shared) {
        printk(KERN_ERR "No iterate_shared in file operations\n");
        return -ENOENT;
    }
    
    /* Create copy of file operations */
    new_fops = kmalloc(sizeof(struct file_operations), GFP_KERNEL);
    if (!new_fops) {
        printk(KERN_ERR "Failed to allocate memory for new file operations\n");
        return -ENOMEM;
    }
    
    /* Copy original operations and replace iterate_shared */
    memcpy(new_fops, original_fops, sizeof(struct file_operations));
    new_fops->iterate_shared = my_iterate_shared;
    
    /* Replace file operations */
    target_inode->i_fop = new_fops;
    
    printk(KERN_INFO "VFS interception successfully set up for %s\n", TARGET_DIR);
    return 0;
}

/* Restore original VFS operations */
static void restore_vfs_operations(void)
{
    if (target_inode && original_fops) {
        /* Restore original file operations */
        target_inode->i_fop = original_fops;
        printk(KERN_INFO "Original VFS operations restored\n");
    }
    
    /* Free allocated file operations */
    if (target_inode && target_inode->i_fop != original_fops) {
        kfree(target_inode->i_fop);
    }
}

/* Character device functions */
static int my_open(struct inode *node, struct file *file)
{
    return 0;
}

static int my_close(struct inode *node, struct file *file)
{
    return 0;
}

static ssize_t my_read(struct file *file_ptr, char *user_buf, size_t len, loff_t *pos)
{
    struct file_to_hide *item;
    char buf[512] = {0};
    int buf_len = 0;
    int ret;
    
    if (*pos > 0) {
        return 0;
    }
    
    read_lock(&list_lock);
    
    list_for_each_entry(item, &hidden_list, list) {
        int added = snprintf(buf + buf_len, sizeof(buf) - buf_len, 
                           "%s\n", item->file_name);
        if (added > 0 && (buf_len + added) < sizeof(buf)) {
            buf_len += added;
        } else {
            break;
        }
    }
    
    read_unlock(&list_lock);
    
    if (buf_len == 0) {
        const char *msg = "No hidden files\n";
        buf_len = strlen(msg);
        strncpy(buf, msg, sizeof(buf));
    }
    
    if (buf_len > len) {
        buf_len = len;
    }
    
    ret = copy_to_user(user_buf, buf, buf_len);
    if (ret != 0) {
        return -EFAULT;
    }
    
    *pos = buf_len;
    return buf_len;
}

static ssize_t my_write(struct file *file_ptr, const char *user_buf, size_t len, loff_t *pos)
{
    char cmd[256];
    char fname[NAME_SIZE];
    struct file_to_hide *item, *temp;
    int found = 0;
    int ret;
    
    if (len == 0 || len >= sizeof(cmd)) {
        return -EINVAL;
    }
    
    ret = copy_from_user(cmd, user_buf, len);
    if (ret != 0) {
        return -EFAULT;
    }
    cmd[len] = '\0';
    
    if (sscanf(cmd, "hide %255s", fname) == 1) {
        write_lock(&list_lock);
        
        list_for_each_entry(item, &hidden_list, list) {
            if (strcmp(item->file_name, fname) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            item = kmalloc(sizeof(struct file_to_hide), GFP_KERNEL);
            if (!item) {
                write_unlock(&list_lock);
                return -ENOMEM;
            }
            strncpy(item->file_name, fname, NAME_SIZE - 1);
            item->file_name[NAME_SIZE - 1] = '\0';
            INIT_LIST_HEAD(&item->list);
            list_add_tail(&item->list, &hidden_list);
            printk(KERN_INFO "Hiding file: %s\n", fname);
        }
        
        write_unlock(&list_lock);
        
    } else if (sscanf(cmd, "unhide %255s", fname) == 1) {
        write_lock(&list_lock);
        
        list_for_each_entry_safe(item, temp, &hidden_list, list) {
            if (strcmp(item->file_name, fname) == 0) {
                list_del(&item->list);
                kfree(item);
                printk(KERN_INFO "Showing file: %s\n", fname);
                found = 1;
                break;
            }
        }
        
        write_unlock(&list_lock);
        
        if (!found) {
            printk(KERN_INFO "File not hidden: %s\n", fname);
        }
    } else {
        printk(KERN_INFO "Invalid command: %s\n", cmd);
        return -EINVAL;
    }
    
    return len;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write,
};

static int __init my_module_start(void)
{
    int ret;
    struct device *dev;
    
    printk(KERN_INFO "Loading VFS interception file hider module\n");
    
    INIT_LIST_HEAD(&hidden_list);
    
    /* Register character device */
    ret = register_chrdev(0, MY_DEVICE, &my_fops);
    if (ret < 0) {
        printk(KERN_ERR "Cannot register device: %d\n", ret);
        return ret;
    }
    
    major_num = ret;
    
    /* Create device class */
    my_class = class_create(MY_DEVICE);
    if (IS_ERR(my_class)) {
        printk(KERN_ERR "Cannot create device class\n");
        unregister_chrdev(major_num, MY_DEVICE);
        return PTR_ERR(my_class);
    }
    
    /* Create device */
    dev = device_create(my_class, NULL, MKDEV(major_num, 0), NULL, MY_DEVICE);
    if (IS_ERR(dev)) {
        printk(KERN_ERR "Cannot create device\n");
        class_destroy(my_class);
        unregister_chrdev(major_num, MY_DEVICE);
        return PTR_ERR(dev);
    }
    
    /* Setup VFS interception */
    ret = setup_vfs_interception();
    if (ret) {
        printk(KERN_ERR "Failed to setup VFS interception: %d\n", ret);
        device_destroy(my_class, MKDEV(major_num, 0));
        class_destroy(my_class);
        unregister_chrdev(major_num, MY_DEVICE);
        return ret;
    }
    
    printk(KERN_INFO "VFS file hider loaded successfully\n");
    printk(KERN_INFO "Device major number: %d\n", major_num);
    printk(KERN_INFO "Target directory: %s\n", TARGET_DIR);
    printk(KERN_INFO "Use: echo 'hide filename' > /dev/invisible\n");
    printk(KERN_INFO "Use: echo 'unhide filename' > /dev/invisible\n");
    printk(KERN_INFO "Use: cat /dev/invisible to see hidden files\n");
    
    return 0;
}

static void __exit my_module_end(void)
{
    struct file_to_hide *item, *temp;
    
    printk(KERN_INFO "Unloading VFS file hider\n");
    
    /* Restore VFS operations */
    restore_vfs_operations();
    
    /* Clean up hidden files list */
    write_lock(&list_lock);
    list_for_each_entry_safe(item, temp, &hidden_list, list) {
        list_del(&item->list);
        kfree(item);
        printk(KERN_INFO "Removed hidden file: %s\n", item->file_name);
    }
    write_unlock(&list_lock);
    
    /* Remove device and class */
    device_destroy(my_class, MKDEV(major_num, 0));
    class_destroy(my_class);
    unregister_chrdev(major_num, MY_DEVICE);
    
    printk(KERN_INFO "VFS file hider unloaded\n");
}

module_init(my_module_start);
module_exit(my_module_end);