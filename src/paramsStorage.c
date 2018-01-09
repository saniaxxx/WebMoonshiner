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

#define STORAGE_NAMESPACE "pps"

void initStorage(nvs_handle *handle, esp_err_t *err){
    // Initialize NVS
    *err = nvs_flash_init();
    if (*err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        *err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( *err );

    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    *err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, handle);
    if (*err != ESP_OK) {
        printf("Error (%d) opening NVS handle!\n", *err);
    } else {
        printf("Done\n");
    }
}

uint32_t getPreParameter(PreParameter parameter, esp_err_t *err){
    //Init
    nvs_handle handle;
    initStorage(&handle, err);

    //Get parameter name
    char parameterName[1];
    sprintf(parameterName, "%d", parameter);

    // Read
    printf("Reading parameter from NVS ... ");
    int32_t value = 0; // value will default to 0, if not set yet in NVS
    *err = nvs_get_i32(handle, parameterName, &value);
    switch (*err) {
        case ESP_OK:
            printf("Done\n");
            printf("Value counter = %d\n", value);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default :
            printf("Error (%d) reading!\n", *err);
    }
    return value;
}

void setPreParameter(PreParameter parameter, uint32_t value, esp_err_t *err){
    nvs_handle handle;
    initStorage(&handle, err);
    if(*err) return;

    //Get parameter name
    char parameterName[1];
    sprintf(parameterName, "%d", parameter);

    // Write
    printf("Updating value in NVS ... ");
    *err = nvs_set_i32(handle, parameterName, value);
    printf((*err != ESP_OK) ? "Failed!\n" : "Done\n");
    if(*err) return;

    printf("Committing updates in NVS ... ");
    *err = nvs_commit(handle);
    printf((*err != ESP_OK) ? "Failed!\n" : "Done\n");

    // Close
    nvs_close(handle);
}
