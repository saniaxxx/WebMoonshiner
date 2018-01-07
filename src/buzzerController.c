/*
 * MIT License
 *
 * Copyright (c) 2017, Alexander Solncev
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "sdkconfig.h"
#include "buzzerController.h"
#include "config/config.h"
#include "publicQueues.h"

#define GPIO_OUTPUT_SPEED LEDC_HIGH_SPEED_MODE

void sound(int gpio_num, uint32_t freq, uint32_t duration)
{
    ledc_timer_config_t timer_conf;
    timer_conf.speed_mode = GPIO_OUTPUT_SPEED;
    timer_conf.bit_num = LEDC_TIMER_10_BIT;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.freq_hz = freq;
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ledc_conf;
    ledc_conf.gpio_num = gpio_num;
    ledc_conf.speed_mode = GPIO_OUTPUT_SPEED;
    ledc_conf.channel = LEDC_CHANNEL_0;
    ledc_conf.intr_type = LEDC_INTR_DISABLE;
    ledc_conf.timer_sel = LEDC_TIMER_0;
    ledc_conf.duty = 0x0; // 50%=0x3FFF, 100%=0x7FFF for 15 Bit
    // 50%=0x01FF, 100%=0x03FF for 10 Bit
    ledc_channel_config(&ledc_conf);

    // start
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0x7F); // 12% duty - play here for your speaker or buzzer
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);
    vTaskDelay(duration / portTICK_PERIOD_MS);
    // stop
    ledc_set_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0, 0);
    ledc_update_duty(GPIO_OUTPUT_SPEED, LEDC_CHANNEL_0);
}

void buzzer_task(void* pvParameters)
{
    Sound_info soundInfo;
    portBASE_TYPE xStatus;
    for (;;) {
        xStatus = xQueueReceive(Sound_queue, &soundInfo, 100 / portTICK_PERIOD_MS);
        if (xStatus == pdPASS) {
            printf("sound: duration = %d, pause = %d\n", soundInfo.duration, soundInfo.pause);
            sound(BUZZER_GPIO_PIN, 770, soundInfo.duration);
            vTaskDelay(soundInfo.pause / portTICK_PERIOD_MS);
        }
    }

    vTaskDelete(NULL);
}

void startBuzzerController(unsigned int priority)
{
    xTaskCreate(buzzer_task, "buzzer_task", STACK_SIZE, NULL, priority, NULL);
}
