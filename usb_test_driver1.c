#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/usb.h>
#define USB_VENDOR_ID 0x0930
#define USB_PRODUCT_ID 0x6544
#define BULK_EP_IN 0x82
#define BULK_EP_OUT 0x01
#define MAX_PKT_SIZE 512
MODULE_LICENSE("GPL");
MODULE_AUTHOR("shashank");
MODULE_DESCRIPTION("usb test driver for toshiba");


struct usb_class_driver usb_cd;
struct usb_device *device;

static unsigned char bulk_buf[MAX_PKT_SIZE];
static int usb_probe(struct usb_interface *interface,const struct usb_device_id *id)
{
int i;
struct usb_host_interface *intface_desc;
struct usb_endpoint_descriptor *endpoint;
int ret;
intface_desc=interface->cur_altsetting;
printk(KERN_INFO "usb info %d now probed : (%04X:%04X)\n",intface_desc->desc.bInterfaceNumber,id->idVendor,id->idProduct);
printk(KERN_INFO "ID->bNumEndpoints : %02x\n",intface_desc->desc.bNumEndpoints);
printk(KERN_INFO "ID->bInterfaceClass: %02x\n",intface_desc->desc.bInterfaceClass);
printk(KERN_INFO "ID->bInterfaceProtocol : %04x\n",intface_desc->desc.bInterfaceProtocol);
printk(KERN_INFO "ID->iInterface :%d\n",intface_desc->desc.iInterface);
printk(KERN_INFO "ID-> bLength : %d\n",intface_desc->desc.bLength);
for(i=0;i<intface_desc->desc.bNumEndpoints;i++)
{
endpoint=&intface_desc->endpoint[i].desc;

printk(KERN_INFO "ED[%d]->bEndpointAddress : %02x\n",i,endpoint->bEndpointAddress);
printk(KERN_INFO "ED[%d]->bmAttributes : %02x\n",i,endpoint->bmAttributes);
printk(KERN_INFO "ED[%d]->wMaxPacketSize : %04x (%d)\n",i,endpoint->wMaxPacketSize,endpoint->wMaxPacketSize);
}

device=interface_to_usbdev(interface);
ret=usb_register_dev(interface,&usb_cd);
if(!ret)
{
pr_info("usb device registration failed\n");
return -1;
}
else
{
pr_info("minor number=%d\n",interface->minor);
}
return 0;
}


static void usb_disconnect(struct usb_interface *interface)
{
pr_info("usb device disconnected with minor number: %d\n",interface->minor);
usb_deregister_dev(interface,&usb_cd);
}



static struct usb_device_id usb_id_table[]={
{ 
USB_DEVICE(USB_VENDOR_ID,USB_PRODUCT_ID)
},
{}
};

MODULE_DEVICE_TABLE(usb,usb_id_table);


static int usb_test_open(struct inode *inode,struct file *filp)
{
pr_info("%s: in open function\n",__func__);
return 0;
}

static int usb_test_close(struct inode *inode,struct file *filp)
{
pr_info("%s: in close function\n",__func__);
return 0;
}


static ssize_t usb_test_read(struct file *filp,char __user *user_buffer,ssize_t count,loff_t *offset)
{
pr_info("%s: in read function\n",__func__);
int retval,read_cnt;
retval=usb_bulk_msg(device,usb_rcvbulkpipe(device,BULK_EP_IN),bulk_buf,MAX_PKT_SIZE,&read_cnt,5000);
if(retval)
{
printk(KERN_INFO "bulk message returned: %d\n",retval);
return retval;
}

if(copy_to_user(user_buffer,bulk_buf,count))
{
return -EFAULT;
}
return count;
}


static ssize_t usb_test_write(struct file *filp,const char __user *user_buffer,ssize_t count,loff_t *offset)
{
pr_info("%s: in write function\n",__func__);
int retval;
int write_cnt;
if(copy_from_user(bulk_buf,user_buffer,count))
{
return -EFAULT;
}
retval=usb_bulk_msg(device,usb_sndbulkpipe(device,BULK_EP_OUT),bulk_buf,count,&write_count,5000);
if(retval)
{
printk(KERN_ERR "bulk message returned: %d\n",retval);
}
return count;
}


static struct file_operations fops={
.owner=THIS_MODULE,
.read=usb_test_read,
.write=usb_test_write,
.open=usb_test_open,
.close=usb_test_close,
};

static struct usb_driver usb_test_driver={
.name="Toshiba test driver",
.probe=usb_probe,
.id_table=usb_id_table,
.disconnect=usb_disconnect,
};

static int __init test_hello_init(void)
{
pr_info("%s: in init function\n",__func__);
usb_register(&usb_test_driver);
return 0;
}
static void __exit test_hello_exit(void)
{
pr_info("%s: in exit function\n",__func__);
usb_deregister(&usb_test_driver);
}

module_init(test_hello_init);
module_exit(test_hello_exit);
