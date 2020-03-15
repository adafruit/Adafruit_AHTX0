/*!
 *  @file Adafruit_AHT10.h
 *
 * 	I2C Driver for the Adafruit AHT10 Humidity and Temperature Sensor
 *library
 *
 * 	This is a library for the Adafruit AHT10 breakout:
 * 	https://www.adafruit.com/products/4566
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	BSD license (see license.txt)
 */

#ifndef _ADAFRUIT_AHT10_H
#define _ADAFRUIT_AHT10_H

#include "Arduino.h"
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define AHT10_I2CADDR_DEFAULT 0x38   ///< AHT10 default i2c address
#define AHT10_CMD_CALIBRATE 0xE1     ///< Calibration command
#define AHT10_CMD_TRIGGER 0xAC       ///< Trigger reading command
#define AHT10_CMD_SOFTRESET 0xBA     ///< Soft reset command
#define AHT10_STATUS_BUSY 0x80       ///< Status bit for busy
#define AHT10_STATUS_CALIBRATED 0x08 ///< Status bit for calibrated

class Adafruit_AHT10;

/**
 * @brief  Adafruit Unified Sensor interface for the humidity sensor component
 * of AHT10
 *
 */
class Adafruit_AHT10_Humidity : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the humidity sensor
    @param parent A pointer to the AHT10 class */
  Adafruit_AHT10_Humidity(Adafruit_AHT10 *parent) { _theAHT10 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x100;
  Adafruit_AHT10 *_theAHT10 = NULL;
};

/**
 * @brief Adafruit Unified Sensor interface for the temperature sensor component
 * of AHT10
 *
 */
class Adafruit_AHT10_Temp : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the temp sensor
      @param parent A pointer to the AHT10 class */
  Adafruit_AHT10_Temp(Adafruit_AHT10 *parent) { _theAHT10 = parent; }

  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);

private:
  int _sensorID = 0x101;
  Adafruit_AHT10 *_theAHT10 = NULL;
};

/*!
 *    @brief  Class that stores state and functions for interacting with
 *            the AHT10 I2C Digital Potentiometer
 */
class Adafruit_AHT10 {
public:
  Adafruit_AHT10();
  ~Adafruit_AHT10();

  bool begin(TwoWire *wire = &Wire, int32_t sensor_id = 0);

  bool getEvent(sensors_event_t *humidity, sensors_event_t *temp);
  uint8_t getStatus(void);
  Adafruit_Sensor *getTemperatureSensor(void);
  Adafruit_Sensor *getHumiditySensor(void);

protected:
  float _temperature, ///< Last reading's temperature (C)
      _humidity;      ///< Last reading's humidity (percent)

  uint16_t _sensorid_humidity; ///< ID number for humidity
  uint16_t _sensorid_temp;     ///< ID number for temperature

  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface

  Adafruit_AHT10_Temp *temp_sensor = NULL; ///< Temp sensor data object
  Adafruit_AHT10_Humidity *humidity_sensor =
      NULL; ///< Humidity sensor data object

private:
  void _fetchTempCalibrationValues(void);
  void _fetchHumidityCalibrationValues(void);
  friend class Adafruit_AHT10_Temp;     ///< Gives access to private members to
                                        ///< Temp data object
  friend class Adafruit_AHT10_Humidity; ///< Gives access to private members to
                                        ///< Humidity data object

  void fillTempEvent(sensors_event_t *temp, uint32_t timestamp);
  void fillHumidityEvent(sensors_event_t *humidity, uint32_t timestamp);
};

#endif
