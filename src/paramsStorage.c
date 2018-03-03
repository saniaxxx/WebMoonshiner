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
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "paramsStorage.h"
#include "esp_ota_ops.h"

#define STORAGE_NAMESPACE "pps"

typedef struct {
  uint32_t min;
  uint32_t max;
} PreParameterRange;

void initStorage(nvs_handle *handle, esp_err_t *err) {
  // Initialize NVS
  *err = nvs_flash_init();
  if (*err == ESP_ERR_NVS_NO_FREE_PAGES) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    *err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(*err);

  // Open
  // printf("\n");
  // printf("Opening Non-Volatile Storage (NVS) handle... ");
  *err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, handle);
  if (*err != ESP_OK) {
    printf("Error (%d) opening NVS handle!\n", *err);
  } else {
    // printf("Done\n");
  }
}

bool isParameterExists(PreParameter parameter) {
  switch (parameter) {
    case WifiMode:
    case HeadPickingSpeed:
    case BodyPickingSpeed:
    case FinishTemperature:
    case ValvePickingSpeed:
    case HimselfWorkingTime:
    case ValvePeriod:
    case DeltaTemperature:
    case DecreasePickingTemperature:
    case CoolingActivationTemperature:
      return true;
    default:
      return false;
  }
}

PreParameterRange getRangeForPreParameter(PreParameter parameter) {
  PreParameterRange result;
  switch (parameter) {
    case WifiMode:
      result.min = 0;
      result.max = 1;
      break;
    case HeadPickingSpeed:
    case BodyPickingSpeed:
      result.min = 0;
      result.max = 100;
      break;
    case FinishTemperature:
      result.min = 85;
      result.max = 100;
      break;
    case ValvePickingSpeed:
      result.min = 100;
      result.max = 10000;
      break;
    case HimselfWorkingTime:
      result.min = 1;
      result.max = 180;
      break;
    case ValvePeriod:
      result.min = 1000;
      result.max = 20000;
      break;
    case DeltaTemperature:
      result.min = 1;
      result.max = 20;
      break;
    case DecreasePickingTemperature:
      result.min = 75;
      result.max = 85;
      break;
    case CoolingActivationTemperature:
      result.min = 60;
      result.max = 75;
      break;
  }
  return result;
}

uint32_t getDefaultValueForPreParameter(PreParameter parameter) {
  switch (parameter) {
    case WifiMode:
      return 1;
    case HeadPickingSpeed:
      return 10;
    case BodyPickingSpeed:
      return 50;
    case FinishTemperature:
      return 93;
    case CoolingActivationTemperature:
      return 73;
    case ValvePickingSpeed:
      return 2000;
    case HimselfWorkingTime:
      return 30;
    case ValvePeriod:
      return 8000;
    case DeltaTemperature:
      return 3;
    case DecreasePickingTemperature:
      return 85;
    default:
      return 0;
  }
}

uint32_t getPreParameter(PreParameter parameter, esp_err_t *err) {
  // check parameter
  if (!isParameterExists(parameter)) {
    *err = ESP_ERR_NVS_INVALID_NAME;
    return 0;
  }
  // Init
  nvs_handle handle;
  initStorage(&handle, err);

  // Get parameter name
  char parameterName[1];
  sprintf(parameterName, "%d", parameter);

  // Read
  // printf("Reading parameter from NVS ... ");
  int32_t value = 0;  // value will default to 0, if not set yet in NVS
  *err = nvs_get_i32(handle, parameterName, &value);
  switch (*err) {
    case ESP_OK:
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      printf("The value is not initialized yet!\n");
      value = getDefaultValueForPreParameter(parameter);
      break;
    default:
      printf("Error (%d) reading!\n", *err);
      value = getDefaultValueForPreParameter(parameter);
      break;
  }
  nvs_close(handle);
  return value;
}

void setPreParameter(PreParameter parameter, uint32_t value, esp_err_t *err) {
  // check parameter
  PreParameterRange range = getRangeForPreParameter(parameter);
  if (!isParameterExists(parameter)) {
    *err = ESP_ERR_NVS_INVALID_NAME;
    return;
  }
  if (value < range.min || value > range.max) {
    *err = ESP_ERR_OTA_VALIDATE_FAILED;
    return;
  }
  nvs_handle handle;
  initStorage(&handle, err);
  if (*err) return;

  // Get parameter name
  char parameterName[1];
  sprintf(parameterName, "%d", parameter);

  // Write
  // printf("Updating value in NVS ... ");
  *err = nvs_set_i32(handle, parameterName, value);
  // printf((*err != ESP_OK) ? "Failed!\n" : "Done\n");
  if (*err) return;

  // printf("Committing updates in NVS ... ");
  *err = nvs_commit(handle);
  // printf((*err != ESP_OK) ? "Failed!\n" : "Done\n");

  // Close
  nvs_close(handle);
}
