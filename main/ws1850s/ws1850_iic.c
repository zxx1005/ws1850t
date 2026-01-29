#include <stdio.h>
#include <string.h>
#include "argtable3/argtable3.h"
#include "driver/i2c_master.h"
#include "esp_console.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "iic_test.h"

#define I2C_NFC_TIMEOUT_VALUE_MS (100)

static i2c_master_bus_handle_t ws_bus_handel;
static i2c_master_dev_handle_t nfc_dev_handle;

static uint32_t i2c_frequency = 100 * 1000;

#if 0
#define IIC_SDA_READ() P03
#define DEVICE_WRITE_ADDR 0x50
#define DEVICE_READ_ADDR 0x51
#define IIC_SDA_IN() REG_INIT(P03F, PnxS_IN | PnxPUS_SPU)  // P10_Input_Mode
#define IIC_SDA_OUT() REG_INIT(P03F, PnxS_OUT | PnxPUS_PD) // P10_PushPull_Mode

void delay_1us(void)
{
  ;
  ;
}

//* 起始位 */
void I2C_Start(void)
{
  IIC_SDA_OUT();
  IIC_SCL_H;
  // while(1){
  IIC_SDA_H;
  //	delay_ms(100);
  // delay_us(10);
  // delay_us(1);
  delay_1us();
  IIC_SDA_L;
  //	delay_ms(100);
  // delay_us(10);
  // delay_us(1);
  //	}
  delay_1us();
  IIC_SCL_L;
  delay_1us();
  // delay_us(1);
}

/* 终止位 */
void I2C_Stop(void)
{
  IIC_SDA_OUT();
  IIC_SCL_L;
  IIC_SDA_L;
  delay_1us();
  // delay_us(1);
  IIC_SCL_H;
  delay_1us();
  // delay_us(1);
  IIC_SDA_H;
  delay_1us();
  // delay_us(1);
}

/*
功能： This function sends a device Ack or NAck signal
输入： When you want to send ACK signal to slave device, you make bAck 0.
When you want to send NACK signal to slave device, you make bAck 1.
返回： NONE
*/
static void I2C_SendAck(u8 bAck)
{
  IIC_SDA_OUT();
  if (bAck)
    IIC_SDA_H;
  else
    IIC_SDA_L;
  delay_1us();
  // delay_us(1);
  IIC_SCL_H;
  delay_1us();
  // delay_us(1);
  IIC_SCL_L;
  delay_1us();
  // delay_us(1);
}

/* 功能：等待从器件 ACK 应答信号
输入：无
输出： 0-有 ACK 信号， 1-无 ACK 信号
*/
static u8 I2C_WaitAck(void)
{
  IIC_SDA_IN();
  IIC_SDA_H;
  delay_1us();
  // delay_us(1);
  IIC_SCL_H;
  delay_1us();
  // delay_us(1);
  if (IIC_SDA_READ())
  {
    IIC_SCL_L;
    delay_1us();
    // delay_us(1);
    return 1;
  }
  IIC_SCL_L;
  delay_1us();
  // delay_us(1);
  return 0;
}

u8 fg_tsm_int = 0; /* 中断触发标志 */

/* 功能：将字节数据 dat 发送出去 (写)
输入： 8 位 dat 数据
返回：无
说明：数据 dat 可以是数据，也可以是地址
*/
static void I2C_WriteByte(u8 dat)
{
  u8 i;
  IIC_SDA_OUT();
  // IIC_SCL_L;
  for (i = 0; i < 8; i++)
  {
    if (dat & 0x80)
    {
      IIC_SDA_H;
    }
    else
    {
      IIC_SDA_L;
    }
    IIC_SCL_H; /* 置时钟线为高，通知被控器开始接收数据位 */
    delay_1us();
    // delay_us(1);
    IIC_SCL_L;
    delay_1us();
    // delay_us(1);
    dat <<= 1;
  }
}

/*-----------------------------------------------------------------------------
功能：用来接收从器件传过来的数据，并判断总线错误（不发应答信号）
输入：无
返回：接收到的数据
------------------------------------------------------------------------------*/
static u8 I2C_ReadByte(void)
{
  u8 i, temp = 0;
  IIC_SDA_IN(); /* 置数据线为输入方式 */
  IIC_SCL_L;    /* 置时钟线为低，准备接收数据位 */
  for (i = 0; i < 8; i++)
  {
    temp <<= 1;
    IIC_SCL_H; /* 置时钟线为高使数据线上数据有效 */
    delay_1us();
    // delay_us(1);
    if (IIC_SDA_READ()) /* 读数据位，数据存放于 temp 中 */
    {
      temp++;
    }
    IIC_SCL_L;
  }
  return temp;
}
/* ----------------- Beautiful part line -------------------------------------*/
/*-----------------------------------------------------------------------------
功能：向 TSM12 写单字节数据或命令
输入： writeData-- 被写数据， WriteAddr-- 待写入数据寄存器地址返回： ErrorStatus
------------------------------------------------------------------------------*/
u8 IIC_Write_Byte(u8 WriteAddr, u8 WriteData)
{
  /* 发送起始位 */
  I2C_Start();

  /* 发送器件地址 */
  I2C_WriteByte(DEVICE_WRITE_ADDR);
  delay_1us();
  // delay_us(1);
  if (I2C_WaitAck())
    return 1;

  /* 发送待写入数据的寄存器地址 */
  I2C_WriteByte(WriteAddr);
  delay_1us();
  // delay_us(1);
  if (I2C_WaitAck())
    return 1;

  /* 发送数据到寄存器中 */
  I2C_WriteByte(WriteData);
  delay_1us();
  // delay_us(1);
  if (I2C_WaitAck())
    return 1;
  /* 产生停止位 */
  // delay();

  I2C_Stop();
  // return SUCCESS;
  return 0;
}
/* ----------------------------------------------------------------------------
功能：向 GT216L 写双字节数据或命令
输入： writeData1 ， 2--被写数据， WriteAddr-- 待写入数据寄存器地址
返回： ErrorStatus
------------------------------------------------------------------------------*/
u8 IIC_Write_2Byte(u8 WriteAddr, u8 WriteData1, u8 WriteData2)
{
  /* 发送起始位 */
  I2C_Start();
  /* 发送器件地址 */
  I2C_WriteByte(DEVICE_WRITE_ADDR);
  if (I2C_WaitAck())
    return 1;
  /* 发送待写入数据的寄存器起始地址 */
  I2C_WriteByte(WriteAddr);
  if (I2C_WaitAck())
    return 1;
  /* 发送数据 1 到寄存器中，即 LSB */
  I2C_WriteByte(WriteData1);
  if (I2C_WaitAck())
    return 1;
  /* 发送数据 2 到寄存器中，即 MSB */
  I2C_WriteByte(WriteData2);
  if (I2C_WaitAck())
    return 1;
  /* 产生停止位 */
  I2C_Stop();
  return 0;
}

/*******************************************************************************
 * 名 称: TSM_ReadOneByte()
 * 功 能: 读单个字节数据* 入口参数 : writeAddr-- 待写入数据寄存器地址
 * 出口参数 : 被读出数据 , ERROR
 * 说 明:
 *******************************************************************************/
u8 IIC_ReadOneByte(u8 writeAddr)
{
  u8 buffer = 0;
  // 第一阶段
  I2C_Start();
  I2C_WriteByte(DEVICE_WRITE_ADDR);
  if (I2C_WaitAck())
    goto L_OUT;
  I2C_WriteByte(writeAddr);
  if (I2C_WaitAck())
    goto L_OUT;
  I2C_Stop();
  // 第二阶段
  I2C_Start();
  I2C_WriteByte(DEVICE_READ_ADDR);
  if (I2C_WaitAck())
    goto L_OUT;
  buffer = I2C_ReadByte();
  I2C_SendAck(1);
  I2C_Stop();
  return buffer;
L_OUT:
  I2C_Stop();
  return 1;
}

/*-----------------------------------------------------------------------------
功能：连续读多个字节数据
输入： writeAddr -- 待写入数据寄存器地址
*butter --读缓冲区的地址。
length -- 到的数据长度
返回： ErrorStatus
------------------------------------------------------------------------------*/
u8 IIC_ReadMutiBytes(u8 writeAddr, u8 *buffer, u8 length)
{
  u8 i;
  /* Power ON */

  // 第一阶段
  /* 发送起始位 */
  I2C_Start();
  /* 发送器件地址 */
  I2C_WriteByte(DEVICE_WRITE_ADDR);
  if (I2C_WaitAck())
    goto L_OUT;
  /* 发送被操作寄存器首地址 */
  I2C_WriteByte(writeAddr);
  if (I2C_WaitAck())
    goto L_OUT;
  /* 产生停止位 */
  I2C_Stop();
  // 第二阶段
  /* 发送起始位 */
  I2C_Start();
  /* 发送 GT216L 器件地址 */
  I2C_WriteByte(DEVICE_READ_ADDR);
  if (I2C_WaitAck())
    goto L_OUT;
  /* 产生一个应该应答信号 */
  for (i = 0; i < length; i++)
  {
    buffer[i] = I2C_ReadByte();
    if (i == (length - 1))
      I2C_SendAck(1); // NAck = 1
    else
      I2C_SendAck(0); // Ack = 0
  } /* 产生停止位 */
  I2C_Stop();
  return 0;
L_OUT:
  /* 产生停止位 */
  I2C_Stop();
  return 1;
}



void SetBitMask(unsigned char reg, unsigned char mask)
{
  char tmp = 0x0;
  tmp = IIC_ReadOneByte(reg);
  IIC_Write_Byte(reg, tmp | mask); // set bit mask
}

/////////////////////////////////////////////////////////////////////
//?    ?:?????
//????:reg[IN]:?????
//          mask[IN]:???
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg, unsigned char mask)
{
  char tmp = 0x0;
  tmp = IIC_ReadOneByte(reg);
  IIC_Write_Byte(reg, tmp & ~mask); // clear bit mask //将数据的位清零
}
#endif

void ws_iic_detect(void) // 扫描iic总线0设备
{

  for (uint8_t addr = 0x00; addr < 0x7F; addr++)
  {
    if (i2c_master_probe(ws_bus_handel, addr, 100) == ESP_OK)
    {
      printf("Found device at 0x%02X\n", addr);
    }
  }

  if (i2c_master_probe(ws_bus_handel, 0X28, 100) == ESP_OK)
  {
    printf("NFC device read succ...\n");
  }
  else
  {
    printf("NFC device read fail...\n");
  }
}

void WS1850_iic_dev_init(void)
{
  // NFC芯片
  i2c_device_config_t i2c_nfc_dev_conf = {
      .scl_speed_hz = i2c_frequency,
      .device_address = 0X28,
  };

  if (i2c_master_bus_add_device(ws_bus_handel, &i2c_nfc_dev_conf, &nfc_dev_handle) != ESP_OK)
  {
    printf("NFC device add failed\n");
    return;
  }
}

void ws_iic_bus_init(void) // iic总线0初始化
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

  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config1, &ws_bus_handel));
}

void ws_iic_init(void)
{
  ws_iic_bus_init();
  WS1850_iic_dev_init();

  ws_iic_detect();
}


#if 0
esp_err_t ws_write_reg(uint8_t reg, uint8_t val)
{
  uint8_t buf[2] = {reg, val};

  return i2c_master_transmit(
      nfc_dev_handle,
      buf,
      sizeof(buf),
      I2C_NFC_TIMEOUT_VALUE_MS);
}

uint8_t ws_read_reg(uint8_t reg)
{
  uint8_t val = 0;
  esp_err_t ret;

  ret = i2c_master_transmit_receive(
      nfc_dev_handle,
      &reg,
      1,
      &val,
      1,
      I2C_NFC_TIMEOUT_VALUE_MS);

  if (ret != ESP_OK)
  {
    return 0;
  }

  return val;
}

#else

esp_err_t ws_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };

    esp_err_t err=i2c_master_transmit(
        nfc_dev_handle,
        buf,
        2,
        I2C_NFC_TIMEOUT_VALUE_MS);

    if (err !=ESP_OK)
    {
      ESP_LOGE("ws write","sent error, reg:0X%02X",reg);
    }
    return err;
}

uint8_t ws_read_reg(uint8_t reg)
{
    uint8_t val = 0;

    esp_err_t ret = i2c_master_transmit_receive(
        nfc_dev_handle,
        &reg,
        1,
        &val,
        1,
        I2C_NFC_TIMEOUT_VALUE_MS);

    if (ret != ESP_OK)
    {
       ESP_LOGE("ws read","sent error, reg:0X%02X",reg);
       return 0xFF;
    }

    return val;
}

#endif


/**
 ****************************************************************
 * @brief SetBitMask() 
 *
 * 将寄存器的某些bit位值1
 *
 * @param: reg 寄存器地址
 * @param: mask 需要置位的bit位
 ****************************************************************
 */
void SetBitMask(unsigned char reg, unsigned char mask)
{
  char tmp = 0x0;
  tmp = ws_read_reg(reg);
  esp_err_t err = ws_write_reg(reg, tmp | mask); // set bit mask
  // printf("SetBitMask ret: %d", err);
}

/**
 ****************************************************************
 * @brief ClearBitMask() 
 *
 * 将寄存器的某些bit位清0
 *
 * @param: reg 寄存器地址
 * @param: mask 需要清0的bit位
 ****************************************************************
 */
void ClearBitMask(unsigned char reg, unsigned char mask)
{
  uint8_t tmp = 0x0;
  tmp = ws_read_reg(reg);
  esp_err_t err = ws_write_reg(reg, tmp & ~mask); // clear bit mask //将数据的位清零
  // printf("ClearBitMask ret: %d", err);
}

void WriteRawRC(unsigned char Address, unsigned char value)
{
  ws_write_reg(Address, value);
}

unsigned char ReadRawRC(unsigned char Address)
{
  return ws_read_reg(Address);
}