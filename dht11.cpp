//
// Created by zhensheng on 1/23/17.
//

#include <iostream>
#include <sstream>
#include <time.h>
#include <unistd.h>
#include "classes/I2CLcd1602.h"
#include "classes/Dht11.h"

#define CYCLE 30

const char *dow[] = {
        "SUN",
        "MON",
        "TUE",
        "WED",
        "THU",
        "FRI",
        "SAT"
};

const char *moy[] = {
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
};

int main(int argc, char *argv[]) {
    I2CLcd1602 il("/dev/i2c-1", 0x26);
    Dht11 dht11(4);
    time_t timestamp;
    struct tm *tm_pointer;

    std::ostringstream out;

    const int *results;

    while (true) {
        try {
            results = dht11.readDatas();
            il.clear();
            time(&timestamp);
            tm_pointer = gmtime(&timestamp);
            out.clear();
            out<<dow[tm_pointer->tm_wday]<< " "<<tm_pointer->tm_mday<<" "<<moy[tm_pointer->tm_mon]<<" "<<tm_pointer->tm_hour<<":"<<tm_pointer->tm_min;
            il.toLine1(out.str().c_str());
            out.clear();
            out<<"T:"<<results[2]<<", H:"<<results[0]<<"%";
            il.toLine2(out.str().c_str());
        } catch (Dht11::DataNotGood e) {
            sleep(3);
            continue;
        }
    }

    return 0;
}