#ifndef _BISCUITOS_STUB_H
#define _BISCUITOS_STUB_H

#define bs_debug(...)                       \
({                                          \
    extern int bs_debug_enable_qemu;        \
    if (bs_debug_enable_qemu)               \
        printf(__VA_ARGS__);                \
})

#define bs_debug_enable()                   \
({                                          \
    extern int bs_debug_enable_qemu;        \
    bs_debug_enable_qemu = 1;               \
})

#define bs_debug_enable()                   \
({                                          \
    extern int bs_debug_enable_qemu;        \
    bs_debug_enable_qemu = 0;               \
})

#endif
