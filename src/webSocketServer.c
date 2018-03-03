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
#include "esp_system.h"
#include "config/config.h"
#include "webSocketServer.h"
#include "publicQueues.h"
#include "cJSON.h"
#include <string.h>

// WebSocket frame receive queue
QueueHandle_t WebSocket_rx_queue;

void web_socket_read_task(void *pvParameters) {
  (void)pvParameters;

  // frame buffer
  WebSocket_frame_t __RX_frame;

  // create WebSocket RX Queue
  WebSocket_rx_queue = xQueueCreate(10, sizeof(WebSocket_frame_t));

  while (1) {
    // receive next WebSocket frame from queue
    if (xQueueReceive(WebSocket_rx_queue, &__RX_frame,
                      100 / portTICK_PERIOD_MS) == pdTRUE) {
      // write frame inforamtion to UART
      // printf("Received new frame from Websocket. Length %d, payload %.*s
      // \r\n",
      //     __RX_frame.payload_length, __RX_frame.payload_length,
      //     __RX_frame.payload);

      cJSON *root = cJSON_Parse(__RX_frame.payload);
      xQueueSend(Json_incoming_queue, &root, 0);
      // free memory
      if (__RX_frame.payload != NULL) free(__RX_frame.payload);
    }
  }
  vTaskDelete(NULL);
}

void web_socket_write_task(void *pvParameters) {
  (void)pvParameters;

  // buffer
  cJSON *json = NULL;

  while (1) {
    // receive string from queue
    if (xQueueReceive(Json_outgoing_queue, &json, 100 / portTICK_PERIOD_MS) ==
        pdTRUE) {
      // stringify json
      char *payload = cJSON_PrintUnformatted(json);
      // check null
      if (payload == NULL) {
        printf("Error sending frame, payload is NULL");
        return;
      }
      // get size
      size_t length = strlen(payload);
      // send
      err_t error = WS_write_data(payload, length);
      if (error) {
        printf("Error sending frame to Websocket, code = %d \r\n", error);
      }
      // free memory
      cJSON_Delete(json);
      free(payload);
    }
  }
  vTaskDelete(NULL);
}

void startWebSocketServer(int priority) {
  // create WebSocker receive task
  xTaskCreate(web_socket_read_task, "ws_process_rx", STACK_SIZE, NULL, priority,
              NULL);

  // create WebSocker send task
  xTaskCreate(web_socket_write_task, "ws_process_wx", STACK_SIZE, NULL,
              priority, NULL);

  // Create Websocket Server Task
  xTaskCreate(ws_server, "ws_server", STACK_SIZE, NULL, priority, NULL);
}
