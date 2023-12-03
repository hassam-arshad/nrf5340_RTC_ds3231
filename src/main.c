/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
/* STEP 3 - Include the header file of the I2C API */
#include <zephyr/drivers/i2c.h>
/* STEP 4.1 - Include the header file of printk() */
#include <zephyr/sys/printk.h>


/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* STEP 8 - Define the I2C slave device address and the addresses of relevant registers */
#define SECOND_REG            0x00
#define MINUTE_REG            0x01
#define	HOUR_REG			  0x02
#define	DAY_OF_WEEK_REG		  0x03
#define DATE_REG			  0x04
#define	MONTH_REG			  0x05
#define YEAR_REG			  0x06

/* STEP 6 - Get the node identifier of the sensor */
#define I2C_NODE DT_NODELABEL(mysensor)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

uint8_t decimal_to_bcd(uint8_t decimal) {
    return ((decimal / 10) << 4) | (decimal % 10);
}

uint8_t bcd_to_decimal(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
/*
setting Date Time setting formate is as follows
Seconds 00–59
Minutes 00–59
Hours   00-23
Day 1–7
Date 01–31
Month 01–12
Year 00–99
*/
void set_date_time(uint8_t * dateTime)
{
	int ret;
    // Convert decimal values to BCD before writing to the DS3231
    uint8_t bcdDateTime[7];
    for (int i = 0; i < 7; i++) {
        bcdDateTime[i] = decimal_to_bcd(dateTime[i]);
    }

    // Set the date and time
    uint8_t regAddresses[7] = {SECOND_REG, MINUTE_REG, HOUR_REG, DAY_OF_WEEK_REG, DATE_REG, MONTH_REG, YEAR_REG};
    for (int i = 0; i < 7; i++) {
        uint8_t data[2] = {regAddresses[i], bcdDateTime[i]};	// first byte of data is reg address and second is data
        ret = i2c_write_dt(&dev_i2c, data, sizeof(data));
        if (ret != 0) {
            printk("Failed to write to I2C device address %x at Reg. %x \n", dev_i2c.addr, data[0]);
            return;
        }
    }
}

void get_date_time(uint8_t *dateTime) {
    
	int ret;

    uint8_t regAddresses[7] = {SECOND_REG, MINUTE_REG, HOUR_REG, DAY_OF_WEEK_REG, DATE_REG, MONTH_REG, YEAR_REG};
    for (int i = 0; i < 7; i++) {
        ret = i2c_write_read_dt(&dev_i2c, &regAddresses[i], 1, &dateTime[i], 1);
        if (ret != 0) {
            printk("Failed to write/read I2C device address %x at Reg. %x \r\n", dev_i2c.addr, regAddresses[i]);
            return;
        }
    }

    // Convert BCD values to decimal after reading from the DS3231
    for (int i = 0; i < 7; i++) {
        dateTime[i] = bcd_to_decimal(dateTime[i]);
    }
}

void main(void)
{

	int ret;

/* STEP 7 - Retrieve the API-specific device structure and make sure that the device is ready to use  */
	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
		return;
	}

/* set date and time */
/*
setting Date Time setting formate is as follows
Seconds 00–59
Minutes 00–59
Hours   00-23
Day 1–7
Date 01–31
Month 01–12
Year 00–99
*/
	uint8_t setDateTime[7] = {0};
/*Testing each reg rollover by setting midnight 31 dec*/
	setDateTime[0] = 00;
	setDateTime[1] = 59;
	setDateTime[2] = 23;
	setDateTime[3] = 7;
	setDateTime[4] = 31;
	setDateTime[5] = 12;
	setDateTime[6] = 23;

/*Testing leap year - works fine
	setDateTime[0] = 00;
	setDateTime[1] = 59;
	setDateTime[2] = 23;
	setDateTime[3] = 7;
	setDateTime[4] = 28;
	setDateTime[5] = 02;
	setDateTime[6] = 23;
	*/
/*	setDateTime[0] = 00;
	setDateTime[1] = 37;
	setDateTime[2] = 22;
	setDateTime[3] = 7;
	setDateTime[4] = 03;
	setDateTime[5] = 12;
	setDateTime[6] = 23;
	*/
	
	set_date_time(setDateTime);

	while (1) {
/* read back date and time after one  */
		uint8_t currentDateTime[7] = {0};

		get_date_time(currentDateTime);
		        // Print the current date and time in one row with ":" separation
        printk("%02d:%02d:%02d %02d/%02d/%02d\n",
               currentDateTime[2], currentDateTime[1], currentDateTime[0],
               currentDateTime[4], currentDateTime[5], currentDateTime[6]);

		k_msleep(SLEEP_TIME_MS);
	}
}
