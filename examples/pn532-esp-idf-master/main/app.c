#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>
#include <esp_log_internal.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "sdkconfig.h"

#include "pn532.h"

#define BLINK_GPIO CONFIG_BLINK_GPIO

#define PN532_SCK CONFIG_PN532_SCK
#define PN532_MOSI CONFIG_PN532_MOSI
#define PN532_SS CONFIG_PN532_SS
#define PN532_MISO CONFIG_PN532_MISO

static const char *TAG = "APP";

static pn532_t nfc;

// void blink_task(void *pvParameter)
// {
//     gpio_pad_select_gpio(BLINK_GPIO);
//     gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
//     while (1)
//     {
//         gpio_set_level(BLINK_GPIO, 0);
//         vTaskDelay(900 / portTICK_PERIOD_MS);
//         gpio_set_level(BLINK_GPIO, 1);
//         vTaskDelay(100 / portTICK_PERIOD_MS);
//     }
// }

// void nfc_task(void *pvParameter)
// {
//     pn532_spi_init(&nfc, PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
//     pn532_begin(&nfc);

//     uint32_t versiondata = pn532_getFirmwareVersion(&nfc);
//     if (!versiondata)
//     {
//         ESP_LOGI(TAG, "Didn't find PN53x board");
//         while (1)
//         {
//             vTaskDelay(1000 / portTICK_RATE_MS);
//         }
//     }
//     // Got ok data, print it out!
//     ESP_LOGI(TAG, "Found chip PN5 %x", (versiondata >> 24) & 0xFF);
//     ESP_LOGI(TAG, "Firmware ver. %d.%d", (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);

//     // configure board to read RFID tags
//     pn532_SAMConfig(&nfc);

//     ESP_LOGI(TAG, "Waiting for an ISO14443A Card ...");

//     while (1)
//     {
//         uint8_t success;
//         // uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
//         // uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
//         uint8_t data[16];  
//         // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
//         // 'uid' will be populated with the UID, and uidLength will indicate
//         // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
//         // success = pn532_readPassiveTargetID(&nfc, PN532_MIFARE_ISO14443A, uid, &uidLength, 0);

//         for(int block=0;block<16;block++){
//         success =pn532_mifareclassic_ReadDataBlock(&nfc,block,data);
//         if (success)
//         {
//             // Display some basic information about the card
//             ESP_LOGI(TAG, "Found an ISO14443A card");
//             for (int i = 0; i < 16; i++) {
//                 printf("%02X ", data[i]);
//             }
//             printf("\n");
//             // ESP_LOGI(TAG, "data: % bytes", data);
//             // ESP_LOGI(TAG, "UID Value:");
//             // esp_log_buffer_hexdump_internal(TAG, uid, uidLength, ESP_LOG_INFO);   
//             vTaskDelay(1000 / portTICK_RATE_MS);         
//         }
//         else
//         {
//             // PN532 probably timed out waiting for a card
//             ESP_LOGI(TAG, "Timed out waiting for a card");
//         }
//         }
//     }
// }
void nfc_read_task(void *pvParameters)
{
    pn532_t pn532;
    uint8_t uid[7];  // Buffer to store the UID of the NFC card
    uint8_t uidLen;  // Length of the UID

    // Initialize PN532 module
    pn532_init(&pn532);

    while (1)
    {
        // Configure PN532 as I2C or SPI as per your wiring
        // For example, if using I2C:
        // pn532_begin(&pn532, PN532_I2C_MODE, 400000);
        // For example, if using SPI:
        // pn532_begin(&pn532, PN532_SPI_MODE, 1000000);

        // Start the NFC reader
        pn532_begin(&pn532);

        // Configure the reader for the desired NFC mode
        pn532_SAMConfig(&pn532);

        // Wait for an NFC card to be detected
        if (pn532_readPassiveTargetID(&pn532, PN532_MIFARE_ISO14443A, uid, &uidLen))
        {
            printf("Found an NFC card!\n");

            // Print the UID of the card
            printf("UID Length: %d bytes\n", uidLen);
            printf("UID Value: ");
            for (int i = 0; i < uidLen; i++)
            {
                printf("%02X ", uid[i]);
            }
            printf("\n");

            // Read all 16 sectors of the NFC card
            for (uint8_t sector = 0; sector < 16; sector++)
            {
                // Authenticate with the sector using a default key (Key A)
                if (pn532_MIFARE_AuthenticateBlock(&pn532, uid, uidLen, sector * 4, PN532_MIFARE_CMD_AUTH_A, (uint8_t *)"FFFFFFFFFFFF"))
                {
                    printf("Authenticated with sector %d\n", sector);

                    // Read all blocks in the sector
                    for (uint8_t block = sector * 4; block < (sector * 4) + 4; block++)
                    {
                        uint8_t data[16];
                        if (pn532_MIFARE_ReadBlock(&pn532, block, data))
                        {
                            printf("Block %d Data: ", block);
                            for (int i = 0; i < 16; i++)
                            {
                                printf("%02X ", data[i]);
                            }
                            printf("\n");
                        }
                        else
                        {
                            printf("Failed to read block %d\n", block);
                        }
                    }
                }
                else
                {
                    printf("Authentication failed for sector %d\n", sector);
                }
            }
        }
        else
        {
                   {
           printf("No NFC card detected\n");
       }

       // Delay before attempting to read another NFC card
       vTaskDelay(2000 / portTICK_PERIOD_MS);
   }
    }
}

void app_main()
{
    // xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(&nfc_read_task, "nfc_task", 4096, NULL, 4, NULL);
}
