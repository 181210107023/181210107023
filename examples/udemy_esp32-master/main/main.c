/**
 * Application entry point.
 */
#include <stdio.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
// #include "my_data.h"

#include <esp_event.h>
#include <esp_system.h>
// #include <esp_https_server.h> 
// #define SSID "Bhargav"
// #define PASS "22102001"
#include "esp_log.h"
#include "rc522.h"
#include <stdio.h>
#include <string.h>

// static const char *TAG = "HTTPS";

// Define client certificate
extern const uint8_t ClientCert_pem_start[] asm("_binary_ClientCert_pem_start");
extern const uint8_t ClientCert_pem_end[]   asm("_binary_ClientCert_pem_end");

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_app.h"
#include "wifi_reset_button.h"
#include "ultrasonic.h"
static const char TAG[] = "main";
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SWITCH_LED_PIN  13
// #define WIFI_LED_PIN 2
#define ENTRY_LED_PIN 12
#define BUTTON1 26
#define BUTTON2 14
bool crud = false;
#define GPIO_TRIGGER	22
#define GPIO_ECHO	21
int isDataNotFound(const char *str) {
  if (strstr(str, "crud Data found!") != NULL && crud) {
    return 1; // "data not found" is present
	// 	gpio_set_level(GPIO_NUM_2, 1);
	// 	gpio_set_level(GPIO_NUM_18, 1);
  }
  else if(strstr(str, "crud Data not found!") != NULL && crud){
    return 2; // "data not found" is not present
  }
  else if(strstr(str, "read Data found!") != NULL){
    return 1; // "data not found" is not present
  }
  else if(strstr(str, "read Data not found!") != NULL){
    return 3; // "data not found" is not present
  }
  else{
    return 4;
  }
}

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    // size_t data_len = evt->data_len;
    // char *example = "<HTML>\n<HEAD>\n<TITLE>Moved Temporarily</TITLE>\n</HEAD>\n<BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#";
    // char *example = "<!doctype html>";
    // char htmlString[500];
    //  sprintf(htmlString, "<!doctype html>\n"
    //                   "<html>\n"
    //                   "<head>\n"
    //                   "<meta name=\"chromevox\" content-script=\"no\">\n"
    //                   "<title>Data not found</title>\n"
    //                   "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/icon?family=Material+Icons\" nonce=\"C7WgwDBP45r3DMrV7iRumA\">\n"
    //                   "<link rel=\"stylesheet\" href=\"/static/macros/client/css/2103210533-mae_html_css_ltr.css\">\n"
    //                   "<script type=\"text/javascript\" src=\"/static/macros/client/js/179929085-warden_bin_i18n_warden__en_gb.js\"></script>\n"
    //                   "</head>\n"
    //                   "<body>\n"
    //                   "<table id=\"warning-bar-table\" class=\"full_size\" cel\n");
    // ESP_LOGI(TAG, "Tag: %s",example);
    // int length_str = strlen(evt->data);
    // int example_len = strlen(example);
    // printf("%d",length_str);
    // printf("%d",example_len);
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        printf("Client HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
        if (isDataNotFound((char *)evt->data) == 1) {
            //  printf("found");
            gpio_set_level(ENTRY_LED_PIN, 1);
            vTaskDelay(100);
            gpio_set_level(ENTRY_LED_PIN, 0);
            vTaskDelay(50);
            gpio_set_level(ENTRY_LED_PIN, 1);
            vTaskDelay(100);
            gpio_set_level(ENTRY_LED_PIN, 0);
            // printf("The string contains 'data not found'.\n");
        } 
        else if (isDataNotFound((char *)evt->data) == 3) {
                gpio_set_level(ENTRY_LED_PIN, 1);
                gpio_set_level(SWITCH_LED_PIN, 1);
                vTaskDelay(100);
                gpio_set_level(ENTRY_LED_PIN, 0);
                gpio_set_level(SWITCH_LED_PIN, 0);
                vTaskDelay(50);
                gpio_set_level(ENTRY_LED_PIN, 1);
                gpio_set_level(SWITCH_LED_PIN, 1);
                vTaskDelay(100);
                gpio_set_level(ENTRY_LED_PIN, 0);
                gpio_set_level(SWITCH_LED_PIN, 1);
        }
        else if(isDataNotFound((char *)evt->data) == 2) {
                gpio_set_level(ENTRY_LED_PIN, 1);
                gpio_set_level(SWITCH_LED_PIN, 0);
                vTaskDelay(50);
                gpio_set_level(ENTRY_LED_PIN, 0);
                gpio_set_level(SWITCH_LED_PIN, 1);
                vTaskDelay(50);
                gpio_set_level(ENTRY_LED_PIN, 1);
                gpio_set_level(SWITCH_LED_PIN, 0);
                vTaskDelay(50);
                gpio_set_level(ENTRY_LED_PIN, 0);
                gpio_set_level(SWITCH_LED_PIN, 1);
                vTaskDelay(100);
                gpio_set_level(SWITCH_LED_PIN, 0);
        }
        else if(isDataNotFound((char *)evt->data) == 4){

        }
            // "data not found" is not present in the string
            // printf("The string does not contain 'data not found'.\n");
     
        // sprintf("my_variable = %zu\n", data_len);
        // printf("DATA %d",evt->data_len);
        // int length_str = strlen(evt->data);
        // int example_len = strlen(example);
        // printf("%d",length_str);
        // printf("%d",example_len);
        // int compare = strcmp(evt->data,htmlString);
        // // printf("\n%s",example);
        // if(compare == 0){
        //     printf("same");
        //     gpio_set_level(ENTRY_LED_PIN, 1); 
        //     vTaskDelay(5);
        //     gpio_set_level(ENTRY_LED_PIN, 0); 
        //  }
        // else{
        //     printf("not match");
        // }
        break;

    default:
        break;
    }
    return ESP_OK;
 } //                         https://script.google.com/macros/s/AKfycbxGVwU0g1jPe0b3dvck_Ia9_IbLQJXnJlSIIiy7qcyhKu4UGVrsdkg7J8RdGrFVU4Nl/exec
static const char* url1 = "https://script.google.com/macros/s/AKfycbxGVwU0g1jPe0b3dvck_Ia9_IbLQJXnJlSIIiy7qcyhKu4UGVrsdkg7J8RdGrFVU4Nl/exec?uid=";
static void client_post_entry_function(char* id)
{ 
    // char url1 = (char*)url;
    // char url1[] = "https://script.google.com/macros/s/AKfycbyN6G8oiIoee2buPEvdudM67B7bbdrfW5UrlwFkqbivbrIieJ7CnoqnhbbeDYeeGg/exec?uid=";
// ?strcat(url1,id);
    // ESP_LOGI(TAG, "Tag: %s",url1);
    char url_1[125];
    strcpy(url_1, url1);
    strcat(url_1,id);
    ESP_LOGI(TAG, "Tag: %s",url_1);
     esp_http_client_config_t config_get = {
        // .url = "https://script.google.com/macros/s/AKfycbwdlbkiIheEvOUd4ZD1B5CavJLQPaOqiWHKphtiC0p1Jc9ngDzbj7uYtK1_mjgAcBiM/exec?sensor=1230&date=8391",
        
        .url = url_1,
        .method = HTTP_METHOD_GET,
        .cert_pem = (const char *)ClientCert_pem_start,
        .event_handler = client_event_get_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    // char url1[] = "https://script.google.com/macros/s/AKfycbyN6G8oiIoee2buPEvdudM67B7bbdrfW5UrlwFkqbivbrIieJ7CnoqnhbbeDYeeGg/exec?uid=";    // heap_caps_free(url1);
}                                  // https://script.google.com/macros/s/AKfycbwTzrxGJgJj-nGuiaV4ngv4H31dHJDx0w6isNuReB0c8WLAIi--hw_CBU-EAMnUGse54Q/exec
static const char* register_url1 = "https://script.google.com/macros/s/AKfycbwTzrxGJgJj-nGuiaV4ngv4H31dHJDx0w6isNuReB0c8WLAIi--hw_CBU-EAMnUGse54Q/exec?uid=";
static void client_post_register_function(char* id)
{ 
    // char url1 = (char*)url;
    // char url1[] = "https://script.google.com/macros/s/AKfycbyN6G8oiIoee2buPEvdudM67B7bbdrfW5UrlwFkqbivbrIieJ7CnoqnhbbeDYeeGg/exec?uid=";
// ?strcat(url1,id);
    // ESP_LOGI(TAG, "Tag: %s",url1);
    char register_url_1[135];
    strcpy(register_url_1, register_url1);
    strcat(register_url_1,id);
    ESP_LOGI(TAG, "Tag: %s",register_url_1);
     esp_http_client_config_t config_get = {
        // .url = "https://script.google.com/macros/s/AKfycbwdlbkiIheEvOUd4ZD1B5CavJLQPaOqiWHKphtiC0p1Jc9ngDzbj7uYtK1_mjgAcBiM/exec?sensor=1230&date=8391",
        
        .url = register_url_1,
        .method = HTTP_METHOD_GET,
        .cert_pem = (const char *)ClientCert_pem_start,
        .event_handler = client_event_get_handler};

    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    // char url1[] = "https://script.google.com/macros/s/AKfycbyN6G8oiIoee2buPEvdudM67B7bbdrfW5UrlwFkqbivbrIieJ7CnoqnhbbeDYeeGg/exec?uid=";    // heap_caps_free(url1);
}
void uint8ArrayToString(uint8_t *array, size_t arrayLength, char *string)
{
    for (size_t i = 0; i < arrayLength; i++)
    {
        sprintf(&string[i * 2], "%02X", array[i]);
    }
}

void tag_handler(uint8_t* sn) { // serial number is always 5 bytes long
    // ESP_LOGI(TAG, "Tag: %i %i %i %i %i",
    //     sn[0], sn[1], sn[2], sn[3], sn[4]
    // );

    size_t array_length = 5;

    // Convert the uint8_t array to a string
    char my_string[array_length * 2 + 1]; // +1 for null terminator
    uint8ArrayToString(sn, array_length, my_string);
    printf("String: %s\n", my_string);
    // client_post_entry_function(my_string);
    //  if (gpio_get_level(BUTTON1) == 1)
    //     {   crud =true;
    //         // client_post_entry_function(my_string);
    //         gpio_set_level(SWITCH_LED_PIN, 1); 
    //         // gpio_set_level(LED_1_PIN, 0); 

    //     } 
    // if (gpio_get_level(BUTTON2) == 1)
    //     {  crud = false;
    //         // client_post_register_function(my_string);
    //         gpio_set_level(SWITCH_LED_PIN, 0); 
    //         // gpio_set_level(LED_PIN, 1); 
    //         // gpio_set_level(LED_1_PIN, 0); 

    //     }
    if (crud)
        {   
            client_post_register_function(my_string);
            gpio_set_level(SWITCH_LED_PIN, 1); 
            // gpio_set_level(LED_1_PIN, 0); 

        } 
    else
        {  
            client_post_entry_function(my_string);
            gpio_set_level(SWITCH_LED_PIN, 0); 
            // gpio_set_level(LED_PIN, 1); 
            // gpio_set_level(LED_1_PIN, 0); 

        } 
    return my_string;
    // client_post_rest_function(my_string);
    // char url_1[200];
    // strcpy(url_1, url1);
    // strcat(url_1,my_string);
    // ESP_LOGI(TAG, "Tag: %s",url_1);
    // url1[]="";
    // char* input2 = (char *) sn;
    // ESP_LOGI(TAG, "Tag: %d",input2);
}

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
    gpio_set_direction(SWITCH_LED_PIN, GPIO_MODE_OUTPUT); 
    // gpio_set_direction(WIFI_LED_PIN, GPIO_MODE_OUTPUT);   
    // gpio_set_direction(ENTRY_LED_PIN, GPIO_MODE_OUTPUT);

    gpio_set_direction(BUTTON1, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON2, GPIO_MODE_INPUT);
    
     /* Set the GPIO pull */
    gpio_set_pull_mode(BUTTON1, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(BUTTON2, GPIO_PULLUP_ONLY);
    if (gpio_get_level(BUTTON1) == 1)
        {   crud =true;
            // client_post_entry_function(my_string);
            gpio_set_level(SWITCH_LED_PIN, 1); 
            // gpio_set_level(LED_1_PIN, 0); 

        } 
    if (gpio_get_level(BUTTON2) == 1)
        {  crud = false;
            // client_post_register_function(my_string);
            gpio_set_level(SWITCH_LED_PIN, 0); 
            // gpio_set_level(LED_PIN, 1); 
            // gpio_set_level(LED_1_PIN, 0); 

        }
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
	     const rc522_start_args_t start_args = {
        .miso_io  = 25,
        .mosi_io  = 23,
        .sck_io   = 19,
        .sda_io   = 22,
        .callback = &tag_handler,

        // Uncomment next line for attaching RC522 to SPI2 bus. Default is VSPI_HOST (SPI3)
        //.spi_host_id = HSPI_HOST
    };
    
    //  if (gpio_get_level(BUTTON1) == 1)
    //     {   crud =true;
    //         // client_post_entry_function(my_string);
    //         gpio_set_level(SWITCH_LED_PIN, 1); 
    //         // gpio_set_level(LED_1_PIN, 0); 

    //     } 
    // if (gpio_get_level(BUTTON2) == 1)
    //     {  crud = false;
    //         // client_post_register_function(my_string);
    //         gpio_set_level(SWITCH_LED_PIN, 0); 
    //         // gpio_set_level(LED_PIN, 1); 
    //         // gpio_set_level(LED_1_PIN, 0); 

    //     }
             gpio_reset_pin(ENTRY_LED_PIN);
    /* Set the GPIO as a push/pull output */
            gpio_reset_pin(SWITCH_LED_PIN);
    /* Set the GPIO as a push/pull output */
            gpio_set_direction(SWITCH_LED_PIN, GPIO_MODE_OUTPUT);
            gpio_set_direction(ENTRY_LED_PIN, GPIO_MODE_OUTPUT);
            
    char string = rc522_start(start_args);
    //  if (gpio_get_level(BUTTON1) == 1)
    //     {  
    //             // client_post_rest_function(my_string);
    //         gpio_set_level(SWITCH_LED_PIN, 1); 
    //         // gpio_set_level(LED_1_PIN, 0); 

    //     } 
    // if (gpio_get_level(BUTTON2) == 1)
    //     {  
    //         // gpio_set_level(LED_PIN, 1); 
    //         gpio_set_level(SWITCH_LED_PIN, 0); 

    //     } 
    printf("String: %c\n", string);
}

