//
// Created by zhensheng on 1/15/17.
//

#include <cstring>
#include "I2CLcd1602.h"


I2CLcd1602::I2CLcd1602(const char *devPath, int addr) {
    i2c_dev = open_i2c(devPath, addr);
    if(i2c_dev <0){
        printf("Errore: %d\n", i2c_dev);
        throw exception();
    }
}

I2CLcd1602::~I2CLcd1602() {
    close_i2c(i2c_dev);
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