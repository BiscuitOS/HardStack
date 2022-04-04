#ifndef _BISCUITOS_DEBUG_H
#define _BISCUITOS_DEBUG_H

extern int bs_debug_kernel_enable;

/* BiscuitOS Debug stub */
#define bs_debug(...)                                           \
({                                                              \
        if (bs_debug_kernel_enable)                             \
                pr_info(__VA_ARGS__);                           \
})

#define bs_debug_kernel_enable()                                \
({                                                              \
        bs_debug_kernel_enable = 1;                             \
})                                                              \

#define bs_debug_disable()                                      \
({                                                              \
        bs_debug_kernel_enable = 0;                             \
})

#define bs_kdebug(...)                                           \
({                                                               \
	pr_info("[BiscuitOS-stub] " __VA_ARGS__);                \
})

#endif
