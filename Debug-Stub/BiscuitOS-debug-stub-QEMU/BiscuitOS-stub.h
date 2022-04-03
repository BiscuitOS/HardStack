#ifndef _BISCUITOS_STUB_H
#define _BISCUITOS_STUB_H

extern int bs_debug_enable_qemu;

#define bs_debug(...)                       \
({                                          \
    if (bs_debug_enable_qemu)               \
        printf(__VA_ARGS__);                \
})

#define bs_debug_enable()                   \
({                                          \
    bs_debug_enable_qemu = 1;               \
})

#define bs_debug_disable()                  \
({                                          \
    bs_debug_enable_qemu = 0;               \
})

#endif
