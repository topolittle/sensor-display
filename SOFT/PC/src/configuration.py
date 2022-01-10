
"""
Provides a class to store the app's configuration
"""

import stat
from os import lstat

DEFAULT_DISPLAY_INTERVAL = 3000
DEFAULT_POOLING_INTERVAL = 5000

class Configuration:
    """ Configuration class
    This class is used to store the configuration information

    Attributes
    ----------
        tty: str
            A string referencing the serial port to use. E.g.: /dev/ttyS0. If the string doesn't
            begin with '/dev/', it will be automatically prepended with.

        pooling_interval: int
            The interval in milliseconds at which the temperature, fan speed and power
            consumption are read and sent to the head unit.

        display_interval: int
            The interval in milliseconds at which the head unit will cycle through 'SYSTEM',
            'CPU' and 'GPU' when configured to 'AUTO'.
    """

    def __init__(self, \
        tty: str, \
        pooling_interval: int = DEFAULT_POOLING_INTERVAL, \
        display_interval: int = DEFAULT_DISPLAY_INTERVAL):
        if not tty.startswith('/dev/'):
            tty = '/dev/' + tty

        if not stat.S_ISCHR(lstat(tty)[stat.ST_MODE]):
            raise TypeError(f"{tty} is not a character device")
        self.tty = tty
        self.pooling_interval = int(pooling_interval) \
            if pooling_interval is not None else DEFAULT_POOLING_INTERVAL
        self.display_interval = int(display_interval) \
            if display_interval is not None else DEFAULT_DISPLAY_INTERVAL
