#include <iostream>
#include <fstream>
#include <string>

#include "classes/TrafficMonitor.h"
#include "classes/I2CLcd1602.h"

#define DEV_FILE_PATH "/proc/net/dev"

using namespace std;

enum UNIT_LIST {
    B,
    KB,
    MB,
    GB
};

static const char *unitToString[] = {
        "B/s",
        "KB/s",
        "MB/s",
        "GB/s"
};

static const char * toHumanReadAble(long long num, double &result, const char * &ptr);

int main() {
    daemon(0, 0);
    I2CLcd1602 il("/dev/i2c-0", 0x27);
    TrafficMonitor tm(DEV_FILE_PATH);
    double tmp;
    const char *unit;
    string tmpStr;

    string::size_type charPos1, charPos2;

    while (true) {
        tm.update();
        MAP_TYPE results = tm.get("eth0");
        MAP_TYPE::iterator it = results.find("eth0");
        il.clear();

        toHumanReadAble((*it).second.field1, tmp, unit);
        tmpStr = to_string(tmp);
        if ((charPos1 = tmpStr.find_first_of('.')) != string::npos &&
            (charPos2 = tmpStr.find_first_not_of('.', charPos1)) != string::npos && charPos2 - charPos1 >= 1)
            tmpStr = tmpStr.substr(0, charPos2 + 1);
        il.toLine1((string("RX: ") + tmpStr + ' ' + unit).c_str());

        toHumanReadAble((*it).second.field2, tmp, unit);
        tmpStr = to_string(tmp);
        if ((charPos1 = tmpStr.find_first_of('.')) != string::npos &&
            (charPos2 = tmpStr.find_first_not_of('.', charPos1)) != string::npos && charPos2 - charPos1 >= 1)
            tmpStr = tmpStr.substr(0, charPos2 + 1);
        il.toLine2((string("TX: ") + tmpStr + ' ' + unit).c_str());

        sleep(3);
    }
    return 0;
}

static const char * toHumanReadAble(long long num, double &result, const char * &ptr) {
    UNIT_LIST unit = B;
    result = num;
    while (unit != GB && result - 1024 > 0) {
        result /= 1024;
        switch (unit) {
            case B:
                unit = KB;
                break;
            case KB:
                unit = MB;
                break;
            case MB:
                unit = GB;
                break;
        };
    }

    return ptr = unitToString[unit];
}