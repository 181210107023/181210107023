// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

#include "iothub_client.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothubtransportmqtt.h"
#include "iothub_client_options.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ultrasonic.h"
#ifdef MBED_BUILD_TIMESTAMP
    #define SET_TRUSTED_CERT_IN_SAMPLES
#endif // MBED_BUILD_TIMESTAMP

#ifdef SET_TRUSTED_CERT_IN_SAMPLES
    #include "certs.h"
#endif // SET_TRUSTED_CERT_IN_SAMPLES

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
#define EXAMPLE_IOTHUB_CONNECTION_STRING CONFIG_IOTHUB_CONNECTION_STRING
static const char* connectionString = "HostName=bm-iot.azure-devices.net;DeviceId=solarpanel;SharedAccessKey=wSFEv3Ja3OJb5h8EKqzcRw4PUfVrgxdY5i4d/96r9Gs=";
#define GPIO_TRIGGER	22
#define GPIO_ECHO	21
static int callbackCounter;
static char msgText[1024];
static char propText[1024];
static bool g_continueRunning;
#define MESSAGE_COUNT 50
#define DOWORK_LOOP_NUM     3

typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId;  // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    int* counter = (int*)userContextCallback;
    const char* buffer;
    size_t size;
    MAP_HANDLE mapProperties;
    const char* messageId;
    const char* correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<null>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<null>";
    }

    // Message content
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        (void)printf("unable to retrieve the message data\r\n");
    }
    else
    {
        (void)printf("Received Message [%d]\r\n Message ID: %s\r\n Correlation ID: %s\r\n Data: <<<%.*s>>> & Size=%d\r\n", *counter, messageId, correlationId, (int)size, buffer, (int)size);
        // If we receive the work 'quit' then we stop running
        if (size == (strlen("quit") * sizeof(char)) && memcmp(buffer, "quit", size) == 0)
        {
            g_continueRunning = false;
        }
    }

    // Retrieve properties from the message
    mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
                size_t index;

                printf(" Message Properties:\r\n");
                for (index = 0; index < propertyCount; index++)
                {
                    (void)printf("\tKey: %s Value: %s\r\n", keys[index], values[index]);
                }
                (void)printf("\r\n");
            }
        }
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;
    size_t id = eventInstance->messageTrackingId;

    if (result == IOTHUB_CLIENT_CONFIRMATION_OK) {
        (void)printf("Confirmation[%d] received for message tracking id = %d with result = %s\r\n", callbackCounter, (int)id, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
        /* Some device specific action code goes here... */
        callbackCounter++;
    }
    IoTHubMessage_Destroy(eventInstance->messageHandle);
}

void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
    (void)printf("\n\nConnection Status result:%s, Connection Status reason: %s\n\n", MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS, result),
                 MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONNECTION_STATUS_REASON, reason));
}

void iothub_client_sample_mqtt_run(void)
{
    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

    EVENT_INSTANCE message;
    ultrasonic_sensor_t sensor = {
		.trigger_pin = GPIO_TRIGGER,
		.echo_pin = GPIO_ECHO
	};
    ultrasonic_init(&sensor);
    
    g_continueRunning = true;
    srand((unsigned int)time(NULL));
    // double avgWindSpeed = 10.0;
    double minTemperature = 20.0;
    // double minHumidity = 60.0;

    callbackCounter = 0;
    int receiveContext = 0;

    if (platform_init() != 0)
    {
        (void)printf("Failed to initialize the platform.\r\n");
    }
    else
    {
        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) == NULL)
        {
            (void)printf("ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            bool traceOn = true;
            IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &traceOn);

            IoTHubClient_LL_SetConnectionStatusCallback(iotHubClientHandle, connection_status_callback, NULL);
            // Setting the Trusted Certificate.  This is only necessary on system with without
            // built in certificate stores.
#ifdef SET_TRUSTED_CERT_IN_SAMPLES
            IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_TRUSTED_CERT, certificates);
#endif // SET_TRUSTED_CERT_IN_SAMPLES

            /* Setting Message call back, so we can receive Commands. */
            if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
            {
                (void)printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
            }
            else
            {
                (void)printf("IoTHubClient_LL_SetMessageCallback...successful.\r\n");

                /* Now that we are ready to receive commands, let's send some messages */
                int iterator = 0;
                double temperature = 0;
                // double humidity = 0;
                time_t sent_time = 0;
                time_t current_time = 0;
            	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	            gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
	            gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
	            gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
                do
                {
                    //(void)printf("iterator: [%d], callbackCounter: [%d]. \r\n", iterator, callbackCounter);
                    time(&current_time);
                    uint32_t distance;
	                esp_err_t res = ultrasonic_measure_cm(&sensor, 200, &distance);
                    printf("Distance: %d cm, %.02f m\n", distance, distance / 100.0);

                     if(distance>15 && distance<20){
	                 	gpio_set_level(GPIO_NUM_2, 1);
	                 	gpio_set_level(GPIO_NUM_18, 0);
	                 	gpio_set_level(GPIO_NUM_5, 0);
	                 }
	                 else if(distance>10 && distance< 15){
	                 	gpio_set_level(GPIO_NUM_5, 1);
	                 	gpio_set_level(GPIO_NUM_2, 1);
	                 	gpio_set_level(GPIO_NUM_18, 0);
	                 }
	                 else if(distance<10){
	                 	gpio_set_level(GPIO_NUM_5, 1);
	                 	gpio_set_level(GPIO_NUM_2, 1);
	                 	gpio_set_level(GPIO_NUM_18, 1);
	                 }
	                 vTaskDelay(5000 / portTICK_PERIOD_MS);
                    // if(distance < dustbin_level_2){

                    // }
                    // else if (distance > dustbin_level_2 && distance < dustbin_level_1)
                    // {

                    // }
                    // else if (distance > dustbin_level_1 && distance < dustbin_level_0)
                    // {

                    // }
                    
                    
                    if ((MESSAGE_COUNT == 0 || iterator < MESSAGE_COUNT)
                        && iterator <= callbackCounter && distance < 10
                        && (difftime(current_time, sent_time) > ((30000) / 1000)))
                    {
                        //temperature = minTemperature + (rand() % 10);
                        // humidity = minHumidity +  (rand() % 20);
                        // uint32_t distance;
	                    // esp_err_t res = ultrasonic_measure_cm(&sensor, 500, &distance);
                        // printf("Distance: %d cm, %.02f m\n", distance, distance / 100.0);
                        sprintf_s(msgText, sizeof(msgText), "{distance:%d}",distance);
                        //  (void)printf("{temprature:24,humidity:50}");
                        if ((message.messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL)
                        {
                            (void)printf("ERROR: iotHubMessageHandle is NULL!\r\n");
                        }
                        else
                        {
                            message.messageTrackingId = iterator;
                            MAP_HANDLE propMap = IoTHubMessage_Properties(message.messageHandle);
                            (void)sprintf_s(propText, sizeof(propText), temperature > 28 ? "true" : "false");
                            if (Map_AddOrUpdate(propMap, "temperatureAlert", propText) != MAP_OK)
                            {
                                (void)printf("ERROR: Map_AddOrUpdate Failed!\r\n");
                            }

                            if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, message.messageHandle, SendConfirmationCallback, &message) != IOTHUB_CLIENT_OK)
                            {
                                (void)printf("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
                            }
                            else
                            {
                                time(&sent_time);
                                (void)printf("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", (int)iterator);
                            }
                        }
                        iterator++;
                    }
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(10);

                    if (MESSAGE_COUNT != 0 && callbackCounter >= MESSAGE_COUNT)
                    {
                        printf("exit\n");
                        break;
                    }
                } while (g_continueRunning);

                (void)printf("iothub_client_sample_mqtt has gotten quit message, call DoWork %d more time to complete final sending...\r\n", DOWORK_LOOP_NUM);
                size_t index = 0;
                for (index = 0; index < DOWORK_LOOP_NUM; index++)
                {
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(1);
                }
            }
            IoTHubClient_LL_Destroy(iotHubClientHandle);
        }
        platform_deinit();
    }
}

int main(void)
{
    // iothub_client_sample_mqtt_run();
    return 0;
}
