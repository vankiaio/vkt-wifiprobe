#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR

#define USE_OPTIMIZE_PRINTF

#define VOWSTAR_IO_DEBUG_ON

#define PLUG_86

#ifdef PLUG_86





#define PIN_POWER           9


#ifdef OLED_VERSION
    #define OLED_POWER          14
#else
    #define PIN_LED_S           5
    #define PIN_GPS_S           10
    #define PIN_POWER           9
#endif



#endif // PLUG_86

#endif
