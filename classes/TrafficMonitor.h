//
// Created by zhensheng on 1/14/17.
//

#ifndef NETWORK_SPEED_MONITOR_TRAFFICEMONITOR_H
#define NETWORK_SPEED_MONITOR_TRAFFICEMONITOR_H

// CPP Headers
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <unordered_map>

// C Headers
#include <unistd.h>

using namespace std;

#define FIELD_TYPE TrafficMonitor::TwoField<long long>
#define MAP_TYPE unordered_map<string, FIELD_TYPE>

#define CYCLE_TIME 2

class TrafficMonitor {
public:
    template <typename FIELD_VAR_TYPE>
    class TwoField {
    public:
        const FIELD_VAR_TYPE field1, field2;
        TwoField(FIELD_VAR_TYPE &field1, FIELD_VAR_TYPE &field2) : field1(field1), field2(field2) {};
    };

    class DevNotFound {};
    class NotInitalUpdated {};

    TrafficMonitor(string devFilePath, bool (*getDataHook)());

    bool update();

    MAP_TYPE get();
    MAP_TYPE get(string dev);

private:
    string devFilePath;
    bool hasDatas = false;
    // ifstream devFileIstream;
    MAP_TYPE preDatas;
    MAP_TYPE postDatas;

    bool (*getDataHook)();

    void recordDatas(MAP_TYPE &which);
    void recordPreDatas() { recordDatas(preDatas); }
    void recordPostDatas() { recordDatas(postDatas); }
    /*
    void resetFileIStream() {
        devFileIstream.clear();
        devFileIstream.seekg(ios_base::beg);
    }
     */
};


#endif //NETWORK_SPEED_MONITOR_TRAFFICEMONITOR_H
