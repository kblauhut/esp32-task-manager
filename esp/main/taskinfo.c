#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_websocket_client.h"

void read_task_info_to_json(cJSON *json)
{
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;
    configRUN_TIME_COUNTER_TYPE ulTotalRunTime;

    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray == NULL)
        return; // TODO: return error

    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

    if (ulTotalRunTime == 0)
        return; // TODO: return error

    cJSON_AddStringToObject(json, "type", "task_info");
    cJSON_AddNumberToObject(json, "total_run_time", ulTotalRunTime);
    cJSON *tasks = cJSON_AddArrayToObject(json, "tasks");

    for (x = 0; x < uxArraySize; x++)
    {
        cJSON *task = cJSON_CreateObject();
        cJSON_AddStringToObject(task, "name", pxTaskStatusArray[x].pcTaskName);
        cJSON_AddNumberToObject(task, "priority", pxTaskStatusArray[x].uxCurrentPriority);
        cJSON_AddNumberToObject(task, "state", pxTaskStatusArray[x].eCurrentState);
        cJSON_AddNumberToObject(task, "stack", pxTaskStatusArray[x].usStackHighWaterMark);
        cJSON_AddNumberToObject(task, "runtime", pxTaskStatusArray[x].ulRunTimeCounter);
        cJSON_AddItemToArray(tasks, task);
    }

    vPortFree(pxTaskStatusArray);
}
