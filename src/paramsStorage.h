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

#include "esp_types.h"
#include "esp_err.h"

typedef enum  PreParameter {
    WifiMode = 0, // режим wifi, 0 - клиент, 1 - сервер
    HeadPickingSpeed = 1, // скорость отбора голов в процентах
    BodyPickingSpeed = 2, // скорость отбора тела в процентах
    FinishTemperature = 3, // температура завершения отбора тела в градусах
    ValvePickingSpeed = 4, // скорость отбора открытого клапана в мл/час
    HimselfWorkingTime = 5, // время работы на себя в минутах
    ValvePeriod = 6, // период срабатывания клапана в мс
    DeltaTemperature = 7, // дельта залета температуры колонны в десятых долях градуса
    DecreasePickingTemperature = 8, // температура снижения отбора в градусах
    CoolingActivationTemperature = 9, // температура включения воды охлаждения в градусах
} PreParameter;

uint32_t getPreParameter(PreParameter parameter, esp_err_t *err);
void setPreParameter(PreParameter parameter, uint32_t value, esp_err_t *err);
