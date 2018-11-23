#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define ICACHE_STORE_TYPEDEF_ATTR __attribute__((aligned(4),packed))
#define ICACHE_STORE_ATTR __attribute__((aligned(4)))
#define ICACHE_RAM_ATTR

#define USE_OPTIMIZE_PRINTF

#define VOWSTAR_IO_DEBUG_ON

#define PLUG_86

#ifdef PLUG_86

//#define PIN_LED 			12
#define PIN_LED_S			14
#define PIN_KEY 			0
#define PIN_POWER           9

#endif // PLUG_86

#endif
