#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#if defined(GLOBAL_DEBUG_ON)
#define PLATFORM_DEBUG_ON
#endif
#if defined(PLATFORM_DEBUG_ON)
#define PLATFORM_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)
#else
#define PLATFORM_DEBUG(format, ...)
#endif

void platform_init(void);

extern os_timer_t timer_3S;

#endif /* __PLATFORM_H__ */
