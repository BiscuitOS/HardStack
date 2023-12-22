#ifndef _BISCUTIOS_MEMORY_FLUID_H
#define _BISCUTIOS_MEMORY_FLUID_H

#define BiscuitOS_memory_fluid_enable()		syscall(600, 1)
#define BiscuitOS_memory_fluid_disable()	syscall(600, 0)

#endif
