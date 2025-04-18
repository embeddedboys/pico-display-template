// Copyright (c) 2024 embeddedboys developers

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:

// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __TFT_H
#define __TFT_H

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "config.h"

struct tft_priv;

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

struct tft_ops {
    void (*write_reg)(struct tft_priv *priv, int len, ...);
    void (*write_vmem)(struct tft_priv *priv, void *vmem, size_t len);

    int (*init_display)(struct tft_priv *priv);
    int (*reset)(struct tft_priv *priv);
    int (*clear)(struct tft_priv *priv, u16 clear);
    int (*sleep)(struct tft_priv *priv, bool on);
    int (*set_dir)(struct tft_priv *priv, u8 dir);
    void (*set_addr_win)(struct tft_priv *priv, int xs, int ys, int xe, int ye);
    void (*video_sync)(struct tft_priv *priv, int xs, int ys, int xe, int ye, void *vmem, size_t len);
};

struct tft_display {
    u32                     xres;
    u32                     yres;
    u32                     bpp;
    u32                     rotate;
    u32                     backlight;

    u32                     xoffs;
    u32                     yoffs;

    bool                    need_tx_buf;

    struct tft_ops          tftops;
};

struct tft_priv {
    u8                      *buf;
    struct {
        void                *buf;
        size_t              len;
    } txbuf;

#if TFT_BUS_TYPE == TFT_BUS_TYPE_SPI
    struct {
        int scl;
        int sda;
        int reset;
        int dc;
        int cs;
        int blk;
    } gpio;
#elif TFT_BUS_TYPE == TFT_BUS_TYPE_I80
    struct {
        int reset;
        int cs;   /* chip select */
        int rs;   /* register/data select */
        int wr;   /* write signal */
        int rd;   /* read signal */
        int bl;   /* backlight */
        int db[TFT_PIN_DB_COUNT];
    } gpio;
#endif

    /* device specific */
    struct tft_display    *display;
    struct tft_ops        *tftops;
} __attribute__((__aligned__(4)));

struct video_frame {
    int xs;
    int ys;
    int xe;
    int ye;
    void *vmem;
    size_t len;
};

#define BIT(nr)			(1UL << (nr))
#define MADCTL 0x36
#define MY  BIT(7)
#define MX  BIT(6)
#define MV  BIT(5)
#define ML  BIT(4)
#define BGR BIT(3)
#define MH  BIT(2)
#define FH  BIT(1)
#define FV  BIT(0)

#define TFT_REG_BUF_SIZE 64
#define TFT_TX_BUF_SIZE 2048
#define TFT_X_RES TFT_HOR_RES
#define TFT_Y_RES TFT_VER_RES

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))
#define dm_gpio_set_value(p,v) gpio_put(p, v)
#define mdelay(v) busy_wait_ms(v)

extern int tft_probe(struct tft_display *display);
extern int tft_driver_init();
extern int tft_fill_color(u16 color);

#if TFT_BUS_TYPE == TFT_BUS_TYPE_SPI

    #if TFT_SPIX == 0
        #define spi_ifce spi0
    #elif TFT_SPIX == 1
        #define spi_ifce spi1
    #else
        #error "Invalid SPI instance!"
    #endif

    #if DISP_OVER_PIO
        extern int pio_spi_tx_init(uint data_pin, uint clk_pin);
        extern void pio_spi_tx_write_buf_dc(void *buf, size_t len, bool dc);
        #define write_buf_dc(p, b, l, r) pio_spi_tx_write_buf_dc(b, l, r)
    #else
        #define write_buf_dc(p, b, l, r) tft_spi_write_buf_dc(p, b, l, r)
    #endif

#elif TFT_BUS_TYPE == TFT_BUS_TYPE_I80
    #if DISP_OVER_PIO
        extern int i80_pio_init(uint8_t db_base, uint8_t db_count, uint8_t pin_wr);
        extern int i80_write_buf_rs(void *buf, size_t len, bool rs);
        #define write_buf_dc(p, b, l, r) i80_write_buf_rs(b, l, r)
    #else
        extern void fbtft_write_gpio16_wr_rs(struct tft_priv *priv, void *buf, size_t len, bool rs);
        #define write_buf_dc(p, b, l, r) fbtft_write_gpio16_wr_rs(p, b, l, r)
    #endif
#elif TFT_BUS_TYPE == TFT_BUS_TYPE_I2C
    #error "I2C bus type not implemented yet!"
#endif

extern void tft_video_flush(int xs, int ys, int xe, int ye, void *vmem, uint32_t len);
extern void tft_async_video_push(struct video_frame *vf);

extern void tft_write_cmd(struct tft_priv *priv, u8 cmd);
extern void tft_write_data(struct tft_priv *priv, u8 data);

#define write_cmd(priv, cmd) tft_write_cmd(priv, cmd)
#define write_data(priv, data) tft_write_data(priv, data)

extern void tft_write_reg(struct tft_priv *priv, int len, ...);
#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__}) / sizeof(int))

extern void tft_write_reg8(struct tft_priv *priv, int len, ...);
extern void tft_write_reg16(struct tft_priv *priv, int len, ...);

#define write_reg(priv, ...) \
    priv->tftops->write_reg(priv, NUMARGS(__VA_ARGS__), __VA_ARGS__)

extern u32 frame_counter;
extern QueueHandle_t xToFlushQueue;
extern portTASK_FUNCTION(video_flush_task, pvParameters);

#endif
