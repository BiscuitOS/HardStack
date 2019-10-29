#ifndef _CHEST_H_
#define _CHEST_H_

extern void chest_probe(void);
extern void chest_remove(void);

extern int chest_read_interface(char *optarg, char *argv[]);
extern int chest_write_interface(char *optarg, char *argv[], int optind);

#endif
