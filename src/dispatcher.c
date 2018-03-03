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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config/config.h"
#include "publicQueues.h"
#include "operationModes.h"
#include "buzzerController.h"
#include "jsonClient.h"
#include "cJSON.h"

static xTaskHandle current_task = NULL;

void stopWorkingTask() {
  if (current_task) {
    printf("%s\n", "delete working task");
    vTaskDelete(current_task);
    current_task = NULL;
  }
}

workingModeFunction getFunctionForMode(OperationMode mode) {
  static workingModeFunction workingModes[6] = {testOfHardware,
                                                boostMode,
                                                selfEmployment,
                                                pickingHeads,
                                                pickingBodyByCubeTemperature,
                                                pickingBodyByColumnTemperature};
  int cnt = sizeof(workingModes) / sizeof(workingModeFunction);
  if (mode >= 0 && mode < cnt) {
    return workingModes[mode];
  }
  return NULL;
}

bool changeWorkingMode(OperationMode mode) {
  workingModeFunction taskToCall = getFunctionForMode(mode);
  if (taskToCall) {
    printf("%s\n", "changing working mode");
    stopWorkingTask();
    xTaskCreate(taskToCall, "workingMode", STACK_SIZE, NULL, TaskPriorityLow,
                &current_task);
    playSoundRepeatedly(mode + 1);
    return true;
  } else {
    printf("%s\n", "unknown mode");
    return false;
  }
}

void dispatcherTask(void* pvParametres) {
  // create queues
  Json_outgoing_queue = xQueueCreate(10, sizeof(cJSON*));
  Json_incoming_queue = xQueueCreate(10, sizeof(cJSON*));
  Sound_queue = xQueueCreate(10, sizeof(Sound_info));

  for (;;) {
    handleClientMessage(&changeWorkingMode);
  }
  vTaskDelete(NULL);
}

void debugTask(void* pvParametres) {
  for (;;) {
    printf("free memory =  %d\n",esp_get_free_heap_size());
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
  vTaskDelete(NULL);
}

void startDispatcherTask(int priority) {
  xTaskCreate(dispatcherTask, "dispatcher_task", STACK_SIZE, NULL, priority,
              NULL);
}

void startDebugTask(int priority) {
  xTaskCreate(debugTask, "debug_task", STACK_SIZE, NULL, priority, NULL);
}
