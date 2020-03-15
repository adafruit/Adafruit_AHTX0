
/*!
 *  @file Adafruit_AHT10.cpp
 *
 *  @mainpage Adafruit AHT10 Humidity and Temperature Sensor library
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for the Adafruit AHT10 Humidity and Temperature Sensor
 * library
 *
 * 	This is a library for the Adafruit AHT10 breakout:
 * 	https://www.adafruit.com/product/4566
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section dependencies Dependencies
 *  This library depends on the Adafruit BusIO library
 *
 *  This library depends on the Adafruit Unified Sensor library
 *
 *  @section author Author
 *
 *  Bryan Siepert for Adafruit Industries
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#include "Arduino.h"
#include <Wire.h>

#include "Adafruit_AHT10.h"

/*!
 *    @brief  Instantiates a new AHT10 class
 */
Adafruit_AHT10::Adafruit_AHT10(void) {}

Adafruit_AHT10::~Adafruit_AHT10(void) {
  if (temp_sensor) {
    delete temp_sensor;
  }
  if (humidity_sensor) {
    delete humidity_sensor;
  }
}

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @param  sensor_id
 *            The unique ID to differentiate the sensors from others
 *    @return True if initialization was successful, otherwise false.
 */
bool Adafruit_AHT10::begin(TwoWire *wire, int32_t sensor_id) {
  delay(20); // 20 ms to power up

  if (i2c_dev) {
    delete i2c_dev; // remove old interface
  }

  i2c_dev = new Adafruit_I2CDevice(AHT10_I2CADDR_DEFAULT, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  uint8_t cmd[3];

  cmd[0] = AHT10_CMD_SOFTRESET;
  if (!i2c_dev->write(cmd, 1)) {
    return false;
  }
  delay(20);

  cmd[0] = AHT10_CMD_CALIBRATE;
  cmd[1] = 0x08;
  cmd[2] = 0x00;
  if (!i2c_dev->write(cmd, 3)) {
    return false;
  }

  while (getStatus() & AHT10_STATUS_BUSY) {
    delay(10);
  }
  if (!(getStatus() & AHT10_STATUS_CALIBRATED)) {
    return false;
  }

  humidity_sensor = new Adafruit_AHT10_Humidity(this);
  temp_sensor = new Adafruit_AHT10_Temp(this);
  return true;
}

/**
 * @brief  Gets the status (first byte) from AHT10
 *
 * @returns 8 bits of status data, or 0xFF if failed
 */
uint8_t Adafruit_AHT10::getStatus(void) {
  uint8_t ret;
  if (!i2c_dev->read(&ret, 1)) {
    return 0xFF;
  }
  return ret;
}

/**************************************************************************/
/*!
    @brief  Gets the humidity sensor and temperature values as sensor events
    @param  humidity Sensor event object that will be populated with humidity
   data
    @param  temp Sensor event object that will be populated with temp data
    @returns true if the event data was read successfully
*/
/**************************************************************************/
bool Adafruit_AHT10::getEvent(sensors_event_t *humidity,
                              sensors_event_t *temp) {
  uint32_t t = millis();

  // read the data and store it!
  uint8_t cmd[3] = {AHT10_CMD_TRIGGER, 0x33, 0};
  if (!i2c_dev->write(cmd, 3)) {
    return false;
  }

  while (getStatus() & AHT10_STATUS_BUSY) {
    delay(10);
  }

  uint8_t data[6];
  if (!i2c_dev->read(data, 6)) {
    return false;
  }
  uint32_t h = data[1];
  h <<= 8;
  h |= data[2];
  h <<= 4;
  h |= data[3] >> 4;
  _humidity = ((float)h * 100) / 0x100000;

  uint32_t tdata = data[3] & 0x0F;
  tdata <<= 8;
  tdata |= data[4];
  tdata <<= 8;
  tdata |= data[5];
  _temperature = ((float)tdata * 200 / 0x100000) - 50;

  // use helpers to fill in the events
  if (temp)
    fillTempEvent(temp, t);
  if (humidity)
    fillHumidityEvent(humidity, t);
  return true;
}

void Adafruit_AHT10::fillTempEvent(sensors_event_t *temp, uint32_t timestamp) {
  memset(temp, 0, sizeof(sensors_event_t));
  temp->version = sizeof(sensors_event_t);
  temp->sensor_id = _sensorid_temp;
  temp->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  temp->timestamp = timestamp;
  temp->temperature = _temperature;
}

void Adafruit_AHT10::fillHumidityEvent(sensors_event_t *humidity,
                                       uint32_t timestamp) {
  memset(humidity, 0, sizeof(sensors_event_t));
  humidity->version = sizeof(sensors_event_t);
  humidity->sensor_id = _sensorid_humidity;
  humidity->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  humidity->timestamp = timestamp;
  humidity->relative_humidity = _humidity;
}

/**
 * @brief Gets the Adafruit_Sensor object for the AHT10's humidity sensor
 *
 * @return Adafruit_Sensor*
 */
Adafruit_Sensor *Adafruit_AHT10::getHumiditySensor(void) {
  return humidity_sensor;
}

/**
 * @brief Gets the Adafruit_Sensor object for the AHT10's humidity sensor
 *
 * @return Adafruit_Sensor*
 */
Adafruit_Sensor *Adafruit_AHT10::getTemperatureSensor(void) {
  return temp_sensor;
}
/**
 * @brief  Gets the sensor_t object describing the AHT10's humidity sensor
 *
 * @param sensor The sensor_t object to be populated
 */
void Adafruit_AHT10_Humidity::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "AHT10_H", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_RELATIVE_HUMIDITY;
  sensor->min_delay = 0;
  sensor->min_value = 0;
  sensor->max_value = 100;
  sensor->resolution = 2;
}
/**
    @brief  Gets the humidity as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
 */
bool Adafruit_AHT10_Humidity::getEvent(sensors_event_t *event) {
  _theAHT10->getEvent(event, NULL);

  return true;
}
/**
 * @brief  Gets the sensor_t object describing the AHT10's tenperature sensor
 *
 * @param sensor The sensor_t object to be populated
 */
void Adafruit_AHT10_Temp::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "AHT10_T", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  sensor->min_delay = 0;
  sensor->min_value = -40;
  sensor->max_value = 85;
  sensor->resolution = 0.3; // depends on calibration data?
}
/*!
    @brief  Gets the temperature as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns true
*/
bool Adafruit_AHT10_Temp::getEvent(sensors_event_t *event) {
  _theAHT10->getEvent(NULL, event);

  return true;
}
