#ifndef _BISCUITOS_DEBUG_H
#define _BISCUITOS_DEBUG_H

/* BiscuitOS Debug stub */
#define bs_debug(...)						\
({								\
	extern int bs_debug_enable;				\
	if (bs_debug_enable)					\
		pr_info(__VA_ARGS__);				\
})

#endif
