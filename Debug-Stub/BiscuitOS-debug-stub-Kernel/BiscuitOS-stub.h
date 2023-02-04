#ifndef _BISCUITOS_DEBUG_H
#define _BISCUITOS_DEBUG_H

extern int bs_debug_kernel_enable;
extern int bs_debug_kernel_enable_one;

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

#define is_bs_enable()	bs_debug_kernel_enable

#endif
