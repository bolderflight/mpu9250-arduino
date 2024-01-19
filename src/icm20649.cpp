/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2024 Bolder Flight Systems Inc
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the “Software”), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include "Icm20649.h"  // NOLINT
#if defined(ARDUINO)
#include <Arduino.h>
#include "Wire.h"
#include "SPI.h"
#else
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include "core/core.h"
#endif

namespace bfs {

void Icm20649::Config(TwoWire *i2c, const I2cAddr addr) {
  imu_.Config(i2c, static_cast<uint8_t>(addr));
  iface_ = I2C;
}
void Icm20649::Config(SPIClass *spi, const uint8_t cs) {
  imu_.Config(spi, cs);
  iface_ = SPI;
}
bool Icm20649::Begin() {
  delay(100);
  imu_.Begin();
  if (iface_ == SPI) {
    /* I2C IF DIS */
    if (!WriteRegister(USER_CTRL_, I2C_IF_DIS_)) {
      Serial.println("I2C IF DIS FALSE");
    }
  }
  /* Set to Bank 0 */
  if(!SetBank(0)) {
    return false;
  }
  delay(100);
  /* Select clock source */
  if (!WriteRegister(PWR_MGMT_1_, CLKSEL_AUTO_)) {
    return false;
  }
  /* Check the WHO AM I byte */
  if (!ReadRegisters(WHO_AM_I_, sizeof(who_am_i_), &who_am_i_)) {
    return false;
  }
  if ((who_am_i_ != WHOAMI_ICM20649_)) {
    return false;
  }
  /* align odr enable */
  SetBank(2);
  if (!WriteRegister(ODR_ALIGN_EN_, ALIGN_ODR_)) {
    return false;
  }
  /* Set the accel range to 30G by default */
  if (!ConfigAccelRange(ACCEL_RANGE_30G)) {
    return false;
  }
  /* Set the gyro range to 4000DPS by default*/
  if (!ConfigGyroRange(GYRO_RANGE_4000DPS)) {
    return false;
  }
  /* Set the AccelDLPF to 111HZ by default */
  if (!ConfigAccelDlpfBandwidth(ACCEL_DLPF_BANDWIDTH_111HZ)) {
    return false;
  }
  /* Set the Gyro DLPF to 184HZ by default */
  if (!ConfigGyroDlpfBandwidth(GYRO_DLPF_BANDWIDTH_119HZ)) {
    return false;
  }
  /* Set the Temp DLPF */


  /* Set the SRD to 0 by default */
  if (!ConfigSrd(0)) {
    return false;
  }
  return true;
}
bool Icm20649::EnableDrdyInt() {
    /* Set to Bank 0 */
  if(!SetBank(0)) {
    return false;
  }
  if (!WriteRegister(INT_ENABLE_1_, INT_RAW_RDY_EN_)) {
    return false;
  }
  return true;
}
bool Icm20649::DisableDrdyInt() {
    /* Set to Bank 0 */
  if(!SetBank(0)) {
    return false;
  }
  if (!WriteRegister(INT_ENABLE_1_, INT_DISABLE_)) {
    return false;
  }
  return true;
}
bool Icm20649::ConfigAccelRange(const AccelRange range) {
  if(!SetBank(2)) {
    return false;
  }
  /* Check input is valid and set requested range and scale */
  switch (range) {
    case ACCEL_RANGE_4G: {
      requested_accel_range_ = range;
      requested_accel_scale_ = 4.0f / 32767.5f;
      break;
    }
    case ACCEL_RANGE_8G: {
      requested_accel_range_ = range;
      requested_accel_scale_ = 8.0f / 32767.5f;
      break;
    }
    case ACCEL_RANGE_16G: {
      requested_accel_range_ = range;
      requested_accel_scale_ = 16.0f / 32767.5f;
      break;
    }
    case ACCEL_RANGE_30G: {
      requested_accel_range_ = range;
      requested_accel_scale_ = 32.0f / 32767.5f;
      break;
    }
    default: {
      return false;
    }
  }
  /* Try setting the requested range */
  ReadRegisters(ACCEL_CONFIG_, 1, data_buf_);
  data_buf_[0] &= ~(0x06);
  data_buf_[0] |= (requested_accel_range_ << 1);
  if (!WriteRegister(ACCEL_CONFIG_, data_buf_[0] )) {
    return false;
  }
  /* Update stored range and scale */
  accel_range_ = requested_accel_range_;
  accel_scale_ = requested_accel_scale_;
  return true;
}
bool Icm20649::ConfigGyroRange(const GyroRange range) {
  if(!SetBank(2)) {
    return false;
  }
  /* Check input is valid and set requested range and scale */
  switch (range) {
    case GYRO_RANGE_500DPS: {
      requested_gyro_range_ = range;
      requested_gyro_scale_ = 500.0f / 32767.5f;
      break;
    }
    case GYRO_RANGE_1000DPS: {
      requested_gyro_range_ = range;
      requested_gyro_scale_ = 1000.0f / 32767.5f;
      break;
    }
    case GYRO_RANGE_2000DPS: {
      requested_gyro_range_ = range;
      requested_gyro_scale_ = 2000.0f / 32767.5f;
      break;
    }
    case GYRO_RANGE_4000DPS: {
      requested_gyro_range_ = range;
      requested_gyro_scale_ = 4000.0f / 32767.5f;
      break;
    }
    default: {
      return false;
    }
  }
  /* Try setting the requested range */
  ReadRegisters(GYRO_CONFIG_1_, 1, data_buf_);
  data_buf_[0] &= ~(0x06);
  data_buf_[0] |= (requested_accel_range_ << 1);
  if (!WriteRegister(GYRO_CONFIG_1_, data_buf_[0])) {
    return false;
  }
  /* Update stored range and scale */
  gyro_range_ = requested_gyro_range_;
  gyro_scale_ = requested_gyro_scale_;
  return true;
}
bool Icm20649::ConfigSrd(const uint8_t srd) {
  if(!SetBank(2)) {
    return false;
  }
  if (!WriteRegister(ACCEL_SMPLRT_DIV_2_, srd)) {
    return false;
  }
  if (!WriteRegister(GYRO_SMPLRT_DIV_, srd)) {
    return false;
  }
  srd_ = srd;
  return true;
}
bool Icm20649::ConfigAccelDlpfBandwidth(const AccelDlpfBandwidth dlpf) {
  if(!SetBank(2)) {
    return false;
  }
  /* Check input is valid and set requested dlpf */
  switch (dlpf) {
    case ACCEL_DLPF_BANDWIDTH_246HZ: {
      accel_requested_dlpf_ = dlpf;
      break;
    }
    case ACCEL_DLPF_BANDWIDTH_111HZ: {
      accel_requested_dlpf_ = dlpf;
      break;
    }
    case ACCEL_DLPF_BANDWIDTH_50HZ: {
      accel_requested_dlpf_ = dlpf;
      break;
    }
    case ACCEL_DLPF_BANDWIDTH_23HZ: {
      accel_requested_dlpf_ = dlpf;
      break;
    }
    case ACCEL_DLPF_BANDWIDTH_11HZ: {
      accel_requested_dlpf_ = dlpf;
      break;
    }
    case ACCEL_DLPF_BANDWIDTH_5HZ: {
      accel_requested_dlpf_ = dlpf;
      break;
    }
    case ACCEL_DLPF_BANDWIDTH_473HZ: {
      accel_requested_dlpf_ = dlpf;
      break;
    }
    default: {
      return false;
    }
  }
  /* Try setting the dlpf */
  ReadRegisters(ACCEL_CONFIG_, 1, data_buf_);
  data_buf_[0] |= 0x01;
  data_buf_[0] &= 0xC7;
  data_buf_[0] |= (accel_requested_dlpf_ << 3);
  if (!WriteRegister(ACCEL_CONFIG_, data_buf_[0])) {
    return false;
  }
  /* Update stored dlpf */
  accel_dlpf_bandwidth_ = accel_requested_dlpf_;
  return true;
}
bool Icm20649::ConfigGyroDlpfBandwidth(const GyroDlpfBandwidth dlpf) {
  if(!SetBank(2)) {
    return false;
  }
  /* Check input is valid and set requested dlpf */
  switch (dlpf) {
    case GYRO_DLPF_BANDWIDTH_196HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    case GYRO_DLPF_BANDWIDTH_151HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    case GYRO_DLPF_BANDWIDTH_119HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    case GYRO_DLPF_BANDWIDTH_51HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    case GYRO_DLPF_BANDWIDTH_23HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    case GYRO_DLPF_BANDWIDTH_11HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    case GYRO_DLPF_BANDWIDTH_5HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    case GYRO_DLPF_BANDWIDTH_361HZ: {
      gyro_requested_dlpf_ = dlpf;
      break;
    }
    default: {
      return false;
    }
  }
  /* second change dlpf */
  ReadRegisters(GYRO_CONFIG_1_, sizeof(data_buf_), data_buf_);
  data_buf_[0] |= 0x01;
  data_buf_[0] &= 0xC7;
  data_buf_[0] |= (gyro_requested_dlpf_<< 3);
  if (!WriteRegister(GYRO_CONFIG_1_, data_buf_[0])) {
    return false;
  }
  /* Update stored dlpf */
  gyro_dlpf_bandwidth_ = gyro_requested_dlpf_;
  return true;
}
void Icm20649::Reset() {
  /* Reset the IMU */
  SetBank(0);
  WriteRegister(PWR_MGMT_1_, H_RESET_);
  /* Wait for IMU to come back up */
  delay(1);
}
bool Icm20649::Read() {
  SetBank(0);
  /* Reset the new data flags */
  new_imu_data_ = false;
  /* Read the data registers */
  if (!ReadRegisters(INT_STATUS_1_, 1, data_buf_)) {
    return false;
  }
  /* Check if data is ready */
  new_imu_data_ = (data_buf_[0] & RAW_DATA_RDY_INT_);
  if (!new_imu_data_) {
    return false;
  }
  ReadRegisters(ACCEL_XOUT_H, 14, data_buf_);
  /* Unpack the buffer */
  accel_cnts_[0] = static_cast<int16_t>(data_buf_[0])  << 8 | data_buf_[1];
  accel_cnts_[1] = static_cast<int16_t>(data_buf_[2])  << 8 | data_buf_[3];
  accel_cnts_[2] = static_cast<int16_t>(data_buf_[4])  << 8 | data_buf_[5];
  gyro_cnts_[0] =  static_cast<int16_t>(data_buf_[6])  << 8 | data_buf_[7];
  gyro_cnts_[1] =  static_cast<int16_t>(data_buf_[8])  << 8 | data_buf_[9];
  gyro_cnts_[2] =  static_cast<int16_t>(data_buf_[10]) << 8 | data_buf_[11];
  temp_cnts_ =     static_cast<int16_t>(data_buf_[12])  << 8 | data_buf_[13];
  /* Convert to float values and rotate the accel / gyro axis */
  accel_[0] = static_cast<float>(accel_cnts_[0]) * accel_scale_ * G_MPS2_;
  accel_[1] = static_cast<float>(accel_cnts_[1]) * accel_scale_ * -1.0f * G_MPS2_;
  accel_[2] = static_cast<float>(accel_cnts_[2]) * accel_scale_ * -1.0f * G_MPS2_;
  temp_ = (static_cast<float>(temp_cnts_) - 21.0f) / TEMP_SCALE_ + 21.0f;
  gyro_[0] = static_cast<float>(gyro_cnts_[0]) * gyro_scale_ * DEG2RAD_;
  gyro_[1] = static_cast<float>(gyro_cnts_[1]) * gyro_scale_ * -1.0f * DEG2RAD_;
  gyro_[2] = static_cast<float>(gyro_cnts_[2]) * gyro_scale_ * -1.0f * DEG2RAD_;
  return true;
}
bool Icm20649::SetBank(uint8_t bank) {
	if(bank != current_bank_) {
    if (WriteRegister(REG_BANK_SEL_, bank << 4)) {
      current_bank_ = bank;
      return true;
    }
    return false;
  }
  return true;
}
bool Icm20649::WriteRegister(const uint8_t reg, const uint8_t data) {
  return imu_.WriteRegister(reg, data, SPI_CLOCK_);
}
bool Icm20649::ReadRegisters(const uint8_t reg, const uint8_t count,
                             uint8_t * const data) {
  return imu_.ReadRegisters(reg, count, SPI_CLOCK_, data);
}

}  // namespace bfs
