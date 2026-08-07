#ifndef __I2CDEV_H__
#define __I2CDEV_H__

#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include "freertos/semphr.h"
#include "driver.h"

#define I2C_CMD_TIMEOUT_MS					50

class I2CDev : public Driver {
	private:
		i2c_port_t i2c_num;
		gpio_num_t sda, scl;
		uint32_t frequency;
		bool installed;
		i2c_master_bus_handle_t bus;
		i2c_master_dev_handle_t devices[128];
		SemaphoreHandle_t bus_mutex;
		esp_err_t device_handle(int addr, i2c_master_dev_handle_t *handle);
		esp_err_t ch_sw(int ch, bool on_off);

	public:
		I2CDev(i2c_port_t _i2c_num, gpio_num_t _sda, gpio_num_t _scl, uint32_t _frequency);
		esp_err_t init(void);
		esp_err_t detect(int ch, int addr);
		esp_err_t write(int ch, int addr, uint8_t *data, size_t data_size);
		esp_err_t read(int ch, int addr, uint8_t *pointer, size_t pointer_size, uint8_t *data, size_t data_size);
};

#endif
