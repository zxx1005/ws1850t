
#include <stdio.h>
#include <string.h>
#include "argtable3/argtable3.h"
#include "driver/i2c_master.h"
#include "esp_console.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include"sc7u22.h"

#include "iic_test.h"

#define SL_SC7U22_RAWDATA_HPF_ENABLE 0x00
#define SL_SC7U22_SDO_PullUP_ENABLE 0x01
#define SC7U22_ADDR 0x30
#define SC7U22_WHO_AM_I 0x01


#define delay1ms(x) vTaskDelay(pdMS_TO_TICKS(x))

static i2c_master_dev_handle_t imu_dev_handle;
// extern i2c_master_dev_handle_t imu_dev_handle;


#define I2C_TIMEOUT_VALUE_MS (100)

uint8_t Gyro_ID = 0;

static uint32_t i2c_frequency = 100 * 1000;

#if 1

esp_err_t sc7u22_write_reg(uint8_t reg, uint8_t val)
{
	uint8_t buf[2] = {reg, val};

	return i2c_master_transmit(
		imu_dev_handle,
		buf,
		sizeof(buf),
		I2C_TIMEOUT_VALUE_MS
	);
}

uint8_t sc7u22_read_reg(uint8_t reg)
{
	uint8_t val = 0;
	esp_err_t ret;

	ret = i2c_master_transmit_receive(
		imu_dev_handle,
		&reg,
		1,
		&val,
		1,
		I2C_TIMEOUT_VALUE_MS);

	if (ret != ESP_OK)
	{
		return 0;
	}

	return val;
}

esp_err_t sc7u22_read_buf(uint8_t reg, uint8_t *buf, uint8_t len)
{
	if (buf == NULL || len == 0) {
		return ESP_ERR_INVALID_ARG;
	}

	return i2c_master_transmit_receive(
		imu_dev_handle,
		&reg,
		1,
		buf,
		len,
		I2C_TIMEOUT_VALUE_MS
	);
}


#endif

uint8_t SC7U22_Check(void)
{
	sc7u22_write_reg( 0x7f, 0x00);
	Gyro_ID = sc7u22_read_reg( SC7U22_WHO_AM_I);
	return Gyro_ID;
}

uint8_t reg_value = 0;

uint8_t SC7U22_Check_ID(void)
{

#if 0
//有如下寄存器可以作为贵司的标志位:0x6C的最高位。
sc7u22_write_reg(0x7f, 0x00);//切换Page
delay1ms(20);
reg_value=sc7u22_read_reg(0x01);
if(reg_value!=0x6A)//SC7U22
return 0;//产品异常
reg_value=sc7u22_read_reg(0x02);//混淆读取，需要判断
if(reg_value!=0x13)//SC7U22
return 0;//产品异常

sc7u22_write_reg(0x7f, 0x00);//切换Page
delay1ms(20);
sc7u22_write_reg(0x40,0x8A);//切换到高性能模式
sc7u22_write_reg(0x42,0xEA);//切换到高性能模式
delay1ms(10);
#endif

	sc7u22_write_reg( 0x7F, 0x89); // 切换Page
	delay1ms(20);
	reg_value = sc7u22_read_reg( 0x6a);
	reg_value = sc7u22_read_reg( 0x6b);
	reg_value = sc7u22_read_reg( 0x6c);
	reg_value = reg_value & 0x80;
	delay1ms(20);
	sc7u22_write_reg( 0x7f, 0x00); // 切换Page
	delay1ms(20);

#ifdef CHECK_GYRO_ID
	if (reg_value == 0x80)
		return 1; // 产品正确
	else
		return 0; // 产品异常
#else
	return 1;
#endif

	// 理论固定值是0x05，贵司出货改为0x85
}

uint8_t SC7U22_POWER_DOWN(void)
{
	uint8_t SL_Read_Reg = 0xff;

	sc7u22_write_reg( 0x7f, 0x00);
	delay1ms(10);
	sc7u22_write_reg( 0x7D, 0x00);
	delay1ms(10);
	SL_Read_Reg = sc7u22_read_reg( 0x7D);
	if (SL_Read_Reg == 0x00)
		return 1;
	else
		return 0;
}

uint8_t SC7U22_SOFT_RESET(void)
{
	uint8_t SL_Read_Reg = 0xff;
	sc7u22_write_reg( 0x7f, 0x00);
	delay1ms(1);
	sc7u22_write_reg( 0x04, 0x10);
	sc7u22_write_reg( 0x4A, 0xA5);
	sc7u22_write_reg( 0x4A, 0xA5);
	delay1ms(200);
	SL_Read_Reg = sc7u22_read_reg( 0x04);
	if (SL_Read_Reg == 0x50)
		return 1;
	else
		return 0;
}

uint8_t SC7U22_Deinit(void)
{
	SC7U22_SOFT_RESET();
	sc7u22_write_reg( 0x40, 0x00);
	sc7u22_write_reg( 0x42, 0x00);
	SC7U22_POWER_DOWN();
	return 1;
}

void SC722_iic_init(void)
{
	// IMU芯片
	i2c_device_config_t i2c_imu_dev_conf = {
		.scl_speed_hz = i2c_frequency,
		.device_address = 0X19,
	};
	
 ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            i2c_manager_get_bus(),
            &i2c_imu_dev_conf,
            &imu_dev_handle
        )
    );
}

uint8_t SC7U22_Init(void)
{

	SC722_iic_init();

	uint8_t retry = 5;
	while (retry--)
	{
		// SC7U22_Check();
		SC7U22_POWER_DOWN();
		SC7U22_SOFT_RESET();
		sc7u22_write_reg(0x7f, 0x01);
		sc7u22_write_reg( 0x7f, 0x00);
		delay1ms(10); // 10ms
		sc7u22_write_reg( 0x7D, 0x0E);
		delay1ms(10); // 10ms
		// sc7u22_write_reg(0x40,0x8B);//800hz odr 平均次数=1.6kHz/ODR
		sc7u22_write_reg( 0x40, 0x8A); // 400hz odr 平均次数=1.6kHz/ODR
		sc7u22_write_reg( 0x41, 0x02); //+-8G
		// sc7u22_write_reg(0x42,0x8B);//800hz odr 平均次数=3.2kHz/ODR，噪声优化
		sc7u22_write_reg( 0x42, 0x8A); // 400hz odr 平均次数=3.2kHz/ODR
		sc7u22_write_reg( 0x43, 0x00); // 2000dps
		sc7u22_write_reg( 0x04, 0x50);
		sc7u22_write_reg( 0x30, 0x02); // 禁止SDO上拉电阻

#if SL_SC7U22_RAWDATA_HPF_ENABLE == 0x01
		uint8_t reg_value = 0;
		sc7u22_write_reg( 0x7f, 0x83);
		delay1ms(1);
		reg_value = sc7u22_read_reg( 0x26);
		reg_value = reg_value | 0xA0;
		sc7u22_write_reg( 0x26, reg_value);
#endif
		if (SC7U22_Check_ID())
		{
			return 1;
		}
	}
	SC7U22_Deinit();
	return 0;
}

void SL_SC7U22_RawData_Read(int16_t *sensor_raw)
{
	uint8_t raw_data[12];
#if 0
	uint8_t  drdy_satus=0x00;
	unsigned short drdy_cnt=0;	
	while((drdy_satus&0x03)!=0x03)//acc+gyro
	{
		drdy_satus=0x00;
		delay10us(1);
		drdy_satus=sc7u22_read_reg(0x0B);
		drdy_cnt++;
		if(drdy_cnt>30000) break;
	}
#endif
	sc7u22_read_buf( 0x0C, &raw_data[0], 12);
#ifdef MAIN_SENSOR
	sensor_raw[2] = -(signed short)((((uint8_t)raw_data[0]) * 256) + ((uint8_t)raw_data[1]));  // ACCX-16位
	sensor_raw[0] = -(signed short)((((uint8_t)raw_data[2]) * 256) + ((uint8_t)raw_data[3]));  // ACCY-16位
	sensor_raw[1] = -(signed short)((((uint8_t)raw_data[4]) * 256) + ((uint8_t)raw_data[5]));  // ACCZ-16位
	sensor_raw[5] = (signed short)((((uint8_t)raw_data[6]) * 256) + ((uint8_t)raw_data[7]));   // GYRX-16位
	sensor_raw[3] = (signed short)((((uint8_t)raw_data[8]) * 256) + ((uint8_t)raw_data[9]));   // GYRY-16位
	sensor_raw[4] = (signed short)((((uint8_t)raw_data[10]) * 256) + ((uint8_t)raw_data[11])); // GYRZ-16位
#else
	sensor_raw[2] = (signed short)((((uint8_t)raw_data[0]) * 256) + ((uint8_t)raw_data[1]));	// ACCX-16位
	sensor_raw[1] = (signed short)((((uint8_t)raw_data[2]) * 256) + ((uint8_t)raw_data[3]));	// ACCY-16位
	sensor_raw[0] = (signed short)((((uint8_t)raw_data[4]) * 256) + ((uint8_t)raw_data[5]));	// ACCZ-16位
	sensor_raw[5] = -(signed short)((((uint8_t)raw_data[6]) * 256) + ((uint8_t)raw_data[7]));	// GYRX-16位
	sensor_raw[4] = -(signed short)((((uint8_t)raw_data[8]) * 256) + ((uint8_t)raw_data[9]));	// GYRY-16位
	sensor_raw[3] = -(signed short)((((uint8_t)raw_data[10]) * 256) + ((uint8_t)raw_data[11])); // GYRZ-16位
#endif
}
