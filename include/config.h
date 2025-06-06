// Copyright (c) 2025 embeddedboys developers
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __CONFIG_H
#define __CONFIG_H

enum {
    TFT_ROTATE_0    = 0x00,
    TFT_ROTATE_90   = 0x01,
    TFT_ROTATE_180  = 0x02,
    TFT_ROTATE_270  = 0x03,
};

// Assume that the backlight is active high.
#ifndef TFT_BLK_ACTIVE_HIGH
    #define TFT_BLK_ACTIVE_HIGH 1
#endif

#define TFT_BUS_TYPE_SPI    0x00
#define TFT_BUS_TYPE_I80    0x01
#define TFT_BUS_TYPE_I2C    0x02

#ifndef TFT_X_OFFS
    #define TFT_X_OFFS 0
#endif

#ifndef TFT_Y_OFFS
    #define TFT_Y_OFFS 0
#endif

#define INDEV_BUS_TYPE_I2C  0x00
#define INDEV_BUS_TYPE_SPI  0x01

#ifndef INDEV_POLLING_PERIOD_MS
    #define INDEV_POLLING_PERIOD_MS 33
#endif

#endif /* __CONFIG_H */
