#include <iostream>
#include <fstream>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "classes/TrafficMonitor.h"
#include "classes/I2CLcd1602.h"

#define DEV_FILE_PATH "/tmp/network_monitor_dev"

using namespace std;

bool get_dev_details();

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

int main(int argc, char *argv[]) {
    if (argc > 1)
        daemon(0, 0);
    I2CLcd1602 il("/dev/i2c-1", 0x27);
    TrafficMonitor tm(DEV_FILE_PATH, get_dev_details);
    double tmp;
    const char *unit;
    string tmpStr;

    string::size_type charPos1, charPos2;

    while (true) {
        if (tm.update()) {
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
        }
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

bool get_dev_details() {
    const char *headers = "GET /proc/net/dev HTTP/1.1\r\nHost: 192.168.1.1\r\nConnection: close\r\nCache-Control: max-age=0\r\nUser-Agent: Network Monitor\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n\r\n";
    int fd, fd_local;

    if ((fd_local = open(DEV_FILE_PATH, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU)) < -1)
        return false;

    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;

    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    inaddr = inet_addr("192.168.1.1");
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else
    {
        hp = gethostbyname("192.168.1.1");
        if (hp == NULL)
            return false;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(80);

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
        return false;
    if (connect(fd, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return false;

    if (write(fd, headers, strlen(headers)) == -1) {
        return false;
    }

    ssize_t readLen;

    char buffer[4], big_buffer[256], *buffer_pointer = buffer, *pointer;

    size_t buffer_size = 1;

    bool header_end = false;

    int i;

    while ((readLen = read(fd, buffer_pointer, buffer_size)) > 0) {
        /*
        for (i = 0, pointer = buffer_pointer; i < readLen; ++pointer, ++i)
            putc(*pointer, stdout);
        fflush(stdout);
        */
        if (!header_end) {
            if (*buffer_pointer == '\r') { // Header end tag start
                read(fd, buffer_pointer, 3);
                if (strncmp("\n\r\n", buffer_pointer, 3) == 0) { // Is that a real header?
                    header_end = true;
                    buffer_pointer = big_buffer;
                    buffer_size = 256;
                }
            }
            continue;
        }

        write(fd_local, buffer_pointer, readLen);
    }

    close(fd);
    close(fd_local);

    return true;
}