//
// Created by zhensheng on 1/15/17.
//

#ifndef NETWORK_SPEED_MONITOR_I2CLCD1602_H
#define NETWORK_SPEED_MONITOR_I2CLCD1602_H

#include <string>
#include <exception>
#include "../includes/i2c.h"
#include "../includes/lcd.h"

using namespace std;

class I2CLcd1602 {
public:
    I2CLcd1602();
    I2CLcd1602(const char *devPath, unsigned char addr);
    I2CLcd1602(const char *I2CBusID, const char *I2CAddress);
    void clear();
    void toLine1(const char *str);
    void toLine2(const char *str);
    virtual ~I2CLcd1602();

private:
    int i2c_dev, i;
    lcd lcd0;
};


#endif //NETWORK_SPEED_MONITOR_I2CLCD1602_H
