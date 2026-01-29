#ifndef __SC7U22_H__
#define __SC7U22_H__


#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif


uint8_t SC7U22_Check(void);
uint8_t SC7U22_POWER_DOWN(void);
uint8_t SC7U22_Init(void);
void SL_SC7U22_RawData_Read(int16_t * sensor_raw);
uint8_t SC7U22_Check_ID(void);

void sc7u22_iic_add_bus(i2c_device_config_t * dev,i2c_master_dev_handle_t * handler);

void SC722_iic_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __SC7U22_H__ */