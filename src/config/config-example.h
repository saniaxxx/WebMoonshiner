#include "ds18b20.h"
#include "driver/gpio.h"

// Wifi Config
#define WIFI_SSID "access_point_name"
#define WIFI_PASSWORD "password"

//Dallas Config
#define DS18B20_GPIO_PIN       (GPIO_NUM_14) // DS18B20 pin
#define DS18B20_DEVICES_QUANTITY          (4)  // quantity of DS18B20
#define DS18B20_RESOLUTION   (DS18B20_RESOLUTION_12_BIT)
#define DS18B20_CHECK_PERIOD        (500)   // milliseconds

// Dimensions the buffer that the task being created will use as its stack.
// NOTE:  This is the number of words the stack will hold, not the number of
// bytes.  For example, if each stack item is 32-bits, and this is set to 100,
// then 400 bytes (100 * 32-bits) will be allocated.
#define STACK_SIZE 2048

// define valve pin
#define VALVE_GPIO_PIN GPIO_NUM_33
