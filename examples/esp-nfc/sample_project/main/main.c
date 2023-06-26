#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "PN532.h"

#define PN532_UART_PORT     UART_NUM_2
#define PN532_TX_PIN        17
#define PN532_RX_PIN        16
#define PN532_RST_PIN       2

static const char *TAG = "PN532";

void uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    uart_param_config(PN532_UART_PORT, &uart_config);
    uart_set_pin(PN532_UART_PORT, PN532_TX_PIN, PN532_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(PN532_UART_PORT, 1024, 0, 0, NULL, 0);
}

void pn532_uart_write(const uint8_t *data, size_t len)
{
    uart_write_bytes(PN532_UART_PORT, (const char *)data, len);
}

int pn532_uart_read(uint8_t *data, size_t len, uint32_t timeout)
{
    int read_len = 0;
    size_t rx_len = 0;

    while (rx_len < len) {
        read_len = uart_read_bytes(PN532_UART_PORT, &data[rx_len], len - rx_len, timeout / portTICK_RATE_MS);
        if (read_len <= 0) {
            break;
        }
        rx_len += read_len;
    }

    return rx_len;
}

void pn532_delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void pn532_log(const char *message)
{
    ESP_LOGI(TAG, "%s", message);
}

void app_main()
{
    ESP_LOGI(TAG, "PN532 ESP32 USART Example");

    uart_init();

    pn532_init(pn532_uart_write, pn532_uart_read, pn532_delay, pn532_log);
    pn532_begin();

    uint32_t versiondata = pn532_getFirmwareVersion();
    if (!versiondata) {
        ESP_LOGE(TAG, "Didn't find PN53x board");
        return;
    }

    pn532_SAMConfig();

    ESP_LOGI(TAG, "Waiting for NFC card or tag...");

    while (1) {
        uint8_t success;
        uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
        uint8_t uidLength;

        success = pn532_readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200);

        if (success) {
            ESP_LOGI(TAG, "Found an NFC card or tag!");
            ESP_LOGI(TAG, "UID Length: %d", uidLength);
            ESP_LOGI(TAG, "UID Value: ");
            for (uint8_t i = 0; i < uidLength; i++) {
                ESP_LOGI(TAG, " 0x%02X", uid[i]);
            }
            ESP_LOGI(TAG, "");

            uint8_t key[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            uint8_t block = 4;
            uint8_t data[16];

            if (pn532_authenticateBlock(uid, uidLength, block, PN532_AUTH_KEY_A, key) == PN532_STATUS_OK) {
                if (pn532_readMemoryBlock(block, data) == PN532_STATUS_OK) {
                    ESP_LOGI(TAG, "Block %d Data: ", block);
                    for (int i = 0; i < 16; i++) {
                        ESP_LOGI(TAG, " 0x%02X", data[i]);
                    }
                    ESP_LOGI(TAG, "");
                } else {
                    ESP_LOGE(TAG, "Read block failed");
                }
            } else {
                ESP_LOGE(TAG, "Authentication failed");
            }

            pn532_halt();
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
