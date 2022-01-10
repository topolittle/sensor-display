# SENSOR MONITOR SERVICE
This is the source code for the service that sends the sensor information
of the CPU, GPU and System to the head unit over a serial port.

## Features
- Linux support
- SystemD unit configuration included
- Install/Uninstall script included
- Fully customizable using a configuration file
- Verbose option for debugging

## Depencies
The code was tested using Python 3.9. However, it may works with an earlier version.
The following Python libraries are required to be installed in the system prior the service installation:
- pyserial
- nvidia-ml-py

## Installation
Simply run the install script as root.  
The script will deploy the following:
- The main sensor_monitor program and its compagnon files to /opt/sensor-monitor
- The configuration file in /etc/sensor-monitor
- The SystemD service unit in /etc/systemd/system  

The script will start and enable the service to start when the system boot.

Note: The install script didn't install the required Python dependencies by itself. You have to install it manually with the `pip3` command.  

Once the installation is completed, you may want to review the configuration file and adjust it to match your system configuration.

## Development
This project includes a VScode devcontainer based on Alpine. Currently the serial port is working from inside the container, but access to the GPU API is not yes supported.

## Future work
- Can be adapted to behave as a daemon using the python-daemon library which implement PEP 3143. This would allows sensor_monitor to work with SysV init.
- Add support for AMD GPUs
- Add a python setup to deploy the compagnon modules in the site-packages folder
- Add support for the GPU API in the devcontainer
