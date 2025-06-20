from __future__ import print_function

import shlex
import subprocess
import sys

from setuptools import find_packages
from setuptools import setup


version = "0.4.4"


if sys.argv[-1] == 'release':
    # Release via github-actions.
    commands = [
        'git tag v{:s}'.format(version),
        'git push origin master --tag',
    ]
    for cmd in commands:
        print('+ {}'.format(cmd))
        subprocess.check_call(shlex.split(cmd))
    sys.exit(0)


setup_requires = []

with open('requirements.txt') as f:
    install_requires = []
    for line in f:
        req = line.split('#')[0].strip()
        install_requires.append(req)

setup(
    name="i2c-for-esp32",
    version=version,
    description="I2C library for ESP32",
    author="iory",
    author_email="yanokura@jsk.imi.i.u-tokyo.ac.jp",
    url="https://github.com/iory/i2c-for-esp32",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    license="MIT",
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Natural Language :: English",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: Implementation :: CPython",
    ],
    packages=find_packages(),
    zip_safe=False,
    setup_requires=setup_requires,
    install_requires=install_requires,
)
