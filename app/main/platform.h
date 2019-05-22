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
void delay_power_on(void);

extern uint8_t *  ap_str;

extern os_timer_t check_id_timer;
extern os_timer_t restart_nb;
extern os_timer_t err_restart;
extern os_timer_t timer_300s;
extern os_timer_t scan_timer;
extern os_timer_t delay_update_timer;
extern os_timer_t delay_discon_timer;
extern os_timer_t wait_nb_con_timer;

extern os_timer_t timer_wait_con_wifi;
//#define OLED


#endif /* __PLATFORM_H__ */
