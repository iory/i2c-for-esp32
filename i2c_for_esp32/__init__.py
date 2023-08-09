# flake8: noqa

import pkg_resources


__version__ = pkg_resources.get_distribution("i2c-for-esp32").version


from i2c_for_esp32.wirepacker import WirePacker
from i2c_for_esp32.wireunpacker import WireUnpacker
