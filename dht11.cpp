//
// Created by zhensheng on 1/23/17.
//

#include <iostream>
#include <iomanip>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include "classes/I2CLcd1602.h"
#include "classes/Dht11.h"

#define CYCLE 30

const char *dow[] = {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
};

const char *moy[] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
};

int main(int argc, char *argv[]) {
    I2CLcd1602 il("/dev/i2c-1", 0x26);
    Dht11 dht11(7);
    time_t timestamp;
    struct tm *tm_pointer;

    std::ostringstream out;

    const int *results;

    while (true) {
        try {
            results = dht11.readDatas();
            il.clear();
            time(&timestamp);
            tm_pointer = localtime(&timestamp);
            out.str(string());
            out<<dow[tm_pointer->tm_wday]<< " "<<tm_pointer->tm_mday<<" "<<moy[tm_pointer->tm_mon]<<" "<<setfill('0')<<setw(2)<<tm_pointer->tm_hour<<":"<<setfill('0')<<setw(2)<<tm_pointer->tm_min;
            il.toLine1(out.str().c_str());
            out.str(string());
            out<<"T:"<<results[2]<<", H:"<<results[0]<<"%";
            il.toLine2(out.str().c_str());
        } catch (Dht11::DataNotGood e) {
            sleep(3);
            continue;
        }
        sleep(CYCLE);
    }

    return 0;
}