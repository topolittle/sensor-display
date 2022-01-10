#ifndef SENSOR_DISPLAY_H_
#define SENSOR_DISPLAY_H_

#define LCD_I2C_ADDR 0x27
#define LCD_WIDTH 16 // The number of char per line the lcd has
#define LCD_HEIGHT 2 // The number of lines the lcd has

#define INPUT_BUTTON_LED_HIGH 5
#define INPUT_BUTTON_LED_LOW 4
#define INPUT_BUTTON 6
#define DEBOUNCE_TIME_MS 30L
#define BUTTON_PUSHED LOW
#define BUTTON_RELEASED HIGH

#define STATS_DISPLAY_DEFAULT_TIME_MS 3000
#define CONFIG_DISPLAY_TIME_MS 3000

#define CURRENT_EEPROM_DATA_VERSION 1

/* Enum that define what sensor reading to display */
enum class SensorItem : u8 {
    system = 0,
    cpu,
    gpu,
    allTemp,
    allFan,
    endEnum
};

/* Enum that define the configuration selection */
enum class ConfigItem : u8 {
    system = 0,
    cpu,
    gpu,
    allTemp,
    allFan,
    cycle,
    buttonLedOff,
    buttonLedHigh,
    buttonLedLow,
    endEnum
};

/* Enum that define the display modes */
enum class DisplayMode : u8 {
    sensor = 0,
    sensorCycle,
    config
};

/* Enum that define the various led states */
enum class LedState : u8 {
    Off = 0,
    Low ,
    High
};

/* Represent the eeprom data structure
 * Add any new item before the parity field. */
struct EepromData
{
    u8 Version;
    u8 Size;
    u8 SelectedDisplayMode;
    u8 SelectedSensorItem;
    u8 LedState;
    u16 StatsDisplayTimeMs;
    u8 Pariry;
};

#endif // SENSOR_DISPLAY_H_
