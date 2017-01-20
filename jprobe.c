#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <linux/fs.h>

static void
long2ip (long ip, char *buf)
{
    sprintf (buf, "%ld.%ld.%ld.%ld", 
                ((0xFF  << 24) & ip) >> 24, 
                ((0xFF << 16) & ip) >> 16,
                ((0xFF << 8) & ip) >> 8, 
                ip & 0xFF);
}


static long
do_sys_connect (int fd, struct sockaddr __user * uservaddr, int addrlen)
{
    struct sockaddr_in  *in;
    char                 buf[1024];
 
    in = (struct sockaddr_in *) uservaddr;
    long2ip (htonl ((in->sin_addr).s_addr), buf);
    /* filter not tcp and port equal 0 */
    if (in->sin_family == AF_INET && in->sin_port != 0) {
        printk (KERN_INFO
	        "jprobe: fd = %d, address = %s, port = %d, addrlen = %d\n", fd,
	        buf, htons (in->sin_port), addrlen);
    }
    jprobe_return ();
    return 0;
}

static struct jprobe my_jprobe = {
    .entry = do_sys_connect,
    .kp = {
	    .symbol_name = "sys_connect",
    },
};

static int __init
jprobe_init (void)
{
    int ret;

    ret = register_jprobe (&my_jprobe);
    if (ret < 0) {
        printk (KERN_INFO "register_jprobe failed, returned %d\n", ret);
        return -1;
    }
    printk (KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	    my_jprobe.kp.addr, my_jprobe.entry);
    return 0;
}

static void __exit
jprobe_exit (void)
{
    unregister_jprobe (&my_jprobe);
    printk (KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init (jprobe_init)
module_exit (jprobe_exit)
MODULE_LICENSE ("GPL");
