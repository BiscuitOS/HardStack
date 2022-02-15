#ifndef _BISCUITOS_DEBUG_H
#define _BISCUITOS_DEBUG_H

/* BiscuitOS Debug stub */
#define bs_debug(...)                                           \
({                                                              \
        extern int bs_debug_enable;                             \
        if (bs_debug_enable)                                    \
                pr_info(__VA_ARGS__);                           \
})                                                              \
                                                                \
#define bs_debug_enable()                                       \
({                                                              \
        extern int bs_debug_enable;                             \
        bs_debug_enable = 1;                                    \
})                                                              \
                                                                \
#define bs_debug_disable()                                      \
({                                                              \
        extern int bs_debug_enable;                             \
        bs_debug_enable = 0;                                    \
})

#endif
