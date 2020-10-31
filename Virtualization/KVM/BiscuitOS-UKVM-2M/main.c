/*
 * KVM Userspace 2M HugePage on BiscuitOS
 *
 * (C) 2020.10.24 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
/* KVM Header */
#include <linux/kvm.h>

/* KVM Path */
#define KVM_PATH		"/dev/kvm"
/* KVM-Virtual-Machine ISO */
#define BISCUITOS_OS_PATH	"/usr/bin/firmware/_BiscuitOS-VM.iso"
/* HugePage Path */
#define BISCUITOS_HUGE_PATH	"/mnt/HugePagefs/BiscuitOS_HugePage"
/* ISO Bunck */
#define ISO_BUNCK		4096
/* KVM Version */
#define KVM_VERSION		12
/* Virutal-Machine Memory size */
#define VM_MEMORY_SIZE		0x1000000

/*
 * Load OS into VM memory.
 */
static int BiscuitOS_load_OS(char *memory)
{
	int fd;
	int nbytes;

	fd = open(BISCUITOS_OS_PATH, O_RDONLY);
	if (fd < 0)
		return fd;

	/* Copy ISO */
	while (1) {
		nbytes = read(fd, memory, ISO_BUNCK);
		if (nbytes <= 0)
			break;
		memory += nbytes;
	}

	close(fd);
	return 0;
}

int main()
{
	struct kvm_userspace_memory_region region;
	int kvm_fd, vm_fd, vcpu_fd, huge_fd;
	struct kvm_run *kvm_run;
	int kvm_run_size;
	struct kvm_sregs sregs;
	struct kvm_regs regs;
	char *vm_memory;
	int error;

	/* Open KVM */
	kvm_fd = open(KVM_PATH, O_RDWR | O_CLOEXEC);
	if (kvm_fd < 0) {
		printf("ERROR[%d]: open %s failed.\n", kvm_fd, KVM_PATH);
		error = kvm_fd;
		goto err_open;
	}

	/* Check KVM Version */
	error= ioctl(kvm_fd, KVM_GET_API_VERSION, NULL);
	if (error != KVM_VERSION) {
		printf("ERROR[%d]: Invalid KVM version.\n", error);
		goto err_version;
	}

	/* Create an Virtual-Machine */
	vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, (unsigned long)0);
	if (vm_fd < 0) {
		printf("ERROR[%d]: Create Virtual-Machine failed.\n", vm_fd);
		error = vm_fd;
		goto err_version;
	}

	/* HugePage */
	huge_fd = open(BISCUITOS_HUGE_PATH, O_CREAT | O_RDWR);
	if (huge_fd < 0) {
		error = huge_fd;
		printf("ERROR[%d]: Open %s failed.\n", error,
							BISCUITOS_HUGE_PATH);
		goto err_version;
	}

	/* Allocate memory for Virtual-Machine from mmap */
	vm_memory = mmap(NULL, 
			 VM_MEMORY_SIZE,
			 PROT_READ | PROT_WRITE,
			 MAP_SHARED,
			 huge_fd,
			 0);

	if (vm_memory == MAP_FAILED) {
		perror("ERROR: Allocat VM memory failed.\n");
		error = -1;
		goto err_huge;
	}
	close(huge_fd);

	/* Load Virtual-VM Code into VM memory */
	error = BiscuitOS_load_OS(vm_memory);
	if (error < 0) {
		printf("ERROR[%d]: Load OS failed.\n", error);
		goto err_load;
	}

	/* Setup KVM memory region */
	region.slot = 0;
	region.flags = 0;
	region.guest_phys_addr = 0x1000;
	region.memory_size = VM_MEMORY_SIZE;
	region.userspace_addr = (uint64_t)vm_memory;
	error = ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &region);
	if (error < 0) {
		printf("ERROR[%d]: KVM_SET_USER_MEMORY_REGION.\n", error);
		goto err_load;
	}

	/* Create Single VCPU */
	vcpu_fd = ioctl(vm_fd, KVM_CREATE_VCPU, (unsigned long)0);
	if (vcpu_fd < 0) {
		printf("ERROR[%d]: KVM_CREATE_VCPU.\n", vcpu_fd);
		error = vcpu_fd;
		goto err_load;
	}

	/* Obtain and check KVM size on Running-time */
	kvm_run_size = ioctl(kvm_fd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	if (kvm_run_size < 0 || kvm_run_size < sizeof(struct kvm_run)) {
		error = kvm_run_size > 0 ? -1 : kvm_run_size;
		printf("ERROR[%d]: KVM_GET_VCPU_MMAP_SIZE.\n", error);
		goto err_load;
	}

	/* Bind kvm_run with vcpu to obtain more running information */
	kvm_run = mmap(NULL,
		       kvm_run_size,
		       PROT_READ | PROT_WRITE,
		       MAP_SHARED,
		       vcpu_fd,
		       0);
	if (!kvm_run) {
		perror("ERROR: Bind kvm and vcpu.");
		error = -1;
		goto err_load;
	}

	/* Obtain Special Registers */
	error = ioctl(vcpu_fd, KVM_GET_SREGS, &sregs);
	if (error < 0) {
		printf("ERROR[%d]: KVM_GET_SREGS.\n", error);
		goto err_sregs;
	}
	/* Set Code/Data segment on address 0, and load OS here */
	sregs.cs.base = 0;
	sregs.cs.selector = 0;
	sregs.ss.base = 0;
	sregs.ss.selector = 0;
	sregs.ds.base = 0;
	sregs.ds.selector = 0;
	sregs.es.base = 0;
	sregs.es.selector = 0;
	sregs.fs.base = 0;
	sregs.fs.selector = 0;
	sregs.gs.base = 0;
	sregs.gs.selector = 0;
	error = ioctl(vcpu_fd, KVM_SET_SREGS, &sregs);
	if (error < 0) {
		printf("ERROR[%d]: KVM_SET_SREGS.\n", error);
		goto err_sregs;
	}

	/* Setup main entry from 0x1000 */
	memset(&regs, 0, sizeof(struct kvm_regs));
	regs.rip	= 0x0000000000001000;
	regs.rflags	= 0x0000000000000002ULL;
	regs.rsp	= 0x00000000ffffffff;
	regs.rbp	= 0x00000000ffffffff;
	error = ioctl(vcpu_fd, KVM_SET_REGS, &regs);
	if (error < 0) {
		printf("ERROR[%d]: KVM_SET_REGS.\n", error);
		goto err_sregs;
	}

	/* Running Virtual-Machine */
	while (1) {
		/* Starting */
		error = ioctl(vcpu_fd, KVM_RUN, NULL);
		if (error < 0) {
			printf("ERROR[%d]: KVM_RUN.\n", error);
			goto err_sregs;
		}

		/* Capture Exit reason */
		switch (kvm_run->exit_reason) {
		case KVM_EXIT_HLT:
			printf("KVM_EXIT_HLT\n");
			return 0;
		case KVM_EXIT_IO: {
			uint8_t data = *(char *)(((char *)(kvm_run) + 
						kvm_run->io.data_offset));
			putchar(data);
			if (data == '\n')
				sleep(6);
			break;
		}
		case KVM_EXIT_FAIL_ENTRY:
			printf("KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_"
			       "reason = %#llx", (unsigned long long)
			kvm_run->fail_entry.hardware_entry_failure_reason);
			break;
		case KVM_EXIT_INTERNAL_ERROR:
			printf("KVM_INTERNAL_ERROR: suberror = %#x\n",
					kvm_run->internal.suberror);
			break;
		default:
			break;
		}
			
	}


	/* Unbind kvm and vcpu */
	munmap(kvm_run, kvm_run_size);
	/* Free Virtual-Machine Memory */
	munmap(vm_memory, VM_MEMORY_SIZE);

	/* Close KVM */
	close(kvm_fd);

	printf("Hello Application Demo on BiscuitOS.\n");
	return 0;

err_sregs:
	munmap(kvm_run, kvm_run_size);
err_load:
	munmap(vm_memory, VM_MEMORY_SIZE);
err_huge:
	close(huge_fd);
err_version:
	close(kvm_fd);
err_open:
	return error;
}
