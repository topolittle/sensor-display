/* Source code for the sensor display head unit
 * Distributed under the GPLv2 License
 * Author: S. Gilbert
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include "sensors_display.h"

#pragma region global

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_WIDTH, LCD_HEIGHT);
SensorItem CurrentSensorItem = (SensorItem)0;
ConfigItem CurrentConfigItem = (ConfigItem)0;
DisplayMode CurrentDisplayMode = DisplayMode::sensorCycle;
LedState CurrentLedState = LedState::High;
DisplayMode OldDisplayMode = CurrentDisplayMode;

/* Indicate wathever we need to update the display or not.
 * Should be set to true each time the display should be updated */
bool UpdateDisplay = true;

/* This indicate if we have received our first set of data
 * one it's set to true, it remains true */
bool DataAvailable = false;

/* The periode at which the CPU, GPU and system stats will cycle on the display */
u16 StatsDisplayTimeMs = STATS_DISPLAY_DEFAULT_TIME_MS;

// Defint an init values
u8 CPUTemp = 0;
u16 CPUFan = 0;
u8 GPUTemp = 0;
u8 GPUFan = 0;
u16 GPUPower = 0;
u8 SystemTemp = 0;
u16 SystemFan = 0;

#pragma endregion

/* Sets the button led according to the CurrentLedState variable */
void SetLed()
{
    switch (CurrentLedState)
    {
    case LedState::Off:
        pinMode(INPUT_BUTTON_LED_HIGH, INPUT);
        pinMode(INPUT_BUTTON_LED_LOW, OUTPUT);
        digitalWrite(INPUT_BUTTON_LED_LOW, LOW);
        break;
    case LedState::Low:
        pinMode(INPUT_BUTTON_LED_HIGH, INPUT);
        pinMode(INPUT_BUTTON_LED_LOW, OUTPUT);
        digitalWrite(INPUT_BUTTON_LED_LOW, HIGH);
        break;
    case LedState::High:
        pinMode(INPUT_BUTTON_LED_LOW, INPUT);
        pinMode(INPUT_BUTTON_LED_HIGH, OUTPUT);
        digitalWrite(INPUT_BUTTON_LED_HIGH, HIGH);
        break;
    }
}

/* Compute a parity on the EEProm data.
 * This is a very simple way to asses the integrity
 * of the stored data
 * @param eeprom data to compute
 * @return the calculated parity on 'eeprom' up to but the last byte */
u8 ComputeEepromParity(EepromData eeprom)
{
    u8 par = 0xff;
    u8 *eeprom_p = (u8 *)&eeprom;
    for (u16 i = 0; i < eeprom.Size - sizeof(eeprom.Pariry); i++)
    {
        par ^= eeprom_p[i];
    }

    return par;
}

/* Save the current config state to the EEPRom
 * @return a EepromData struct representing the saved data */
EepromData StoreDataToEeprom()
{
    EepromData eeprom;
    eeprom.Version = CURRENT_EEPROM_DATA_VERSION;
    eeprom.Size = sizeof(EepromData);
    eeprom.SelectedDisplayMode = (u8)CurrentDisplayMode;
    eeprom.SelectedSensorItem = (u8)CurrentSensorItem;
    eeprom.LedState = (u8)CurrentLedState;
    eeprom.StatsDisplayTimeMs = StatsDisplayTimeMs;
    eeprom.Pariry = ComputeEepromParity(eeprom);
    EEPROM.put(0, eeprom);
    return eeprom;
}

/* Load the previously saved state from the EEProm
* Validate version and data integrity and reset the
* EEProm data to a know state if they are not valid. */
void PopulateDataFromEeprom()
{
    EepromData eeprom;
    EEPROM.get(0, eeprom);

    if (eeprom.Version != CURRENT_EEPROM_DATA_VERSION || ComputeEepromParity(eeprom) != eeprom.Pariry)
    {
        // Reset the eeprom to a know state
        eeprom = StoreDataToEeprom();
    }

    CurrentDisplayMode = (DisplayMode)eeprom.SelectedDisplayMode;
    CurrentSensorItem = (SensorItem)eeprom.SelectedSensorItem;
    CurrentLedState = (LedState)eeprom.LedState;
    StatsDisplayTimeMs = eeprom.StatsDisplayTimeMs;
}

/* Read the push button and debounce its state
 * @return true if the button was activated and it's
 * fully debounced. Otherwise false. */
bool isButtonDebounced()
{
    static unsigned long lastDebounceTime = 0;
    static int buttonState = HIGH;
    static int lastButtonState = buttonState;
    bool state = false;
    int buttonReading;

    // Read the input button and debounce it
    buttonReading = digitalRead(INPUT_BUTTON);
    if (buttonReading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    lastButtonState = buttonReading;
    if ((millis() - lastDebounceTime) > DEBOUNCE_TIME_MS && buttonReading != buttonState)
    {
        buttonState = buttonReading;
        if (buttonState == BUTTON_PUSHED)
        {
            state = true;
        }
    }

    return state;
}

/* Look for data on the serial port and process it if present.
 * The data has the forms of CCV..V; where:
 * - 'CC' is a two chars command code
 * - 'V' is one or more numeric char that form the value associated with the command
 * - ';' is a marker char that signal the end of a command.
 * Multiple command can be received on the same line, for example the
 * command 'ST34;CT55;GT67;CF1152;' without quote will set the system temperature
 * to 34 celcius, the CPU temperature to 55 celcius and the CPU fan speed to 1152 RPM.
 *
 *  Here is the supported serial commands.
 *
 *  Sets CPU temperature in Celcius
 *  CT<cpu_temp>;
 *
 *  Sets CPU fan in rotation per minutes
 *  CF<fan_rpm>;
 *
 *  Sets GPU temperature in Celcius
 *  GT<gpu_temp>;
 *
 *  Sets GPU fan percentage
 *  GF<fan_percent>;
 *
 *  Sets GPU power consumption
 *  GP<gpu_power>;
 *
 *  Sets the system temperature in Celcius
 *  ST<system_temp>;
 *
 *  Sets the system fan in rotation per minutes
 *  SF<system_fan>;
 * 
 * Sets the periode at which the CPU, GPU and system stats will cycle on the display, in milliseconds
 * DT<periode_ms>; */
void processSerialData()
{
    if (Serial1.available() > 0)
    {
        String serData = Serial1.readStringUntil(';');
        String serCommand = serData.substring(0, 2);
        long serValue = serData.substring(2).toInt();
        DataAvailable = true;

        if (CurrentDisplayMode != DisplayMode::config)
        {
            UpdateDisplay = true;
        }

        if (serCommand == "CT")
        {
            CPUTemp = (u8)serValue;
        }
        else if (serCommand == "CF")
        {
            CPUFan = (u16)serValue;
        }
        else if (serCommand == "GT")
        {
            GPUTemp = (u8)serValue;
        }
        else if (serCommand == "GF")
        {
            GPUFan = (u8)serValue;
        }
        else if (serCommand == "GP")
        {
            GPUPower = (u16)serValue;
        }
        else if (serCommand == "ST")
        {
            SystemTemp = (u8)serValue;
        }
        else if (serCommand == "SF")
        {
            SystemFan = (u16)serValue;
        }
        else if (serCommand == "DT")
        {
            StatsDisplayTimeMs = (u16)serValue;
            StoreDataToEeprom();
        }
    }
}

/* Display an amount of blank char to the LCD.
 * This can be useful to delete the remaining of the line
 * without using lcd.clear() beforehand
 * @param count the number of blank chars to display */
void DisplayBlankChar(u8 count)
{
    for (u8 i = 0; i < count; i++)
    {
        lcd.print(" ");
    }
}

/* Display the current configuration item.
 * Also, check whatever a timeout has occured and if so,
 * register the new configuration */
void ManageDisplayConfig()
{
    static unsigned long lastConfigTime = 0;

    if (UpdateDisplay)
    {
        lcd.clear();
        if ((u8)CurrentConfigItem <= (u8)ConfigItem::cycle)
        {
            lcd.setCursor(4, 0);
            lcd.print("DISPLAY");
        }
        else
        {
            lcd.setCursor(3, 0);
            lcd.print("BUTTON LED");
        }

        switch (CurrentConfigItem)
        {
        case ConfigItem::cpu:
            lcd.setCursor(6, 1);
            lcd.print("CPU");
            break;
        case ConfigItem::gpu:
            lcd.setCursor(6, 1);
            lcd.print("GPU");
            break;
        case ConfigItem::system:
            lcd.setCursor(5, 1);
            lcd.print("SYSTEM");
            break;
        case ConfigItem::allTemp:
            lcd.setCursor(3, 1);
            lcd.print("ALL (TEMP)");
            break;
        case ConfigItem::allFan:
            lcd.setCursor(3, 1);
            lcd.print("ALL (FAN)");
            break;
        case ConfigItem::cycle:
            lcd.setCursor(5, 1);
            lcd.print("AUTO");
            break;
        case ConfigItem::buttonLedOff:
            lcd.setCursor(6, 1);
            lcd.print("OFF");
            break;
        case ConfigItem::buttonLedLow:
            lcd.setCursor(6, 1);
            lcd.print("LOW");
            break;
        case ConfigItem::buttonLedHigh:
            lcd.setCursor(5, 1);
            lcd.print("HIGH");
            break;
        }

        UpdateDisplay = false;
        lastConfigTime = millis();
    }
    else if ((millis() - lastConfigTime) > CONFIG_DISPLAY_TIME_MS)
    {
        // Reset the display to sensors and set the new config
        switch (CurrentConfigItem)
        {
        case ConfigItem::cpu:
            CurrentDisplayMode = DisplayMode::sensor;
            CurrentSensorItem = SensorItem::cpu;
            break;
        case ConfigItem::gpu:
            CurrentDisplayMode = DisplayMode::sensor;
            CurrentSensorItem = SensorItem::gpu;
            break;
        case ConfigItem::system:
            CurrentDisplayMode = DisplayMode::sensor;
            CurrentSensorItem = SensorItem::system;
            break;
        case ConfigItem::allTemp:
            CurrentDisplayMode = DisplayMode::sensor;
            CurrentSensorItem = SensorItem::allTemp;
            break;
        case ConfigItem::allFan:
            CurrentDisplayMode = DisplayMode::sensor;
            CurrentSensorItem = SensorItem::allFan;
            break;
        case ConfigItem::cycle:
            CurrentDisplayMode = DisplayMode::sensorCycle;
            CurrentSensorItem = (SensorItem)0;
            break;
        case ConfigItem::buttonLedOff:
            CurrentDisplayMode = OldDisplayMode;
            CurrentLedState = LedState::Off;
            SetLed();
            break;
        case ConfigItem::buttonLedLow:
            CurrentDisplayMode = OldDisplayMode;
            CurrentLedState = LedState::Low;
            SetLed();
            break;
        case ConfigItem::buttonLedHigh:
            CurrentDisplayMode = OldDisplayMode;
            CurrentLedState = LedState::High;
            SetLed();
            break;
        }

        UpdateDisplay = true;
        StoreDataToEeprom();
    }
}

/* Display the sensor value according to the current configuration.
 * When the configuration is set to 'sensorCycle', it also cycle to
 * every sensors */
void ManageDisplaySensor()
{
    static unsigned long lastChangeTime = 0;
    char line1[(LCD_WIDTH + 1) * sizeof(char)];
    char line2[(LCD_WIDTH + 1) * sizeof(char)];
    char gpuChar[4];

    if (CurrentDisplayMode == DisplayMode::sensorCycle && ((millis() - lastChangeTime) > ((unsigned long)StatsDisplayTimeMs)))
    {
        // Here we have to move to another sensor
        lastChangeTime = millis();
        CurrentSensorItem = (SensorItem)((u8)(CurrentSensorItem) + 1);
        if (CurrentSensorItem == SensorItem::allTemp)
            CurrentSensorItem = (SensorItem)0;
        UpdateDisplay = true;
    }

    if (UpdateDisplay)
    {
        memset(line1, '\0', sizeof(char));
        memset(line2, '\0', sizeof(char));
        if (DataAvailable)
        {
            // Prepare the message to be displayed
            sprintf(line1, "    tmp fan");
            if (GPUFan >= 100)
            {
                sprintf(gpuChar, "MAX");
            }
            else
            {
                sprintf(gpuChar, "%2d%%", GPUFan % 100);
            }

            switch (CurrentSensorItem)
            {
            case SensorItem::cpu:
                sprintf(line2, "CPU %2dC %d rpm", CPUTemp % 100, CPUFan % 10000);
                break;
            case SensorItem::gpu:
                sprintf(line1, "    tmp fan pwr");
                sprintf(line2, "GPU %2dC %s %dw", GPUTemp % 100, gpuChar, GPUPower % 1000);
                break;
            case SensorItem::system:
                sprintf(line2, "SYS %2dC %d rpm", SystemTemp % 100, SystemFan % 10000);
                break;
            case SensorItem::allTemp:
                sprintf(line1, " SYS  CPU  GPU");
                sprintf(line2, " %2dC  %2dC  %2dC", SystemTemp % 100, CPUTemp % 100, GPUTemp % 100);
                break;
            case SensorItem::allFan:
                sprintf(line1, " SYS   CPU   GPU");
                sprintf(line2, "%4d  %4d   %s", SystemFan % 10000, CPUFan % 10000, gpuChar);
                break;
            }
        }
        else
        {
            sprintf(line1, " AWAITING DATA");
        }

        lcd.setCursor(0, 0);
        lcd.print(line1);
        DisplayBlankChar(LCD_WIDTH - strlen(line1));
        lcd.setCursor(0, 1);
        lcd.print(line2);
        DisplayBlankChar(LCD_WIDTH - strlen(line2));
        UpdateDisplay = false;
    }
}

/* Display either the configuration or the sensors
 * according to the current display mode */
void ManageDisplay()
{
    if (CurrentDisplayMode == DisplayMode::config)
    {
        ManageDisplayConfig();
    }

    ManageDisplaySensor();
}

void setup()
{
    lcd.init();
    lcd.clear();
    lcd.backlight();

    pinMode(INPUT_BUTTON, INPUT_PULLUP);
    Serial1.begin(115200);
    PopulateDataFromEeprom();
    SetLed();
}

void loop()
{
    while (true)
    {
        if (isButtonDebounced())
        {
            if (CurrentDisplayMode != DisplayMode::config)
            {
                OldDisplayMode = CurrentDisplayMode;
                CurrentDisplayMode = DisplayMode::config;
                CurrentConfigItem = (ConfigItem)0;
            }
            else
            {
                CurrentConfigItem = (ConfigItem)((u8)CurrentConfigItem + 1);
                if (CurrentConfigItem == ConfigItem::endEnum)
                {
                    CurrentConfigItem = (ConfigItem)0;
                }
            }

            UpdateDisplay = true;
        }

        ManageDisplay();
        processSerialData();
    }
}
