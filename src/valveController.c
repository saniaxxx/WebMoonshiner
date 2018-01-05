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
static unsigned int valve_cycle_period_ms = 5000;

xSemaphoreHandle pwmMutex(){
  static xSemaphoreHandle pwm_mutex = NULL;
  if(pwm_mutex == NULL){
    pwm_mutex = xSemaphoreCreateMutex();
  }
  return pwm_mutex;
}

xSemaphoreHandle periodMutex(){
  static xSemaphoreHandle period_mutex = NULL;
  if(period_mutex == NULL){
    period_mutex = xSemaphoreCreateMutex();
  }
  return period_mutex;
}

void loop(){
  if(getValvePWM() == 0){
    vTaskDelay(100 / portTICK_PERIOD_MS);
    return;
  }
  // calculate periods
  float high_output_period = getValvePeriod() * getValvePWM() / 100;
  float low_output_period = getValvePeriod() - high_output_period;

  // set valve off (output low)
  gpio_set_level(VALVE_GPIO_PIN, 1);
  vTaskDelay(high_output_period / portTICK_PERIOD_MS);

  // set valve on (output high)
  gpio_set_level(VALVE_GPIO_PIN, 0);
  vTaskDelay(low_output_period / portTICK_PERIOD_MS);

  // reset pwm
  setValvePWM(0);
}

void valveControllerTask(void *pvParameter){
  gpio_pad_select_gpio(VALVE_GPIO_PIN);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(VALVE_GPIO_PIN, GPIO_MODE_OUTPUT);
  while(1) {
    loop();
  }
}

bool setValvePWM(unsigned int percents) {
  if (percents > 100){
    return false;
  }
  if( xSemaphoreTake( pwmMutex(), portMAX_DELAY ) == pdTRUE ){
    valve_pwm_percents = percents;
    xSemaphoreGive( pwmMutex() );
  }
  return true;
}

unsigned int getValvePWM() {
  unsigned int result = 0;
  if( xSemaphoreTake( pwmMutex(), portMAX_DELAY ) == pdTRUE ){
    result = valve_pwm_percents;
    xSemaphoreGive( pwmMutex() );
  }
  return result;
}

bool setValvePeriodMillisec(unsigned int microseconds) {
  if (microseconds <= 0){
    return false;
  }
  if( xSemaphoreTake( periodMutex(), portMAX_DELAY ) == pdTRUE ){
    valve_cycle_period_ms = microseconds;
    xSemaphoreGive( periodMutex() );
  }
  return true;
}

unsigned int getValvePeriod() {
  unsigned int result = 0;
  if( xSemaphoreTake( periodMutex(), portMAX_DELAY ) == pdTRUE ){
    result = valve_cycle_period_ms;
    xSemaphoreGive( periodMutex() );
  }
  return result;
}

void startValveController(int priority){
    xTaskCreate(valveControllerTask, "valveControllerTask", STACK_SIZE, NULL, priority, NULL);
}
