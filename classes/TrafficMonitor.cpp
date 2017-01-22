//
// Created by zhensheng on 1/14/17.
//

#include "TrafficMonitor.h"

TrafficMonitor::TrafficMonitor(string devFilePath, bool (*getDataHook)()) : devFileIstream(devFilePath), getDataHook(getDataHook) {
}

bool TrafficMonitor::update() {
    hasDatas = true;
    if (!getDataHook())
        return false;
    recordPreDatas();
    sleep(CYCLE_TIME);
    if (!getDataHook())
        return false;
    recordPostDatas();
    return true;
};

void TrafficMonitor::recordDatas(MAP_TYPE &which) {
    resetFileIStream();
    which.clear();

    int line = 0, i;
    string tmp, devName;
    string::size_type charPos1, charPos2;
    long long receive, transmit;
    while (!devFileIstream.eof()) {
        ++line;
        getline(devFileIstream, tmp);

        // Remove two lines at the top of the file
        if (line <= 2)
            continue;

        if ((charPos1 = tmp.find_first_not_of(' ', 0)) != string::npos) {
            if ((charPos2 = tmp.find_first_of(':', charPos1)) != string::npos) {
                devName = tmp.substr(charPos1, charPos2 - charPos1);
                charPos1 = tmp.find_first_not_of(' ', charPos2 + 1);
                charPos2 = tmp.find_first_of(' ', charPos1);
                receive = stoll(tmp.substr(charPos1, charPos2 - charPos1));

                // Move to transmit
                for (i = 0; i < 8; ++i) {
                    charPos1 = tmp.find_first_not_of(' ', charPos2 + 1);
                    charPos2 = tmp.find_first_of(' ', charPos1);
                }
                transmit = stoll(tmp.substr(charPos1, charPos2 - charPos1));

                which.insert(pair<string, FIELD_TYPE>(devName, TwoField<long long>(receive, transmit)));
            }
        }
    }
}

MAP_TYPE TrafficMonitor::get() {
    MAP_TYPE results, tmp;
    for (MAP_TYPE::iterator it = preDatas.begin(); it != preDatas.end(); ++it)
        try {
            tmp = get((*it).first);
            results.insert(tmp.begin(), tmp.end());
        } catch (DevNotFound e) {
            continue;
        }
    return results;
}

MAP_TYPE TrafficMonitor::get(string dev) {
    if (!hasDatas)
        throw NotInitalUpdated();
    MAP_TYPE result;
    MAP_TYPE::iterator preIt, postIt;
    if ((preIt = preDatas.find(dev)) == preDatas.end() || (postIt = postDatas.find(dev)) == postDatas.end())
        throw DevNotFound();
    long long receive_rate, transmit_rate;
    receive_rate = (postIt->second.field1 - preIt->second.field1) / CYCLE_TIME;
    transmit_rate = (postIt->second.field2 - preIt->second.field2) / CYCLE_TIME;
    result.insert(pair<string, FIELD_TYPE>(dev, FIELD_TYPE(receive_rate, transmit_rate)));
    return result;
}