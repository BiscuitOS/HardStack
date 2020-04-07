#ifndef _BISCUITOS_KVM_H
#define _BISCUITOS_KVM_H

int kvm_init_bs(void *opaque, unsigned vcpu_size, unsigned vcpu_align,
                                struct module *module);

#define BS_DUP()	printk("Expand..[%s-%d]\n", __func__, __LINE__)

#endif
