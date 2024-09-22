//ESP32 IDF version 5.1.4
//Author: M G Vybhav Nag
//Email: vybhav@craftpower.cc

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"                               //ESP32 RTOS
#include "freertos/task.h"                                   //ESP32 RTOS task Handler
#include "esp_system.h"                                      //ESP32 System library to idetify handle warkings, errors, exceptions etc
#include "esp_wifi.h"
#include "esp_event.h"                                       //ESP32 Event Handler - to handle Wifi, Web socket events
#include "nvs_flash.h"                                       //ESP32 File system driver
#include "esp_log.h"                                         //Serial Logs library
#include "driver/i2s.h"                                      //ESP32 I2S Library
#include "esp_websocket_client.h"                            //Web Socket Client Library - External, need to install from component library
#include "lwip/sockets.h"
#include "esp_netif.h"                                       //ESP32 Internet Driver - can be used to configure WiFi, ethernet, GSM etc...
#include "esp_err.h"                                         //ESP32 Error Database & Handler

// WiFi Configuration
#define WIFI_SSID               "WiFi"                        //Enter you Wifi SSID here
#define WIFI_PASS               "wifi_password"               //Enter you WiFi password here

// WebSocket Configuration
#define WEBSOCKET_URI           "ws://yourserver:port"        //URL of web socket client

// I2S Configuration
#define I2S_BITS_PER_SAMPLE     I2S_BITS_PER_SAMPLE_16BIT    //Audio Bitrate as per Speaker+Mic Datasheet - 16 Bit default
#define I2S_CHANNEL_FORMAT      I2S_CHANNEL_FMT_ONLY_LEFT    //Audio Speaker+Mic Channel - Left only default
#define I2S_COMM_FORMAT         I2S_COMM_FORMAT_STAND_I2S    //I2S Audio driver communication standard - Philips default
#define SAMPLE_RATE             16000                        //Audio Sample rate - 16Khz default
#define I2S_NUM                 I2S_NUM_0                    //I2S Port - Default PORT 0 [Use only port 0 if using more than 1 mic]
#define I2S_BUFFER_SIZE         2048                         //I2S Buffer Size - 2048 default, ffor 2 channels 1048 samples = 2048 bytes of data is captured], anything higher will crash / bootloop ESP

// I2S GPIO Configuration - No default config, configure as per your hardware
#define I2S_BCK_IO              26                           //ESP32 Pin to Speaker&Mic BCK
#define I2S_WS_IO               25                           //ESP32 Pin to Speaker&Mic WS
#define I2S_DATA_OUT_IO         22                           //ESP32 Pin to Speaker Input
#define I2S_DATA_IN_IO          23                           //ESP32 Pin to Mic Input

// WebSocket buffer and messages
#define STOP_MESSAGE            "Stop"                       //Default Stop code sent to web server after 10 seconds of audio
#define WS_BUFFER_SIZE          2048                         //WS buffer, same as I2S Audio Buffer size

static const char *TAG = "Audio_WS";                         //Tag for LOGS

// WebSocket client handle
esp_websocket_client_handle_t ws_client;

//Declare RTOS Task andd Functions
void send_audio_data(void *arg);
void websocket_task(void *pvParameters);

//Wifi Setup & Configuration
void wifi_init(void) {
    esp_netif_init();                                       //Initiate netif as base for internet configuration
    esp_event_loop_create_default();                        //Initiate default netif to handle wifi exceptions
    esp_netif_create_default_wifi_sta();                    //Create a WiFi client

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();    //Create WiFi handler
    esp_wifi_init(&cfg);                                    //Start WiFi client

//Set WiFi SSID and Password
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,                              //WiFi SSID Configured in line 21
            .password = WIFI_PASS,                          //WiFi Password Configured in line 22
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);                       //Set WiFi as Client / Station
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);     //Send Credentials to WiFi pheriperal
    esp_wifi_start();                                       //Turn ON WiFi
    esp_wifi_connect();                                     //Connect to WiFi Accesspoint configured
}

// I2S Audio Driver - Ensure choosen Hardware is compatible and both Mic and Speaker has same Sample rate, Bit rate, Channels, communication format
void i2s_init(void) {

    //Set I2S driver configuration
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX, //Start ESP as I2S Master and allow both Mi and Speaker
        .sample_rate = SAMPLE_RATE,                          //Sample rate configured in line 31
        .bits_per_sample = I2S_BITS_PER_SAMPLE,              //Bitrate configured in line 28
        .channel_format = I2S_CHANNEL_FORMAT,                //Audio Channel config - configured in line 29
        .communication_format = I2S_COMM_FORMAT,             //Communication standard used - configured in line 30
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,            //Do Not change
        .dma_buf_count = 8,                                  //Do Not change - Number of allocated Buffers
        .dma_buf_len = I2S_BUFFER_SIZE,                      //Do not increase more thank 2048 - I2S Audio Buffer configured in line 33
        .use_apll = false,                                   //High prescision Master clock, not required for most new devices
    };
    
    //Set I2S Pin connections to I2S driver
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,                            // Configurable BCK pin
        .ws_io_num = I2S_WS_IO,                              // Configurable WS pin
        .data_out_num = I2S_DATA_OUT_IO,                     // Configurable data out pin
        .data_in_num = I2S_DATA_IN_IO,                       // Configurable data in pin
    };

    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);       //Install the driver for I2S audio
    i2s_set_pin(I2S_NUM, &pin_config);                       //Direct the driver to GPIO port I2S device is connected to
}

// Web socket Event Handler - Interrupt
void websocket_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

    if (event_id == WEBSOCKET_EVENT_DATA) {                 //Check if event is due to new data recieved from server
        // Play received data on speaker
        if (data->op_code == WS_TRANSPORT_OPCODES_BINARY) { //Check if recieved data is binary [raw data transported in WS is binary irrespective of actual data encoding]
            int len = data->data_len;                       //Get Data length for Audio driver
            char *audio_buffer = (char *)data->data_ptr;    // Changed data from binary to char*

            size_t bytes_written = 0;
            i2s_write(I2S_NUM, audio_buffer, len, &bytes_written, portMAX_DELAY); //Play audio on Speaker
        }
    }
}

//Configure and Initiate websocket client
void websocket_init(void) {

    // set websocket URL
    esp_websocket_client_config_t websocket_cfg = {
        .uri = WEBSOCKET_URI,                              //Websocket URL configured in line 25
    };

    ws_client = esp_websocket_client_init(&websocket_cfg); //Create Websocket Handler
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL); //Setup Websocket event handler link
    esp_websocket_client_start(ws_client);                 //Start and connect Websocket client to server
}

// Function to continuously record and send PCM audio data to WS server
void send_audio_data(void *arg) {
    char audio_buffer[I2S_BUFFER_SIZE];  // Changed to char
    size_t bytes_read;

    while (true) {
        // Record audio for 20 seconds, sending data in chunks of 2048 samples
        for (int i = 0; i < (SAMPLE_RATE / (I2S_BUFFER_SIZE / 2)) * 20; i++) {
            i2s_read(I2S_NUM, audio_buffer, I2S_BUFFER_SIZE, &bytes_read, portMAX_DELAY);                     //record 2048 samples of PCM audio to buffer - audio buffer is char, so int16 is saved as HEX
            esp_websocket_client_send_bin(ws_client, (const char *)audio_buffer, bytes_read, portMAX_DELAY);  // signed int16 to const char* [HEX, little endian] - send raw PCM data of 2048 sample buffer to Websocket server
        }

        // Send "Stop" message after recording
        esp_websocket_client_send_text(ws_client, STOP_MESSAGE, strlen(STOP_MESSAGE), portMAX_DELAY);         //at end of 20 seconds send stop code to server - configured in line 42
        
        //Allow time for other tasks, such as WiFi, Websocket, Recieving audio from server
        taskYIELD();
    }
}

// RTOS task for Audio record and transmit
void websocket_task(void *pvParameters) {
    send_audio_data(NULL);                 //call audio record task
}

//Main Loop
void app_main(void) {
    esp_log_level_set("*", ESP_LOG_INFO); //Logs
    nvs_flash_init();                     //Initiate file system
    
    wifi_init();                          //Initiate WiFi
    i2s_init();                           //Initiate I2S Audio
    websocket_init();                     //Initiate Websocket client

    xTaskCreate(websocket_task, "websocket_task", 8192, NULL, 5, NULL); //Configure RTOS Task
}
