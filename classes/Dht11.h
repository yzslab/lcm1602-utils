//
// Created by zhensheng on 1/23/17.
//

#ifndef DHT11_DHT11_H
#define DHT11_DHT11_H

#define MAXTIMINGS 85

#include <wiringPi.h>
#include <stdint.h>
#include <exception>

class Dht11 {
public:
    class DataNotGood : public std::exception {};

    Dht11(int dhtPin) : dhtPin(dhtPin) {
        if (wiringPiSetup() == -1)
            throw std::exception();
    };

    const int *readDatas();

private:
    const int dhtPin;
    int dht11_dat[5] = { 0, 0, 0, 0, 0 };
};


#endif //DHT11_DHT11_H
