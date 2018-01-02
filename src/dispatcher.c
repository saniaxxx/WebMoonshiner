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
#include "customQueues.h"
#include "WebSocket_Task.h"

void dispatcherTask( void *pvParametres) {
    // create queues
    WebSocket_rx_queue = xQueueCreate(10, sizeof(WebSocket_frame_t));
    WebSocket_wx_queue = xQueueCreate(10, sizeof(char*));
    Temperatures_queue = xQueueCreate(3, sizeof(Temperature_info));
    Sound_queue = xQueueCreate(10, sizeof(Sound_info));

    for( ;; ) {
      Temperature_info tempinfo;
      if (xQueueReceive(Temperatures_queue, &tempinfo, 0) == pdTRUE)
      {
            xQueueSend(WebSocket_wx_queue, &tempinfo.temperature, 0);
      }
      Sound_info soundInfo = {.duration = 100, .pause = 5000};
      xQueueSend(Sound_queue, &soundInfo, 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS );
    }

    vTaskDelete( NULL );
}

 void startDispatcherTask(int priority){
   xTaskCreate(&dispatcherTask, "dispatcher_task", STACK_SIZE, NULL, priority, NULL);
 }
