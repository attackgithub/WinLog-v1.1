#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess

def install_pip_and_pillow():

    subprocess.call("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\scripts\\get-pip.py --quiet", shell=True)

    subprocess.call("pip install --quiet --upgrade pip", shell=True)

    subprocess.call("pip install --quiet pillow", shell=True)

install_pip_and_pillow()
