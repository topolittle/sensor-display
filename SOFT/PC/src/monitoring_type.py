"""
Provides enum for the different sensor sources
"""

from enum import Enum, auto

class MonitoringType(Enum):
    """
    Emum for the different sensor sources
    """

    SYSFS = auto()
    NVIDIA_API = auto()
    AMD_API = auto()
