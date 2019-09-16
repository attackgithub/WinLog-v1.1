#!/usr/bin/env python
# -*- coding: utf-8 -*-

from PIL import Image

def convert_to_jpeg() :
    image = Image.open('BaseBrd.bmp')
    image.save('BaseBrd.jpg', "JPEG")

convert_to_jpeg()
