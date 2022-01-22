# SENSOR DISPLAY UNIT - EMBEDDED CODE
This is the code for the sensor display unit.
It target the Keystudio Ks0503 Pro Micro board but any Atmel ATMEGA 32U4 Arduino
board with an Leonardo bootloader should do the job. It's designed to fit in
a standard 5.25" PC drive bay.  
The device is powered by a single Molex 4 pin PC power connecter and it gets its data from a service running on the PC through an RS232 serial connection.

Please refers to the schematic diagram for the wiring of the board.
For the complete part list, refers to the bill of material document (BOM).

## Features
The unit has two modes of operation:
- Sensor display
- Configuration
In the sensor display mode, the display will display the current stored sensors values.
In the configuration mode, the display will cycle through all the available settings. Refers to the 'settings' section for more information.

## Settings
When pushing the button, the unit enters the configuration mode. Each subsequent button push will cycle to the next setting. Here is the list of all available configuration:
- SYSTEM: Display the system temperature and fan RPM
- CPU: Display the CPU temperature and fan RPM
- GPU: Display the GPU temperature, fan percentage and power consumption
- ALL (TEMP): Display the system, CPU and GPU temperature
- ALL (FAN): Display the system, CPU and GPU fan RPM or percentage
- AUTO: Cycle through SYSTEM, CPU and GPU every 3 seconds
- BUTTON LED OFF: Turn off the button blue LEDssss
- BUTTON LED HIGH: Turn on the button blue LED at high intensity
- BUTTON LED LOW: Turn on the button blue LED at low intensity

After 3 seconds of inactivity, the unit will revert to sensor display mode, the new configuration is saved to the persistent memory (EEProm) and it take effect immediately.

## Dependencies
The code requires the following libraries to be installed:  
**LiquidCrystal I2C**  
Version: 1.1.2  
Author: Frank de Brabander  
URL: https://github.com/marcoschwartz/LiquidCrystal_I2C

**Wire (Build-in library)**  
Version: 1.0.0  
Author: Nicholas Zambetti  
URL: https://www.arduino.cc/en/Reference/Wire

**EEPROM (Build-in library)**  
Version: 2.0.0  
Authors: David A. Mellis. and Christopher Andrews  
URL: https://www.arduino.cc/en/Reference/EEPROM

Note: If your are using the included VSCode devcontainer, libraries are already included and installed.

# Serial commands
Each serial command has the forms of `CCV..V;` where:
- 'CC' is a two chars command code
- 'V' is one or more numeric char that form the value associated with the command
- ';' is a marker char and signals the end of a command  
 Multiple command can be received on the same line, for example the
 command `ST34;CT55;GT67;CF1152;` will sets the system temperature
 to 34 celcius, the CPU temperature to 55 celcius and the CPU fan speed to 1152 RPM.  
 Refer to the source code to the complete list of accepted commands.
 
# Using with Visual Studio Code
## Using the devcontainer
The devcontainer is a self-contained environment that allows to compiles and upload
Arduino source code without having to install anything on your computer. You just need Docker.
[More Information about VSCode devcontainer here.](https://code.visualstudio.com/docs/remote/containers)
1. Open this folder (where this README.MD is located)
2. Hit Shift-CTRL-P  to open the command palette
3. Type and select: Remote-Containers: Reopen in Container

## Inside the devcontainer
You will have access to intellisense, syntax highlighting, build, upload and many more.
Take a look [here](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino) for the available features.

### Commands in a nutshell
**Build / Verify:** ALT+CTRL+R  
**Upload:** ALT+CTRL+U
