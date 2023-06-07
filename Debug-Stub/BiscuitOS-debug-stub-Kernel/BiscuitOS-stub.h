#ifndef _BISCUITOS_DEBUG_H
#define _BISCUITOS_DEBUG_H

extern int bs_debug_kernel_enable;
extern int bs_debug_kernel_enable_one;
extern unsigned long bs_debug_async_data;

/* BiscuitOS Debug stub */
#define bs_debug(...)                                           \
({                                                              \
        if (bs_debug_kernel_enable && bs_debug_kernel_enable_one) \
		pr_info("[BiscuitOS-stub] " __VA_ARGS__);       \
})

#define bs_kdebug(...)                                           \
({                                                               \
	pr_info("[BiscuitOS-stub] " __VA_ARGS__);                \
})

#define bs_debug_enable()                                       \
({                                                              \
        bs_debug_kernel_enable = 1;                             \
        bs_debug_kernel_enable_one = 1;                         \
})                                                              \

#define bs_debug_disable()                                      \
({                                                              \
        bs_debug_kernel_enable = 0;                             \
        bs_debug_kernel_enable_one = 0;                         \
})

#define bs_debug_enable_one()                                   \
({                                                              \
        bs_debug_kernel_enable_one = 1;                         \
})                                                              \

#define bs_debug_disable_one()                                  \
({                                                              \
        bs_debug_kernel_enable_one = 0;                         \
})

#define bs_debug_async_enable(x)				\
({								\
	if ((unsigned long)x == bs_debug_async_data) 		\
		bs_debug_enable();				\
	else							\
		bs_debug_disable();				\
})

#define is_bs_enable()	bs_debug_kernel_enable

#endif
