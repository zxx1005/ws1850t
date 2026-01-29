/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "iic_test.h"

#include "esp_console.h"
#include "esp_vfs_fat.h"
#include "cmd_i2ctools.h"
#include "driver/i2c_master.h"

#include "sc7u22.h"
#include "i2s_es8311.h"
#include "nfc.h"
#include "ws1850_iic.h"

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

// static gpio_num_t i2c_gpio_sda = CONFIG_EXAMPLE_I2C_MASTER_SDA;
// static gpio_num_t i2c_gpio_scl = CONFIG_EXAMPLE_I2C_MASTER_SCL;

// static i2c_port_t i2c_port = I2C_NUM_0;

static void blink_led_on(void)
{

    for (uint8_t i = 0; i < 9; i++)
    {
        led_strip_set_pixel(led_strip, i, 125, 125, 125);
        // led_strip_set_pixel(led_strip, i, 16, 16, 16);
    }

    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);

    vTaskDelay(pdMS_TO_TICKS(3000));

    led_strip_clear(led_strip);
}

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    uint8_t i = 0;
    // s_led_state = get_led_state();
    s_led_state = get_nfc_state();
    if (s_led_state)
    {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        for (i = 0; i < 8; i++)
        {
            // led_strip_set_pixel(led_strip, i, 153, 153, 153);
            // led_strip_set_pixel(led_strip, i, 255, 255, 255);
            led_strip_set_pixel(led_strip, i, 125, 125, 125);
            // led_strip_set_pixel(led_strip, i, 16, 16, 16);
        }

        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    }
    else
    {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 8, // 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif

static void blink_led_task(void *arg)
{

    /* Configure the peripheral according to the LED type */
    configure_led();

    // blink_led_on();

    while (1)
    {
        // ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        // s_led_state =0;
        // s_led_state =1;
        // s_led_state = !s_led_state;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void iic_tool_task(void *arg)
{

    // iic_init_test();
    // iic_detect_test();
    // iic_dev_add_bus();



    while (1)
    {
        // iic_read_charge_data();
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // iic_detect_test();
        // vTaskDelay(pdMS_TO_TICKS(2000));

    }
}
static void iic_tool_task1(void *arg)
{

    // iic_init_test1();
    // iic_detect_test1();
    // iic_dev_add_bus1();

    ws1850_NFC_gpio_init();
    ws_iic_init();

    PcdReset();

    while (1)
    {
        // iic_read_nfc_data();
        Card_Check();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
static void sc7u22_task(void *arg)
{

    int16_t sensor_raw[6]; // Raw data from

    
    SC7U22_Init();
    SC7U22_Check();

    while (1)
    {
        SL_SC7U22_RawData_Read(sensor_raw);
        printf("SC7U22 Accel data: %d, %d, %d\r\n", sensor_raw[0], sensor_raw[1], sensor_raw[2]);
        printf("SC7U22 Gyro data: %d, %d, %d\r\n", sensor_raw[3], sensor_raw[4], sensor_raw[5]);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void ws1850_task(void *arg)
{
    printf("WS1850 NFC test start\n-----------------------------\n");
    
    ws1850_NFC_gpio_init();
    ws_iic_init();
    
    PcdReset();
    while (1)
    {
        // Card_Check();

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{

    gpio_init_test();

    iic_init_test();
    iic_detect_test();
    // iic_dev_add_bus();


    // xTaskCreate(blink_led_task, "blink_led_task", 1024 * 10, NULL, tskIDLE_PRIORITY +1, NULL);
    xTaskCreate(blink_led_task, "blink_led_task", 1024 * 10, NULL, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(iic_tool_task, "iic_tool_task", 1024 * 10, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(iic_tool_task1, "iic_tool_task1", 1024 * 10, NULL, configMAX_PRIORITIES - 4, NULL);
    xTaskCreate(sc7u22_task, "sc7u22_task", 1024 * 10, NULL, configMAX_PRIORITIES - 5, NULL);
    // xTaskCreate(ws1850_task, "ws1850_task", 1024 * 10, NULL, configMAX_PRIORITIES - 6, NULL);

    // es8311_task();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
