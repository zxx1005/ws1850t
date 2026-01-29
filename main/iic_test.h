
#ifndef _IIC_TEST_H_
#define _IIC_TEST_H_

#include "driver/i2c_master.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_dev_SDA_IO GPIO_NUM_39
#define I2C_dev_SCL_IO GPIO_NUM_38

#define I2C_NFC_SDA_IO GPIO_NUM_47
#define I2C_NFC_SCL_IO GPIO_NUM_21

void gpio_init_test(void);
void iic_init_test(void);
void iic_detect_test(void);
void iic_dev_add_bus(void);
void iic_read_charge_data(void);
void iic_read_imu_data(void);
void iic_read_audio_data(void);


i2c_master_bus_handle_t i2c_manager_get_bus(void);



void iic_init_test1(void);
void iic_detect_test1(void);
void iic_dev_add_bus1(void);
void iic_read_nfc_data(void);


// esp_err_t sc7u22_write_reg(uint8_t reg, uint8_t val);
// uint8_t sc7u22_read_reg(uint8_t reg);
// uint8_t sc7u22_read_buf(uint8_t reg, uint8_t *buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* _IIC_TEST_H_ */