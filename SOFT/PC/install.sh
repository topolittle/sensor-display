#!/bin/bash

# Installation script for the sensor-monitor service
# Author: S. Gilbert

readonly EINVAL=22      # Invalid argument 
readonly EABORT=125     # Operation aborted

# The path where we are running
readonly this_dir="$(dirname "$(readlink -f "$0")")"

# The folder where to install the stuff
readonly install_dir=/opt/sensor-monitor

# Config dir
readonly config_dir=/etc/sensor-monitor

readonly service_name=sensor-monitor.service

# Print message to stderr
# Argument 1: The message to display
print_error() {
    (>&2 printf "ERROR: $1\\n")
}

# Print message to stderr and terminate with the specified exit code
# Argument 1: The message to display
# Argument 2: The exit code
exit_error() {
    print_error "$1"
    exit $2
}

show_help() {
    echo "Usage: $(basename "$(readlink -f "$0")") [options]"
    echo "Available options:"
    echo "  -d | --disable-service  Don't automatically start and enable the service"
    echo "                          once the installation is completed"
    echo "  -h | --help             Show this help message"
    echo "  -u | --uninstall        Uninstall the service"
    echo "  -y | --accept-all       Proceed without asking"
}

# Test if the specified module is installed:
# Argument 1: The module name
test_module_installed() {
    if ! pip3 list | grep "$1" > /dev/null; then
        echo "Warning: The required module '$1' is not installed in the system. You should install it using the following command:"
        echo "sudo pip3 install $1"
    fi
}

# Ask the user to continue by entering 'yes', otherwise exit the script
accept_or_die() {
    while true; do
        read -r -p "Continue (y/n)? " choice
        case "$choice" in
            y|Y ) break;;
            n|N )
                exit_error "Operation aborted." $EABORT;;
            * );;
        esac
    done
}

install() {
    if ! $accept_all; then
        echo "This will install sensor-monitor service in the system. Do you want to continue?"
        accept_or_die
    fi

    echo "Installing sensor-monitor"
    mkdir -p "$install_dir"
    cp "$this_dir"/src/*.py "$install_dir"/
    cp "$this_dir"/src/sensor_monitor "$install_dir"/
    mkdir -p "$config_dir"
    cp "$this_dir"/config/sensor-monitor.conf "$config_dir"/
    cp "$this_dir"/systemd/$service_name /etc/systemd/system/

    # Refresh systemd daemon list
    systemctl daemon-reload
    if $disable_service; then
        echo "Skipping the service automatic startup"
    else
        echo "Enabling the service"
        systemctl enable --now $service_name
    fi

    echo "The instalation was completed successfully"
    test_module_installed "pyserial"
    test_module_installed "nvidia-ml-py"
}

uninstall() {
    if ! $accept_all; then
        echo "This will uninstall sensor-monitor service in the system. Do you want to continue?"
        accept_or_die
    fi

    echo "Stopping the service, if any"
    systemctl stop $service_name|| true
    systemctl disable $service_name || true
    systemctl daemon-reload

    echo "Uninstalling sensor-monitor"
    rm -f /etc/systemd/system/$service_name
    rm -rf "$config_dir"
    rm -rf "$install_dir"
    echo "The uninstalation was completed successfully"
}

if [ "$EUID" -ne 0 ]; then
    exit_error "You must execute this script as root"
fi

set -e
show_help_message=false
accept_all=false
uninstall=false
disable_service=false
command_count=0

while [[ $# -gt 0 ]]
do
    key="$1"
    if [[ $key == -* ]]; then
        case $key in
        -d|--disable-service)
            disable_service=true
        ;;
        -h|--help)
            show_help_message=true
        ;;
        -u|--uninstall)
            uninstall=true
        ;;
        -y|--accept-all)
            accept_all=true
        ;;
        *)
            exit_error "Unknown option: ${1}. Use --help to get help" $EINVAL
        ;;
        esac
    else
        command_count=$((command_count + 1))
        case $command_count in
            *)
                exit_error "Too many arguments: ${1}. Use --help to get help" $EINVAL
            ;;
        esac

    fi
    shift
done

if $show_help_message; then
    show_help
    exit 0
fi

if $uninstall; then
    uninstall
else
    install
fi

exit 0
