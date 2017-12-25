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
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "config/config.h"
#include "webSocketServer.h"

// WebSocket frame receive queue
extern QueueHandle_t WebSocket_rx_queue;
// WebSocket frame send queue
extern QueueHandle_t WebSocket_wx_queue;

void web_socket_read_task(void* pvParameters)
{
    (void)pvParameters;

    // frame buffer
    WebSocket_frame_t __RX_frame;

    while (1) {
        // receive next WebSocket frame from queue
        if (xQueueReceive(WebSocket_rx_queue, &__RX_frame,
                3 * portTICK_PERIOD_MS)
            == pdTRUE) {
            // write frame inforamtion to UART
            printf("Received new frame from Websocket. Length %d, payload %.*s \r\n",
                __RX_frame.payload_length, __RX_frame.payload_length,
                __RX_frame.payload);

            // free memory
            if (__RX_frame.payload != NULL)
                free(__RX_frame.payload);
        }
    }
    vTaskDelete( NULL );
}

void web_socket_write_task(void* pvParameters)
{
    (void)pvParameters;

    // buffer
    char* payload;

    while (1) {
        // receive string from queue
        if (xQueueReceive(WebSocket_wx_queue, &payload,
                3 * portTICK_PERIOD_MS)
            == pdTRUE) {

            // get size
            size_t payload_length = sizeof(payload)/sizeof(*payload);

            // write inforamtion to UART
            printf("Sending frame to Websocket . Length %d, payload %.*s \r\n",
                payload_length, payload_length,
                payload);

            // send
            WS_write_data(payload, payload_length);

            // free memory
            if (payload != NULL){
              free(payload);
            }
        }
    }
    vTaskDelete( NULL );
}

esp_err_t event_handler(void* ctx, system_event_t* event) { return ESP_OK; }

void startWebSocketServer(int priority)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t sta_config = {.sta = {.ssid = WIFI_SSID,
                                    .password = WIFI_PASSWORD,
                                    .bssid_set = false } };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    // create WebSocker receive task
    xTaskCreate(&web_socket_read_task, "ws_process_rx", STACK_SIZE, NULL, priority, NULL);

    // create WebSocker send task
    xTaskCreate(&web_socket_write_task, "ws_process_wx", STACK_SIZE, NULL, priority, NULL);

    // Create Websocket Server Task
    xTaskCreate(&ws_server, "ws_server", STACK_SIZE, NULL, priority, NULL);
}
