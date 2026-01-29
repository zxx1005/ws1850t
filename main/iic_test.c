#include <stdio.h>
#include <string.h>
#include "argtable3/argtable3.h"
#include "driver/i2c_master.h"
#include "esp_console.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include"sc7u22.h"
#include "iic_test.h"

static const char *TAG = "cmd_i2ctools";

#define I2C_TOOL_TIMEOUT_VALUE_MS (100)
// #define I2C_TOOL_TIMEOUT_VALUE_MS (50)
static uint32_t i2c_frequency = 100 * 1000;
static i2c_master_bus_handle_t g_i2c_dev_bus  = NULL;
i2c_master_bus_handle_t tool_bus_handle1;

#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_NUM_18)| (1ULL << GPIO_NUM_40))
// #define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_NUM_37) | (1ULL << GPIO_NUM_14))

static i2c_master_dev_handle_t charge_dev_handle;
static i2c_master_dev_handle_t audio_dev_handle;
static i2c_master_dev_handle_t nfc_dev_handle;

i2c_master_dev_handle_t imu_dev_handle;

void gpio_init_test(void) // 引脚初始化
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // io_conf.pin_bit_mask = 1ULL << GPIO_NUM_37;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 1;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    // gpio_set_level(GPIO_NUM_37, 1);
    gpio_set_level(GPIO_NUM_18, 1);
    gpio_set_level(GPIO_NUM_40, 1);
}


i2c_master_bus_handle_t i2c_manager_get_bus(void)
{
    if (g_i2c_dev_bus) {
        return g_i2c_dev_bus;
    }

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_1,
        .sda_io_num = I2C_dev_SDA_IO,
        .scl_io_num = I2C_dev_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        // .flags.enable_internal_pullup = true,
        .flags.enable_internal_pullup = false,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &g_i2c_dev_bus));
    return g_i2c_dev_bus;
}



void iic_init_test(void) // iic总线1初始化
{
    // i2c_master_bus_config_t i2c_bus_config = {
    //     .clk_source = I2C_CLK_SRC_DEFAULT,
    //     .i2c_port = I2C_NUM_1,
    //     .sda_io_num = GPIO_NUM_39,
    //     .scl_io_num = GPIO_NUM_38,
    //     .glitch_ignore_cnt = 7,
    //     // .flags.enable_internal_pullup = true,
    //     .flags.enable_internal_pullup = false,
    // };

    // ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &g_i2c_dev_bus ));

    i2c_manager_get_bus();
}





void iic_detect_test(void) // 扫描iic总线1设备
{
    for (uint8_t addr = 0x00; addr < 0x7F; addr++)
    {
        if (i2c_master_probe(g_i2c_dev_bus , addr, 100) == ESP_OK)
        {
            printf("Found device at 0x%02X\n", addr);
        }
    }
}




void iic_dev_add_bus(void) // 将iic总线1扫描到的设备添加到总线
{

    // 充电芯片
    i2c_device_config_t i2c_charge_dev_conf = {
        .scl_speed_hz = i2c_frequency,
        .device_address = 0X1A,
    };
    // IMU芯片
    i2c_device_config_t i2c_imu_dev_conf = {
        .scl_speed_hz = i2c_frequency,
        .device_address = 0X19,
    };
    // 音频芯片
    i2c_device_config_t i2c_audio_dev_conf = {
        .scl_speed_hz = i2c_frequency,
        .device_address = 0X18,
    };

    if (i2c_master_bus_add_device(g_i2c_dev_bus , &i2c_charge_dev_conf, &charge_dev_handle) != ESP_OK)
    {
        printf("charge device add failed\n");
        return;
    }

    if (i2c_master_bus_add_device(g_i2c_dev_bus , &i2c_imu_dev_conf, &imu_dev_handle) != ESP_OK)
    {
        printf("imu device add failed\n");
        return;
    }
    if (i2c_master_bus_add_device(g_i2c_dev_bus , &i2c_audio_dev_conf, &audio_dev_handle) != ESP_OK)
    {
        printf("audio device add failed\n");
        return;
    }


}

void iic_read_charge_data(void)
{

    int charge_data_addr = 0X00;
    // int charge_data_addr = 0X0B;

    uint8_t charge_data[16] = {0};

    uint8_t len = 16;

    esp_err_t ret = i2c_master_transmit_receive(charge_dev_handle, (uint8_t *)&charge_data_addr, 1, charge_data, len, I2C_TOOL_TIMEOUT_VALUE_MS);
    if (ret == ESP_OK)
    {
        printf("charge data:%d\r\n", charge_data[0]);
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW(TAG, "Bus is busy");
    }
    else
    {
        ESP_LOGW(TAG, "Read failed");
    }
}

void iic_read_imu_data(void)
{

    uint8_t imu_data[16] = {0};
    uint8_t len = 16;
    int imu_data_addr = 0X04;

    esp_err_t ret = i2c_master_transmit_receive(imu_dev_handle, (uint8_t *)&imu_data_addr, 1, imu_data, len, I2C_TOOL_TIMEOUT_VALUE_MS);
    if (ret == ESP_OK)
    {
        printf("IMU data:%d\r\n", imu_data[0]);
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW(TAG, "Bus is busy");
    }
    else
    {
        ESP_LOGW(TAG, "Read failed");
    }
}

void iic_read_audio_data(void)
{

    uint8_t audio_data[16] = {0};
    uint8_t len = 16;
    int audio_data_addr = 0X24;

    esp_err_t ret = i2c_master_transmit_receive(audio_dev_handle, (uint8_t *)&audio_data_addr, 1, audio_data, len, I2C_TOOL_TIMEOUT_VALUE_MS);
    if (ret == ESP_OK)
    {
        printf("Audio data:%d\r\n", audio_data[0]);
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW(TAG, "Bus is busy");
    }
    else
    {
        ESP_LOGW(TAG, "Read failed");
    }
}


void iic_init_test1(void) // iic总线0初始化
{
    i2c_master_bus_config_t i2c_bus_config1 = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_NFC_SDA_IO,
        .scl_io_num = I2C_NFC_SCL_IO,
        .glitch_ignore_cnt = 7,
        // .flags.enable_internal_pullup = true,
        .flags.enable_internal_pullup = false,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config1, &tool_bus_handle1));
}


void iic_detect_test1(void) // 扫描iic总线0设备
{
    printf("NFC test\n");

    if (i2c_master_probe(tool_bus_handle1, 0X28, 100) == ESP_OK)
    {
        printf("NFC device read succ...\n");
    }
    else
    {
        printf("NFC device read fail...\n");
    }
}

void iic_dev_add_bus1(void) // 将iic总线0扫描到的设备添加到总线
{
    i2c_device_config_t i2c_dev_conf = {
        .scl_speed_hz = i2c_frequency,
        .device_address = 0X28,
    };

    if (i2c_master_bus_add_device(tool_bus_handle1, &i2c_dev_conf, &nfc_dev_handle) != ESP_OK)
    {
        printf("NFC device add failed\n");
        return;
    }
}

void iic_read_nfc_data(void)
{

    uint8_t data[16] = {0};
    uint8_t len = 16;
    int data_addr = 0X37;

    esp_err_t ret = i2c_master_transmit_receive(nfc_dev_handle, (uint8_t *)&data_addr, 1, data, len, I2C_TOOL_TIMEOUT_VALUE_MS);
    if (ret == ESP_OK)
    {
        printf("NFC Version:%d\r\n", data[0]);
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        ESP_LOGW(TAG, "Bus is busy");
    }
    else
    {
        ESP_LOGW(TAG, "Read failed");
    }
}


