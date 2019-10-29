#ifndef _AT24C08_LIB_H_
#define _AT24C08_LIB_H_

#define I2C_BUS		"/dev/i2c-1"
#define I2C_ADDR	0x50
#define I2C_M_WR	0
#define I2C_CHUNK_SIZE	8 /* 8 bytes */

#define div_up(x, y)	(((x) + (y - 1)) / (y))
#define div_down(x, y)	((x) / (y))

extern int at24c08_Random_read(int fd, unsigned char addr,
			unsigned char offset, unsigned char *buf);
extern int at24c08_Sequen_read(int fd, unsigned char addr,
			unsigned char offset, unsigned char *buf, int len);
extern int at24c08_Current_Address_read(int fd, unsigned char addr,
			unsigned char *buf);
extern int at24c08_Byte_write(int fd, unsigned char addr, unsigned char offset,
			unsigned char data);
extern int at24c08_Page_write(int fd, unsigned char addr, unsigned char offset,
			unsigned char *buf, int len);
extern int at24c08_packet_read(int fd, unsigned char addr, unsigned char offset,
			unsigned char *buf, int len);
extern int at24c08_packet_write(int fd, unsigned char addr, 
			unsigned char offset, unsigned char *buf, int len);
#endif
