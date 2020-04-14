/*
 * written by renyuan on 2020.1.4
 *
 *
 */
#include "include.h"
#include "extern.h"
#include "pgcache_scan.h"


struct list_head *_super_blocks = NULL;
spinlock_t *_sb_lock = NULL;
spinlock_t *inode_sb_list_lock = NULL;

extern struct task_struct init_task;
iterate_supers_addr iterate_supers_function;



static LIST_HEAD(ordered_list);
static DEFINE_SPINLOCK(ordered_list_lock);

void order_list_clear(void);
void print_top_num(int n);
void scan_inodes_pagecache_one_sb(struct super_block *sb, void *arg);
int scan_file_inode(const void *v, struct file *f, unsigned fd);
int scan_caches_sysctl_handler(struct ctl_table *table, int write,
        void __user *buffer, size_t *length, loff_t *ppos);




static void order_list_add(pgcount_node_t *new) 
{
    pgcount_node_t *node = NULL, *tmp = NULL;

    if (list_empty(&ordered_list)) {
        list_add(&new->list, &ordered_list);
        return;
    }

    list_for_each_entry_safe(node, tmp, &ordered_list, list) {
        if (new->pagecount > node->pagecount) {
            list_add(&new->list, node->list.prev);
            return;
        } 
    }

    list_add_tail(&new->list, &ordered_list);

    return;
}

void order_list_clear(void)
{
    pgcount_node_t *node = NULL, *tmp = NULL;

    spin_lock(&ordered_list_lock);
    list_for_each_entry_safe(node, tmp, &ordered_list, list) {
        list_del(&node->list);
        kfree(node);
    }
    spin_unlock(&ordered_list_lock);

    return;
}

#define K (1024)
#define M (1024 * (K))
#define G (1024 * (M))

void print_top_num(int num)
{
    int count = 0;
    uint64_t gb = 0UL, mb = 0UL, kb = 0UL;
    uint64_t bytes = 0UL;
    pgcount_node_t *n = NULL, *tmp = NULL;

    if (list_empty(&ordered_list)) {
        printk("ordered_list is empty !\n");
	goto out;
    }

    spin_lock(&ordered_list_lock);
    printk("\n"); 
    list_for_each_entry_safe(n, tmp, &ordered_list, list) {
        if (count++ < num) {
            bytes = n->pagecount * PAGE_SIZE;
            gb  = bytes / G;
            mb  = (bytes - (gb * G)) / M;
            kb  = (bytes - (gb * G) - (mb * M)) / K; 

            if (gb != 0) {
                printk("pgscan: %6s ino: %10llu icount: %u nrpage: %8llu %3lluGB,%3lluMB,%3lluKB isz: %llu\t pid: %-6u\t comm: %s path: %s\n", 
                    n->devname, n->ino, n->icount, n->pagecount, gb, mb, kb, n->size, n->pid, n->comm, n->abspath);
            } else if (mb != 0) {
                printk("pgscan: %6s ino: %10llu icount: %u nrpage: %8llu %3lluGB,%3lluMB,%3lluKB isz: %llu\t pid: %-6u\t comm: %s path: %s\n", 
                    n->devname, n->ino, n->icount, n->pagecount, gb, mb, kb, n->size, n->pid, n->comm, n->abspath);
            } else if (kb != 0) {
                printk("pgscan: %6s ino: %10llu icount: %u nrpage: %8llu %3lluGB,%3lluMb,%3lluKB isz: %llu\t pid: %-6u\t comm: %s path: %s\n", 
                    n->devname, n->ino, n->icount, n->pagecount, gb, mb, kb, n->size, n->pid, n->comm, n->abspath);
            }
        }
    }
    printk("\n"); 

out:
    spin_unlock(&ordered_list_lock);
    return;    
}

void scan_inodes_pagecache_one_sb(struct super_block *sb, void *arg)
{
    struct inode *inode = NULL;
    pgcount_node_t *pgc = NULL;
    struct address_space *mapping = NULL;
    //struct dentry *de   = NULL;
  
    spin_lock(&ordered_list_lock); 
    spin_lock(inode_sb_list_lock);
    list_for_each_entry(inode, &sb->s_inodes, i_sb_list) {
        if (pgc == NULL) {
            pgc = kzalloc((sizeof(pgcount_node_t)), GFP_KERNEL);
            if (pgc == NULL) {
                printk("kmalloc failed, nomem");
                goto out;
            }
        }

        spin_lock(&inode->i_lock);
        mapping = inode->i_mapping;
        if (inode->i_state & (I_FREEING | I_WILL_FREE | I_NEW) || mapping->nrpages == 0) {
            spin_unlock(&inode->i_lock);
            continue;
        }
	if (sb->s_bdev && sb->s_bdev->bd_part && sb->s_bdev->bd_disk) {
            pgc->ino = inode->i_ino;
            snprintf(pgc->devname, sizeof(pgc->devname), "%s%d",  sb->s_bdev->bd_disk->disk_name, 
                               sb->s_bdev->bd_part->partno);
            pgc->pagecount = mapping->nrpages;
            pgc->icount = atomic_read(&inode->i_count);
        } else {
            snprintf(pgc->devname, sizeof(pgc->devname), "(null)");
            //printk("%s ino: %ul\tnrpages: %ul\n", __FUNCTION__, inode->i_ino, mapping->nrpages);
        }
        
        spin_unlock(&inode->i_lock);
/*
        de = d_find_alias(inode);
        if (de) {
            memset(buf, 0, PATH_MAX);
            res = dentry_path_raw(de, buf, (PATH_MAX - 10 - 1));
        printk("res=%d path = %s\n", IS_ERR(res), buf);
            if (!IS_ERR(res) && d_unlinked(de)) {
                memcpy((ptr + strlen(buf)), "//deleted", 10);
                memcpy(pgc->abspath, buf, strlen(buf));
            }
        }
*/
        order_list_add(pgc);
        pgc = NULL;
    } // list_for_each_entry

out:
    spin_unlock(inode_sb_list_lock);
    spin_unlock(&ordered_list_lock); 

    return;
}

#if 0
static int count_open_files(struct fdtable *fdt)
{
	int size = fdt->max_fds;
	int i;

	/* Find the last open fd */
	for (i = size / BITS_PER_LONG; i > 0; ) {
		if (fdt->open_fds[--i])
			break;
	}
	i = (i + 1) * BITS_PER_LONG;
	return i;
}
#endif

int scan_file_inode(const void *v, struct file *f, unsigned fd)
{
    int ret = 0;
    char *p = NULL;
   struct inode *inode = NULL; 
    struct address_space *mapping = NULL; 
    pgcount_node_t *pgc = NULL;
    struct super_block *sb = NULL;
    const struct task_struct *tsk = v;

    if (!f) 
        return 0;

    //spin_lock(&f->f_lock); 
    inode = f->f_mapping->host; //f->f_inode only is cached 
    if (inode) {
        //iget(inode);
        mapping = inode->i_mapping;
        if (mapping->nrpages == 0)
            goto out;

        if (sysctl_pgcache_scan_file_deleted_but_used) {
            if (!((inode->i_nlink == 0) && (atomic_read(&inode->i_count) != 0))) {
                goto out;
            }
        }
        pgc = kzalloc((sizeof(pgcount_node_t) + PATH_MAX + 11), GFP_KERNEL);
        if (pgc == NULL) {
            ret = -ENOMEM;
            goto out;
        }
        pgc->abspath = (char *)(pgc + 1);
        p = d_path(&f->f_path, pgc->abspath, (PATH_MAX + 11));
        pgc->ino = inode->i_ino;
        pgc->size = i_size_read(inode);
        pgc->abspath = p;
        if (inode->i_sb) {
            sb = inode->i_sb;
            if (sb && sb->s_bdev && sb->s_bdev->bd_disk) {
                snprintf(pgc->devname, sizeof(pgc->devname), "%s%d",  sb->s_bdev->bd_disk->disk_name, 
                               sb->s_bdev->bd_part->partno);
            } else {
                snprintf(pgc->devname, sizeof(pgc->devname), "(null)");
            }
        }
        pgc->pagecount = mapping->nrpages;
        pgc->icount = atomic_read(&inode->i_count);
        if (tsk) {
            pgc->pid = tsk->pid;
            memcpy(pgc->comm, tsk->comm, TASK_COMM_LEN);
        }
#if 0
        printk("path = %s, ino = %lu fd: %u devname: %s com: %s nrpage： %d\n", pgc->abspath, pgc->ino, fd, 
                     pgc->devname, tsk->comm, pgc->pagecount);
#endif
        if (pgc) {
            order_list_add(pgc);
        }
        
        //iput(inode);
    }

out:
    return 0;
}
 
int scan_process_inodes_pagecache(void)
{
    struct task_struct *p = NULL;
    //struct fdtable *fdt = NULL;
    //const struct cred *cred;
    //pid_t ppid, tpid;

    rcu_read_lock();
    for_each_process(p) {
        if ((p == &init_task) || (p == current)) {
            continue;
        }
        //cred = get_task_cred(p);
	task_lock(p);
        if (p->files) {
            spin_lock(&ordered_list_lock); 
            iterate_fd(p->files, 1, scan_file_inode, p);
            spin_unlock(&ordered_list_lock); 
        }
	task_unlock(p);

//	put_cred(cred);
    }
    rcu_read_unlock();

    return 0;
}

int scan_caches_sysctl_handler(struct ctl_table *table, int write,
        void __user *buffer, size_t *length, loff_t *ppos)
{
    int ret = 0;

    ret = proc_dointvec_minmax(table, write, buffer, length, ppos);
    if (ret)
        return ret;
#if 0
    printk("%s ret = %d sysctl_pgcache_scan_mode = %d\n", __FUNCTION__, ret, sysctl_pgcache_scan_mode);
#endif
    if (write) {
        switch (sysctl_pgcache_scan_mode) {
            case 0:
                order_list_clear();
                scan_process_inodes_pagecache();
                print_top_num(sysctl_pgcache_scan_top_n);
                break;
            case 1:
                order_list_clear();
                iterate_supers_function(scan_inodes_pagecache_one_sb, NULL);
                print_top_num(sysctl_pgcache_scan_top_n);
                break;
            case 2:
                order_list_clear();
                iterate_supers_function(scan_inodes_pagecache_one_sb, NULL);
                scan_process_inodes_pagecache();
                print_top_num(sysctl_pgcache_scan_top_n);
                break;
            default:
                break;
        }
    }

    return 0;
}


