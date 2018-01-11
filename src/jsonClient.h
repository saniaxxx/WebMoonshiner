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

#include "esp_system.h"

typedef enum ServerMessageType{
    ServerMessageTypeTemperature = 0,
    ServerMessageTypeAck = 1,
} ServerMessageType;

typedef enum ClientMessageType{
    ClientMessageTypeChangeMode = 0,
    ClientMessageTypeGetPreParameter = 1,
    ClientMessageTypeSetPreParameter = 2,
} ClientMessageType;

typedef enum OperationMode{
    OperationModeTest = 0,
    OperationModeBoost = 1,
    OperationModeSelf = 2,
    OperationModeHeads = 3,
    OperationModeBody = 4,
} OperationMode;

void sendTempToClient();
void handleClientMessage(bool (*changeWorkingMode)(OperationMode));
