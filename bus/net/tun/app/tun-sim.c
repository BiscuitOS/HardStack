#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <errno.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
 
/**
 *  make interface link up
 */
int interface_up(char *interface_name)
{
    int s;
    struct ifreq ifr;
    short flag;
 
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error create socket :%d\n", errno);
        return -1;
    }
 
    strcpy(ifr.ifr_name, interface_name);
 
    flag = IFF_UP;
    if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0) {
        printf("Error up %s :%d\n", interface_name, errno);
        return -1;
    }
 
    ifr.ifr_ifru.ifru_flags |= flag;
 
    if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0) {
        printf("Error up %s :%d\n", interface_name, errno);
        return -1;
    }
 
    return 0;
 
}
 
/**
 *  set up ip address
 */
int set_ipaddr(char *interface_name, char *ip)
{
    int s;
 
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error up %s :%d\n", interface_name, errno);
        return -1;
    }
 
    struct ifreq ifr;
    strcpy(ifr.ifr_name, interface_name);
 
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = PF_INET;
    inet_aton(ip, &addr.sin_addr);
 
    memcpy(&ifr.ifr_ifru.ifru_addr, &addr, sizeof(struct sockaddr_in));
 
    if (ioctl(s, SIOCSIFADDR, &ifr) < 0) {
        printf("Error set %s ip :%d\n", interface_name, errno);
        return -1;
    }
 
    return 0;
}

static int netmask_set(char *interface_name, char *netmask)
{
    int sock_netmask;
    struct ifreq ifr_mask;
    struct sockaddr_in *sin_net_mask;

    sock_netmask = socket(AF_INET, SOCK_STREAM, 0);
    if( sock_netmask == -1)
        return -1;

    memset(&ifr_mask, 0, sizeof(ifr_mask));
    strncpy(ifr_mask.ifr_name, interface_name, sizeof(ifr_mask.ifr_name) -1);
    sin_net_mask = (struct sockaddr_in *)&ifr_mask.ifr_addr;
    sin_net_mask->sin_family = AF_INET;
    inet_pton(AF_INET, netmask, &sin_net_mask->sin_addr);

    if(ioctl(sock_netmask, SIOCSIFNETMASK, &ifr_mask) < 0)
        return -1;

    return 0;
}

/**
 *  Create a tun device.
 */
int tun_create(char *dev, int flags)
{
    struct ifreq ifr;
    int fd, err;
 
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        printf("Error :%d\n", errno);
        return -1;
    }
 
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags |= flags;
 
    if (*dev != '\0') {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }
 
    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        printf("Error :%d\n", errno);
        close(fd);
        return -1;
    }
 
    strcpy(dev, ifr.ifr_name);
 
    return fd;
}
 
/**
 * Setup route
 */
int route_add(char *interface_name)
{
    int skfd;
    struct rtentry rt;
 
    struct sockaddr_in dst;
    struct sockaddr_in gw;
    struct sockaddr_in genmask;
 
    memset(&rt, 0, sizeof(rt));
 
    genmask.sin_addr.s_addr = inet_addr("255.255.255.255");

    bzero(&dst, sizeof(struct sockaddr_in));
    dst.sin_family = PF_INET;
    dst.sin_addr.s_addr = inet_addr("10.0.0.1");
 
    rt.rt_metric = 0;
    rt.rt_dst = *(struct sockaddr *)&dst;
    rt.rt_genmask = *(struct sockaddr *)&genmask;
 
    rt.rt_dev = interface_name;
    rt.rt_flags = RTF_UP | RTF_HOST;
 
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
        printf("Error route add :%d\n", errno);
        return -1;
    }
 
    return 0;
}
 
int main(int argc, char *argv[])
{
    int tun, ret;
    char tun_name[IFNAMSIZ];
    unsigned char buf[4096];
    unsigned char ip[4];
 
    tun_name[0] = '\0';
    /* IFF_TAP: layer 2; IFF_TUN: layer 3 */
    tun = tun_create(tun_name, IFF_TUN | IFF_NO_PI);
    if (tun < 0) {
        return 1;
    }
    printf("TUN name is %s\n", tun_name);
 
    /* ¼¤»îÐéÄâÍø¿¨ */
    interface_up(tun_name);
    /* Ôö¼Óµ½ÐéÄâÍø¿¨µÄÂ·ÓÉ */
    route_add(tun_name);
 
    while (1) {
	int i;

        ret = read(tun, buf, sizeof(buf));
        printf("read %d bytes\n", ret);
		
        for (i = 0; i < ret; i++) {
            printf("%02x ", buf[i]);
        }
        printf("\n");
        if (ret < 0)
            break;
        /* ½»»»Ô´Ä¿µÄIP */
        memcpy(ip, &buf[12], 4);
        memcpy(&buf[12], &buf[16], 4);
        memcpy(&buf[16], ip, 4);
        /* ping request type 8 => ping reply type 0 */
        buf[20] = 0;
        /* adjust checksum */
        *((unsigned short *)&buf[22]) += 8;
        ret = write(tun, buf, ret);
        printf("write %d bytes\n", ret);
        for (i = 0; i < ret; i++) {
            printf("%02x ", buf[i]);
        }
        printf("\n");
    }
 
    return 0;
}
