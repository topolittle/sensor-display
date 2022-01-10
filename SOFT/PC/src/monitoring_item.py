"""
Provides a class for storing the various sensor to monitor
"""

from monitoring_type import MonitoringType

class MonitoringItem:
    """ MonitoringItem class
    This class is used to store the an item to monitor

    Attributes
    ----------
        monitoring_type: enum, MonitoringType
            The sensor access type type: sysfs or api

        cmd: str
            The command to send to the head unit for this sensor

        display_interval: str[]
            When the monitoring type is sysfs, a collection of sysfs
            to look for this sensor information
    """

    def __init__(self, monitoring_type: MonitoringType, cmd, paths = None):
        self.monitoring_type = monitoring_type
        self.paths = paths
        self.cmd = cmd
