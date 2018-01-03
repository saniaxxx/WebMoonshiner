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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "config/config.h"
#include "valveController.h"

static unsigned int valve_pwm_percents = 0;

void blink_task(void *pvParameter)
{
    gpio_pad_select_gpio(VALVE_GPIO_PIN);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(VALVE_GPIO_PIN, GPIO_MODE_OUTPUT);
    while(1) {
      float high_output_period = 1000 * VALVE_CYCLE_PERIOD * valve_pwm_percents / 100;
      float low_output_period = 1000 * VALVE_CYCLE_PERIOD - high_output_period;
      /* Set valve off (output low) */
      gpio_set_level(VALVE_GPIO_PIN, 0);
      vTaskDelay(low_output_period / portTICK_PERIOD_MS);
      /* Set valve on (output high) */
      gpio_set_level(VALVE_GPIO_PIN, valve_pwm_percents?1:0);
      vTaskDelay(high_output_period / portTICK_PERIOD_MS);
    }
}

void setValvePWM(unsigned int percents) {
  valve_pwm_percents = percents;
}

unsigned int getValvePWM() {
  return valve_pwm_percents;
}

void startValveController(int priority){
    xTaskCreate(&blink_task, "blink_task", STACK_SIZE, NULL, priority, NULL);
}
