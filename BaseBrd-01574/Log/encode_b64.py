#!/usr/bin/env python
# -*- coding : utf-8 -*-

import base64

with open("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\Log\\BaseBrd.log", "r") as f:
    encoded = base64.b64encode(f.read())

f.close()

with open ("C:\\Windows\\Branding\\BaseBrd\\BaseBrd-01574\\Log\\BaseBrd_encoded.log", "w+") as f :
    f.write(encoded)

f.close()
