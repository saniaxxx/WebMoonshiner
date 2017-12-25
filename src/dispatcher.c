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
#include "temperatureChecker.h"
#include "webSocketServer.h"
#include "config/config.h"


// WebSocket frame receive queue
QueueHandle_t WebSocket_rx_queue;
// WebSocket frame send queue
QueueHandle_t WebSocket_wx_queue;
// Temperatures data queue
QueueHandle_t Temperatures_queue;

void dispatcherTask( void *pvParametres) {
    // create WebSocket RX Queue
    WebSocket_rx_queue = xQueueCreate(10, sizeof(WebSocket_frame_t));
    // create WebSocket WX Queue
    WebSocket_wx_queue = xQueueCreate(10, sizeof(char*));
    // create queue for temperatures data
    Temperatures_queue = xQueueCreate(3, sizeof(Temperature_info));

    for( ;; ) {
      Temperature_info tempinfo;
      if (xQueueReceive(Temperatures_queue, &tempinfo, 0) == pdTRUE)
      {
            xQueueSendToFront(WebSocket_wx_queue, &tempinfo.temperature, 0);
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS );
    }

    vTaskDelete( NULL );
}

 void startDispatcherTask(int priority){
   xTaskCreate(&dispatcherTask, "dispatcher_task", STACK_SIZE, NULL, priority, NULL);
 }
