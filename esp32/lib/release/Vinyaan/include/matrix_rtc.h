#ifndef __MATRIX_RTC_H__
#define __MATRIX_RTC_H__

#include "driver.h"
#include "device.h"
#include "i2c-dev.h"

#define SCROLLING_STRING_LEN			31
#define LED16x8_MAX_CHARS				2

#define MATRIX_RTC_BUFFER_SIZE				16
#define DATETIME_STR_LEN					31
#define DATE_STR_LEN						15
#define TIME_STR_LEN						15

class Matrix_RTC : public Device {
	private:
		// Matrix state
		enum {
			m_detect, m_clrscr, m_cmd_init, m_show, m_scrolling_init, m_scrolling, m_idle, m_error, m_wait
		} matrix_state;
		TickType_t matrix_tickcnt;
		uint8_t buffer[16], temp_buffer[16];
		uint8_t scrolling_buffer[16];
		char scrolling_string[SCROLLING_STRING_LEN + 1], scrolling_temp_string[SCROLLING_STRING_LEN + 1];
		uint8_t curr_char_index, start_strip_index, display_width, font_width, curr_heading_width, cmd_index;
		uint8_t matrix_flag, set_flag, clr_flag;

		// RTC state
		enum {
		   r_detect, r_read, r_wait, r_write, r_cal, r_cal_coarse, r_error
		} rtc_state;
		TickType_t rtc_tickcnt, polling_tickcnt;
		uint8_t rtc_flag;
		uint8_t read_buffer[MATRIX_RTC_BUFFER_SIZE], write_buffer[MATRIX_RTC_BUFFER_SIZE];
		uint8_t cal_value, cal_coarse_value;
		char datetime_str[DATETIME_STR_LEN + 1], date_str[DATE_STR_LEN + 1], time_str[TIME_STR_LEN + 1];

		void matrix_process(Driver *drv);
		void rtc_process(Driver *drv);

	public:
		// constructor
		Matrix_RTC(int bus_ch, int dev_addr);
		// override
		void init(void);
		void process(Driver *drv);
		int prop_count(void);
		bool prop_name(int index, char *name);
		bool prop_unit(int index, char *unit);
		bool prop_attr(int index, char *attr);
		bool prop_read(int index, char *value);
		bool prop_write(int index, char *value);
		
		// Matrix methods
		int busy(void);
		int idle(void);
		void wait_idle(void);
		void show(uint8_t *buf);
		void scroll(char *buf, bool scroll_flag);
		void scroll(int val, bool scroll_flag);
		void scroll(double val, bool scroll_flag);
		void scroll(double val, bool scroll_flag, int precision);

		// RTC methods
		char *get_datetime(void);
		char *get_datetime_with_second(void);
		char *get_date(void);
		char *get_time(void);
		char *get_time_with_second(void);
		int get(int index);
		void write(char *str);
		bool write_flag(void);
		void cal(int val);
		void cal_coarse(int val);
};

#endif
