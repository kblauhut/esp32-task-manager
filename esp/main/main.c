/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "network.h"

void vTaskGetRunTimeStats1(char *pcWriteBuffer)
{
    printf("vTaskGetRunTimeStats1\n");
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    configRUN_TIME_COUNTER_TYPE ulTotalRunTime, ulStatsAsPercentage;

    // Make sure the write buffer does not contain a string.
    // pcWriteBuffer = 0x00;

    // Take a snapshot of the number of tasks in case it changes while this
    // function is executing.
    uxArraySize = uxTaskGetNumberOfTasks();
    printf("uxArraySize: %d\n", uxArraySize);

    // Allocate a TaskStatus_t structure for each task.  An array could be
    // allocated statically at compile time.
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    printf("alloced \n");

    if (pxTaskStatusArray != NULL)
    {

        // Generate raw status information about each task.
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

        // return;
        // For percentage calculations.
        ulTotalRunTime /= 100UL;

        // Avoid divide by zero errors.
        if (ulTotalRunTime > 0)
        {
            // For each populated position in the pxTaskStatusArray array,
            //     format the raw data as human readable ASCII data
            for (x = 0; x < uxArraySize; x++)
            {
                // What percentage of the total run time has the task used?
                // This will always be rounded down to the nearest integer.
                // ulTotalRunTimeDiv100 has already been divided by 100.
                ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;

                if (ulStatsAsPercentage > 0UL)
                {
                    printf("%s %ld %ld \n", pxTaskStatusArray[x].pcTaskName, pxTaskStatusArray[x].ulRunTimeCounter, ulStatsAsPercentage);
                    // printf("%s\t\t%lu\t\t%lu%%\r\n", pxTaskStatusArray[x].pcTaskName, pxTaskStatusArray[x].ulRunTimeCounter, ulStatsAsPercentage);
                }
                else
                {
                    printf("%s %ld %ld \n", pxTaskStatusArray[x].pcTaskName, pxTaskStatusArray[x].ulRunTimeCounter, ulStatsAsPercentage);
                    // If the percentage is zero here then the task has
                    // consumed less than 1% of the total run time.
                    // printf("%s\t\t%lu\t\t<1%%\r\n", pxTaskStatusArray[x].pcTaskName, pxTaskStatusArray[x].ulRunTimeCounter);
                }

                // pcWriteBuffer += strlen((char *)pcWriteBuffer);
            }
        }
    }

    // The array is no longer needed, free the memory it consumes.
    vPortFree(pxTaskStatusArray);
}

void app_main(void)
{
    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
    char pcWriteBuffer[1000];
    wifi_connect();

    vTaskGetRunTimeStats1(pcWriteBuffer);

    // esp_restart();
}
