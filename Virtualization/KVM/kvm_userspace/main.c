/*
 * KVM
 *
 * (C) 2020.04.07 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
/* KVM */
#include <linux/kvm.h>

static int kvm(uint8_t code[], size_t code_len)
{
	size_t mem_size;
	size_t vcpu_mmap_size;
	void *mem;
	int kvmfd, vmfd;
	int user_entry = 0x0;
	int vcpufd;
	struct kvm_userspace_memory_region region = {
		.slot = 0,
		.flags = 0,
		.guest_phys_addr = 0,
		.memory_size = mem_size,
		.userspace_addr = (size_t)mem,
	};
	struct kvm_run *run;
	struct kvm_regs regs;
	struct kvm_sregs sregs;
	int ret;

	/* Open KVM */
	kvmfd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
	if (kvmfd < 0) {
		printf("Open /dev/kvm failed: %d\n", errno);
		return -1;
	}

	/* Create KVM */
	vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);
	if (vmfd < 0) {
		printf("IOCTL KVM_CREATE_VM failed: %d\n", errno);
		goto out;
	}

	/* setup user memory region */
	mem_size = 0x40000000; /* 1G */
	mem = mmap(0, mem_size, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (!mem) {
		printf("MMAP: Error on mem. %d\n", errno);
		goto out;
	}

	memcpy((void *)((size_t)mem + user_entry), code, code_len);
	ret = ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);
	if (ret < 0) {
		printf("IOCTL: KVM_SET_USER_MEMORY_REGION: %d\n", errno);
		goto out_mmap;
	}

	/* Create and setup vCPU */
	vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
	if (vcpufd < 0) {
		printf("IOCTL: KVM_CREATE_VCPU: %d\n", errno);
		goto out_mmap;
	}

	/* setup up memory for vCPU */
	vcpu_mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
	if (vcpu_mmap_size < 0) {
		printf("IOCTL: KVM_GET_VCPU_MMAP_SIZE: %d\n", errno);
		goto out_mmap;
	}
	run = (struct kvm_run *)mmap(0, vcpu_mmap_size, PROT_READ | PROT_WRITE,
					MAP_SHARED, vcpufd, 0);
	if (!run) {
		printf("MMAP: Error on run: %d\n", errno);
		goto out_mmap;
	}

	/* setup vCPU's register */
	ret = ioctl(vcpufd, KVM_GET_REGS, &regs);
	if (ret < 0) {
		printf("IOCTL: KVM_GET_REGS: %d\n", errno);
		goto out_mmap2;
	}
	regs.rip = user_entry;
	/* stack address */
	regs.rsp = 0x200000;
	/* in x86 the 0x2 bit should always be set */
	regs.rflags = 0x2;
	ret = ioctl(vcpufd, KVM_SET_REGS, &regs);
	if (ret < 0) {
		printf("IOCTL: KVM_SET_REGS: %d\n", errno);
		goto out_mmap2;
	}

	/* Special registers include segment registers */
	ret = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
	if (ret < 0) {
		printf("IOCTL: KVM_GET_SREGS: %d\n", errno);
		goto out_mmap2;
	}
	/* let base of code segment sequal to zero */
	sregs.cs.base = sregs.cs.selector = 0;
	ret = ioctl(vcpufd, KVM_SET_SREGS, &sregs);
	if (ret < 0) {
		printf("IOCTL: KVM_SET_SREGS: %d\n", errno);
		goto out_mmap2;
	}
	
	/* execute vm and handle exit reason */
	while (1) {
		ret = ioctl(vcpufd, KVM_RUN, NULL);
		if (ret < 0) {
			printf("IOCTL: KVM_RUN: %d\n", errno);
			goto out_mmap2;
		}

		switch (run->exit_reason) {
		case KVM_EXIT_HLT:
			printf("KVM_EXIT_HLT\n");
			goto OK;
		case KVM_EXIT_IO:
			/* TODO: check port and direction here */
			putchar(*(((char *)run) + run->io.data_offset));
			break;
		case KVM_EXIT_FAIL_ENTRY:
			printf("KVM_EXIT_FAIL_ENTRY: "
				"hardware_entry_failure_reason = %#llx\n",
				run->fail_entry.hardware_entry_failure_reason);
		case KVM_EXIT_INTERNAL_ERROR:
			printf("KVM_EXIT_INTERNAL_ERROR: suberror = %#x\n",
					run->internal.suberror);
		case KVM_EXIT_SHUTDOWN:
			printf("KVM_EXIT_SHUTDOWN\n");
		default:
			printf("Unhandled reason: %d\n", run->exit_reason);
		}
	}

OK:
	munmap((void *)run, vcpu_mmap_size);
	munmap((void *)mem, mem_size);
	/* close */
	close(kvmfd);
	printf("KVM close OK...\n");

	return 0;
out_mmap2:
	munmap((void *)run, vcpu_mmap_size);
out_mmap:
	munmap((void *)mem, mem_size);
out:
	close(kvmfd);
	return -1;
}

int main(void)
{
	uint8_t code[] = "\x80\x61\xBA\x17\x02\xEE\xB0\n\xEE\xF4";

	kvm(code, sizeof(code));

	return 0;
}
