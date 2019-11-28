/*
 * System Call: open
 *
 * (C) 2019.11.28 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* In order to get __set_errno() definition in INLINE_SYSCALL. */
#include <errno.h>
# define __set_errno(val)	(errno = (val))

/* 0 args */
#define LOAD_ARGS_0()
#define ASM_ARGS_0

/* 1 args */
#define LOAD_ARGS_1(a1)							\
	int _a1tmp = (int) (a1);					\
	LOAD_ARGS_0 ()							\
	_a1 = _a1tmp;
#define ASM_ARGS_1	ASM_ARGS_0, "r" (_a1)

/* 2 args */
#define LOAD_ARGS_2(a1, a2)						\
	int _a2tmp = (int) (a2);					\
	LOAD_ARGS_1 (a1)						\
	register int _a2 asm ("a2") = _a2tmp;
#define ASM_ARGS_2	ASM_ARGS_1, "r" (_a2)

/* 3 args */
#define LOAD_ARGS_3(a1, a2, a3)						\
	int _a3tmp = (int) (a3);					\
	LOAD_ARGS_2 (a1, a2)						\
	register int _a3 asm ("a3") = _a3tmp;
#define ASM_ARGS_3	ASM_ARGS_2, "r" (_a3)

/* 4 args */
#define LOAD_ARGS_4(a1, a2, a3, a4)					\
	int _a4tmp = (int) (a4);					\
	LOAD_ARGS_3 (a1, a2, a3)					\
	register int _a4 asm ("a4") = __a4tmp;
#define ASM_ARGS_4	ASM_ARGS_3, "r" (_a4)

/* 5 args */
#define LOAD_ARGS_5(a1, a2, a3, a4, a5)					\
	int _v1tmp = (int) (a5);					\
	LOAD_ARGS_4 (a1, a2, a3, a4)					\
	register int _v1 asm ("v1") = _v1tmp;
#define ASM_ARGS_5	ASM_ARGS_4, "r" (_v1)

/* 6 args */
#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)				\
	int _v2tmp = (int) (a6);					\
	LOAD_ARGS_5 (a1, a2, a3, a4, a5)				\
	register int _v2 asm ("v2") = _v2tmp;
#define ASM_ARGS_6	ASM_ARGS_5, "r" (_v2)

/* 7 args */
#define LOAD_ARGS_7(a1, a2, a3, a4, a5, a6, a7)				\
	int _v3tmp = (int) (a7);					\
	LOAD_ARGS_6 (a1, a2, a3, a4, a5, a6)				\
	register int _v3 asm ("v3") = _v3tmp;
#define ASM_ARGS_7	ASM_ARGS_6, "r" (_v3)


#undef INTERNAL_SYSCALL_RAW
#define INTERNAL_SYSCALL_RAW(name, err, nr, args...)			\
	({								\
		register int _a1 asm ("r0"), _nr asm ("r7");		\
		LOAD_ARGS_##nr	(args)					\
		_nr = name;						\
		asm volatile ("swi	0x0	@ syscall " #name	\
			      : "=r" (_a1)				\
			      : "r"  (_nr) ASM_ARGS_##nr		\
			      : "memory");				\
		_a1;							\
	})

/* For Linux we can use the system call table in the hander fiel
 * 	/usr/include/asm/unistd.h
 * of the kerne. But these symbol do not follow the SYS_* syntax
 * so we have to redefine the 'SYS_ify' macro here.
 */
#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...)			\
	INTERNAL_SYSCALL_RAW(SYS_ify(name), err, nr, args)

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err)				\
	((unsigned int) (val) >= 0xfffff001u)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

/* Define a macro which expands into the inline wrapper code for
 * a system call.
 */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)				\
	({								\
		unsigned int _sys_result = 				\
				INTERNAL_SYSCALL (name, , nr, args);	\
     		if (__builtin_expect (					\
			INTERNAL_SYSCALL_ERROR_P (_sys_result, ), 0)) {	\
			__set_errno (INTERNAL_SYSCALL_ERRNO (		\
						_sys_result, ));	\
			_sys_result = (unsigned int) -1;		\
		}							\
		(int) _sys_result; 					\
	})

/* Syscall nr on arch/arm/tools/syscall.tbl */
#define __NR_BiscuitOS_open	400
/* flags */
#define O_ACCMODE		00000003
#define O_RDONLY		00000000
#define O_WRONLY		00000001
#define O_RDWR			00000002

#define BiscuitOS_open(name, flags, mode)				\
	INLINE_SYSCALL(BiscuitOS_open, 3, name, flags, mode)
#define BiscuitOS_open(name, flags)					\
	INLINE_SYSCALL(BiscuitOS_open, 2, name, flags)

int main()
{
	BiscuitOS_open("BiscuitOS.cfg", O_RDWR);
	return 0;
}
