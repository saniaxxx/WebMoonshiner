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

#include "dispatcher.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config/config.h"
#include "publicQueues.h"
#include "operationModes.h"
#include "cJSON.h"

static xTaskHandle current_task = NULL;

void stopWorkingTask(){
  if(current_task){
    printf("%s\n", "delete working task");
    vTaskDelete(current_task);
    current_task = NULL;
  }
}

void changeWorkingMode(OperationMode mode){
  printf("%s\n", "changing working mode");
  if (mode == OperationModeTest){
    printf("%s\n", "testOfHardware");
    stopWorkingTask();
    xTaskCreate(testOfHardware, "testOfHardware", STACK_SIZE, NULL, TaskPriorityLow, &current_task);
  }
  else if (mode == OperationModePass){
    printf("%s\n", "doNothing");
    stopWorkingTask();
    xTaskCreate(doNothing, "doNothing", STACK_SIZE, NULL, TaskPriorityLow, &current_task);
  }else{
    printf("%s\n", "unknown mode");
  }
}

void dispatcherTask( void *pvParametres) {
    // create queues
    Json_outgoing_queue = xQueueCreate(10, sizeof(cJSON*));
    Json_incoming_queue = xQueueCreate(10, sizeof(cJSON*));
    Temperatures_queue = xQueueCreate(3, sizeof(Temperature_info));
    Sound_queue = xQueueCreate(10, sizeof(Sound_info));

    for( ;; ) {
      cJSON *root = NULL;

      if (xQueueReceive(Json_incoming_queue, &root, 100 / portTICK_PERIOD_MS) == pdTRUE) {
        MessageType message_type = (MessageType)cJSON_GetObjectItem(root,"type")->valueint;
        if (message_type == MessageTypeChangeMode) {
          OperationMode mode = (OperationMode)cJSON_GetObjectItem(root,"mode")->valueint;
          changeWorkingMode(mode);
        }
      }
    }
    vTaskDelete( NULL );
}

void startDispatcherTask(int priority){
 xTaskCreate(dispatcherTask, "dispatcher_task", STACK_SIZE, NULL, priority, NULL);
}
