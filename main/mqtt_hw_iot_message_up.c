#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mqtt_hw_iot_message_up.h"

HW_IOT_MES_DES *hw_iot_malloc_des(void)
{
    HW_IOT_MES_DES *mes = (HW_IOT_MES_DES *)malloc(sizeof(HW_IOT_MES_DES));
    if (mes)
    {
        memset(mes, 0, sizeof(HW_IOT_MES_DES));
        mes->services_json = cJSON_CreateObject();
        if (mes->services_json)
        {
            cJSON *services = cJSON_CreateArray();
            if (services)
            {
                cJSON_AddItemToObject(mes->services_json, "services", services);
            }
        }
    }
    return mes;
}

void hw_iot_set_mes_des(HW_IOT_MES_DES *des, const char *service_id, const char *name, int val)
{
    if (des == NULL || des->services_json == NULL || service_id == NULL || name == NULL)
    {
        return;
    }

    cJSON *services = cJSON_GetObjectItem(des->services_json, "services");
    if (services == NULL)
    {
        return;
    }

    cJSON *service = NULL;
    cJSON_ArrayForEach(service, services)
    {
        cJSON *sid = cJSON_GetObjectItem(service, "service_id");
        if (sid && strcmp(sid->valuestring, service_id) == 0)
        {
            break;
        }
        service = NULL;
    }

    if (service == NULL)
    {
        service = cJSON_CreateObject();
        if (service)
        {
            cJSON_AddStringToObject(service, "service_id", service_id);
            cJSON *properties = cJSON_CreateObject();
            if (properties)
            {
                cJSON_AddItemToObject(service, "properties", properties);
            }
            cJSON_AddItemToArray(services, service);
        }
    }

    if (service)
    {
        cJSON *properties = cJSON_GetObjectItem(service, "properties");
        if (properties)
        {
            cJSON_AddNumberToObject(properties, name, val);
        }
    }
}

void hw_iot_mes_string(HW_IOT_MES_DES *des)
{
    if (des == NULL || des->services_json == NULL)
    {
        return;
    }

    if (des->mes_js_str != NULL)
    {
        free(des->mes_js_str);
        des->mes_js_str = NULL;
        des->mes_js_len = 0;
    }

    char *json_str = cJSON_PrintUnformatted(des->services_json);
    if (json_str)
    {
        des->mes_js_len = strlen(json_str);
        des->mes_js_str = (char *)malloc(des->mes_js_len + 1);
        if (des->mes_js_str)
        {
            strcpy(des->mes_js_str, json_str);
        }
        cJSON_free(json_str);
    }
}

void hw_iot_free_des(HW_IOT_MES_DES *des)
{
    if (des == NULL)
    {
        return;
    }

    if (des->services_json)
    {
        cJSON_Delete(des->services_json);
    }

    if (des->mes_js_str)
    {
        free(des->mes_js_str);
    }

    free(des);
}
