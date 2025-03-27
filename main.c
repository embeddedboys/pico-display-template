// Copyright (c) 2024 embeddedboys developers
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "pico/time.h"
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/platform.h"
#include "pico/stdio_uart.h"

#include "hardware/pll.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "tft.h"
#include "backlight.h"
// #include "debug.h"

QueueHandle_t xToFlushQueue = NULL;

void vApplicationTickHook()
{

}

portTASK_FUNCTION(example_video_push_task, pvParameters)
{
#define CUBE_SIZE 128
    static uint16_t video_memory[CUBE_SIZE * CUBE_SIZE] = {0};
    static struct video_frame vf = {
        .len = sizeof(video_memory),
        .vmem = video_memory,
        .xs = TFT_HOR_RES / 2 - (CUBE_SIZE / 2) - 1,
        .xe = TFT_HOR_RES / 2 + (CUBE_SIZE / 2) - 1,
        .ys = TFT_VER_RES / 2 - (CUBE_SIZE / 2) - 1,
        .ye = TFT_VER_RES / 2 + (CUBE_SIZE / 2) - 1,
    };

    tft_driver_init();
    backlight_driver_init();

    tft_fill_color(0x0);
    backlight_set_level(100);

    printf("going to video push loop\n");
    /* Non buffered, flush pixel by pixel */
    // for (uint16_t color = 0;; color+=255)
    //     tft_fill_color(color);

    for (;;) {
        memset(video_memory, (rand() % 255), sizeof(video_memory));
        tft_async_video_push(&vf);
    }

}

int main(void)
{
    /* NOTE: DO NOT MODIFY THIS BLOCK */
#define CPU_SPEED_MHZ (DEFAULT_SYS_CLK_KHZ / 1000)
    if(CPU_SPEED_MHZ > 266 && CPU_SPEED_MHZ <= 360)
        vreg_set_voltage(VREG_VOLTAGE_1_20);
    else if (CPU_SPEED_MHZ > 360 && CPU_SPEED_MHZ <= 396)
        vreg_set_voltage(VREG_VOLTAGE_1_25);
    else if (CPU_SPEED_MHZ > 396)
        vreg_set_voltage(VREG_VOLTAGE_MAX);
    else
        vreg_set_voltage(VREG_VOLTAGE_DEFAULT);

    set_sys_clock_khz(CPU_SPEED_MHZ * 1000, true);
    clock_configure(clk_peri,
                    0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                    CPU_SPEED_MHZ * MHZ,
                    CPU_SPEED_MHZ * MHZ);
    stdio_uart_init_full(uart0, 115200, 16, 17);
    // stdio_uart_init();

    printf("\n\n\nPICO DM SPI Template LVGL Porting\n");

    xToFlushQueue = xQueueCreate(2, sizeof(struct video_frame));

    TaskHandle_t video_push_handler;
    xTaskCreate(example_video_push_task, "video_push", 256, NULL, (tskIDLE_PRIORITY + 1), &video_push_handler);
    vTaskCoreAffinitySet(video_push_handler, (1 << 0));

    TaskHandle_t video_flush_handler;
    xTaskCreate(video_flush_task, "video_flush", 256, NULL, (tskIDLE_PRIORITY + 2), &video_flush_handler);
    vTaskCoreAffinitySet(video_flush_handler, (1 << 1));

    printf("calling freertos scheduler, %lld\n", time_us_64());
    vTaskStartScheduler();
    for(;;);

    return 0;
}
