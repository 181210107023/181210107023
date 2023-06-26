/**
 * Application entry point.
 */

#include "esp_log.h"
#include "nvs_flash.h"


#include "wifi_app.h"
#include "wifi_reset_button.h"
#include "ultrasonic.h"
static const char TAG[] = "main";
#define GPIO_TRIGGER	22
#define GPIO_ECHO	21

void wifi_application_connected_events(void)
{
	ESP_LOGI(TAG, "WiFi Application Connected!!");

}

void app_main(void)
{
    // // Initialize NVS
	// ultrasonic_sensor_t sensor = {
	// 	.trigger_pin = GPIO_TRIGGER,
	// 	.echo_pin = GPIO_ECHO
	// };
	// gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	// gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
	// gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
	// gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
	// gpio_set_level(GPIO_NUM_2, 1);
	// //  gpio_set_level(GPIO_NUM_4, 1);
	// // gpio_set_level(GPIO_NUM_5, 1);
	// // gpio_set_level(GPIO_NUM_18, 1);
	
    // ultrasonic_init(&sensor);
	// uint32_t distance;
	// while(true){
	// esp_err_t res = ultrasonic_measure_cm(&sensor, 200, &distance);
    // printf("Distance: %d cm, %.02f m\n", distance, distance / 100.0);
	// if(distance>15 && distance<20){
	// 	gpio_set_level(GPIO_NUM_2, 1);
	// 	gpio_set_level(GPIO_NUM_18, 0);
	// 	gpio_set_level(GPIO_NUM_5, 0);
	// }
	// else if(distance>10 && distance< 15){
	// 	gpio_set_level(GPIO_NUM_5, 1);
	// 	gpio_set_level(GPIO_NUM_2, 1);
	// 	gpio_set_level(GPIO_NUM_18, 0);
	
	// }
	// else if(distance<10){
	// 	gpio_set_level(GPIO_NUM_5, 1);
	// 	gpio_set_level(GPIO_NUM_2, 1);
	// 	gpio_set_level(GPIO_NUM_18, 1);
	// }
	// vTaskDelay(5000 / portTICK_PERIOD_MS);

	// }
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start Wifi
	wifi_app_start();

	// Configure Wifi reset button
	wifi_reset_button_config();

	// Start DHT22 Sensor task


	// Set connected event callback
	wifi_app_set_callback(&wifi_application_connected_events);
}

