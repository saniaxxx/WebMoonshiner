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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config/config.h"
#include "publicQueues.h"
#include "valveController.h"
#include "operationModes.h"
#include "jsonClient.h"
#include "buzzerController.h"
#include "paramsStorage.h"
#include "temperatureChecker.h"
#include "cJSON.h"

void testOfHardware(void* pvParametres)
{
    //test ds18b20 devices
    sendStatusToClient(0);

    //test valve
    setValvePeriodMillisec(100); //100 ms
    setValvePWM(100); //100%

    //sleep
    vTaskSuspend(NULL);
}

void boostMode(void *pvParametres){
    bool wasNotified = false;
    for (;;) {
        esp_err_t err;
        uint32_t activationTemperature = getPreParameter(CoolingActivationTemperature, &err);
        Temperature_info info =  getTemperatures();
        if(wasNotified && info.cubeTemperature >= activationTemperature){
            playSoundRepeatedly(5);
            wasNotified = true;
        }
        sendStatusToClient(0);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void selfEmployment(void* pvParametres)
{
    esp_err_t err;
    uint32_t himselfWorkingTime = getPreParameter(HimselfWorkingTime, &err);
    uint32_t himselfWorkingTimeInTicks = himselfWorkingTime * 60 * portTICK_RATE_MS;
    portTickType LastTick = xTaskGetTickCount();
    for (;;) {
        if((xTaskGetTickCount() - LastTick) > himselfWorkingTimeInTicks){
              playSoundRepeatedly(5);
              vTaskSuspend(NULL);
        }
        sendStatusToClient(0);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

void pickingHeads(void* pvParametres)
{
    for (;;) {
        esp_err_t err;
        uint32_t pwm = getPreParameter(HeadPickingSpeed, &err);
        uint32_t period = getPreParameter(ValvePeriod, &err);
        setValvePeriodMillisec(period);
        setValvePWM(pwm);
        sendStatusToClient(pwm);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

void pickingBody(void* pvParametres)
{
    for (;;) {
        esp_err_t err;
        uint32_t decrease_temp = getPreParameter(DecreasePickingTemperature, &err);
        uint32_t finish_temp = getPreParameter(FinishTemperature, &err);
        uint32_t basepwm = getPreParameter(BodyPickingSpeed, &err);
        uint32_t period = getPreParameter(ValvePeriod, &err);
        setValvePeriodMillisec(period);
        float pwm = 0;
        Temperature_info info =  getTemperatures();
        if(info.cubeTemperature > finish_temp){
            playSoundRepeatedly(5);
            vTaskSuspend(NULL);
        }else if(info.cubeTemperature < decrease_temp){
            pwm = basepwm; //нет уменьшения
        }else{
            uint32_t delta = finish_temp - decrease_temp; // считаем за 100 процентов
            uint32_t overstatement = decrease_temp - info.cubeTemperature; // залет
            pwm = (delta - overstatement) / delta * basepwm; // автоуменьшение
        }
        setValvePWM(pwm);
        sendStatusToClient(pwm);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}
