//
// Created by zhensheng on 1/15/17.
//

#include <cstring>
#include "I2CLcd1602.h"

I2CLcd1602::I2CLcd1602() = default;

I2CLcd1602::I2CLcd1602(const char *devPath, unsigned char addr) {
    i2c_dev = open_i2c(devPath, addr);
    if(i2c_dev <0){
        printf("Error: %d\n", i2c_dev);
        throw exception();
    }
}

I2CLcd1602::I2CLcd1602(const char *I2CBusID, const char *I2CAddress){
    char i2cBusFile[64];
    if (strlen(I2CBusID) > 50) {
        printf("Error: Invalid I2C Bus ID.\n");
        throw exception();
    }

    int length = sprintf(i2cBusFile, "/dev/i2c-%s", I2CBusID);
    i2cBusFile[length] = '\0';

    unsigned int address;
    if (sscanf(I2CAddress, "%x", &address) < 0) {
        printf("Error: Invalid I2C Address %s.\n", I2CAddress);
        throw exception();
    }

    i2c_dev = open_i2c(i2cBusFile, (unsigned char) address);
    if(i2c_dev < 0){
        printf("Error: %d\n", i2c_dev);
        throw exception();
    }
}

void I2CLcd1602::clear() {
    lcd_init(&lcd0, i2c_dev);
    lcd_clear(&lcd0);
}

void I2CLcd1602::toLine1(const char *str) {
    lcd_print(&lcd0, str, strlen(str), 0);
}

void I2CLcd1602::toLine2(const char *str) {
    lcd_print(&lcd0, str, strlen(str), 1);
}

I2CLcd1602::~I2CLcd1602() = default;