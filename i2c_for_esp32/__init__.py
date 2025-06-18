# flake8: noqa

import importlib.metadata


__version__ = importlib.metadata.version("i2c-for-esp32")


from i2c_for_esp32.wirepacker import WirePacker
from i2c_for_esp32.wireunpacker import WireUnpacker
