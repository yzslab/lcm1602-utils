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
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include "classes/TrafficMonitor.h"
#include "classes/I2CLcd1602.h"

#define DEV_FILE_PATH "/tmp/network_monitor_dev"

using namespace std;

bool get_dev_details();
bool get_dev_detail_by_socket();

enum UNIT_LIST {
    B,
    KB,
    MB,
    GB,
    bps,
    Kbps,
    Mbps,
    Gbps
};

static const char *unitToString[] = {
        "B/s",
        "KB/s",
        "MB/s",
        "GB/s",
        "bps",
        "Kbps",
        "Mbps",
        "Gbps"
};

static const char * toHumanReadAble(long long num, double &result, const char * &ptr);
static const char * toHumanReadAbleBps(long long num, double &result, const char * &ptr);

int main(int argc, char *argv[]) {
    setbuf(stdout, nullptr); // Turn of stdout buffer

    if (unlink(DEV_FILE_PATH) == -1) {
        if (errno != ENOENT) {
            perror("unlink()");
            return 1;
        }
    }

    if (argc == 4 || argc == 2) {
        printf("Run as daemon.\n");
        daemon(0, 0);
    }

    I2CLcd1602 il;

    if (argc < 3) {
        printf("Use default I2C bus: /dev/i2c-0, address: 0x27.\n");
        il = I2CLcd1602("/dev/i2c-0", 0x27);
    } else {
        printf("Use custom I2C bus: /dev/i2c-%s, address: 0x%s.\n", argv[1], argv[2]);
        il = I2CLcd1602(argv[1], argv[2]);
    }

    if (getuid() == 0) {
        printf("Set to nobody.\n");
        setgid(65534);
        setuid(65534);
    }

    TrafficMonitor tm(DEV_FILE_PATH, get_dev_detail_by_socket);
    double tmp;
    const char *unit;
    const char *tmpStringPointer;
    string tmpStr;

    string::size_type charPos1, charPos2;

    auto toHumanReadAbleCallable = toHumanReadAbleBps;

    while (true) {
        if (toHumanReadAbleCallable == toHumanReadAbleBps)
            toHumanReadAbleCallable = toHumanReadAble;
        else
            toHumanReadAbleCallable = toHumanReadAbleBps;

        if (tm.update()) {
            MAP_TYPE results = tm.get("eth0");
            MAP_TYPE::iterator it = results.find("eth0");
            il.clear();

            toHumanReadAbleCallable((*it).second.field1, tmp, unit);
            tmpStr = to_string(tmp);
            if ((charPos1 = tmpStr.find_first_of('.')) != string::npos &&
                (charPos2 = tmpStr.find_first_not_of('.', charPos1)) != string::npos && charPos2 - charPos1 >= 1)
                tmpStr = tmpStr.substr(0, charPos2 + 1);

            tmpStringPointer = (string("RX: ") + tmpStr + ' ' + unit).c_str();
            il.toLine1(tmpStringPointer);

            toHumanReadAbleCallable((*it).second.field2, tmp, unit);
            tmpStr = to_string(tmp);
            if ((charPos1 = tmpStr.find_first_of('.')) != string::npos &&
                (charPos2 = tmpStr.find_first_not_of('.', charPos1)) != string::npos && charPos2 - charPos1 >= 1)
                tmpStr = tmpStr.substr(0, charPos2 + 1);

            tmpStringPointer = (string("TX: ") + tmpStr + ' ' + unit).c_str();
            il.toLine2(tmpStringPointer);
        } else {
            printf("Error on update, retry.\n");
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

static const char * toHumanReadAbleBps(long long num, double &result, const char * &ptr) {
    UNIT_LIST unit = bps;
    result = num * 8;
    while (unit != Gbps && result - 1024 > 0) {
        result /= 1024;
        switch (unit) {
            case bps:
                unit = Kbps;
                break;
            case Kbps:
                unit = Mbps;
                break;
            case Mbps:
                unit = Gbps;
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

bool get_dev_detail_by_socket() {
    bool returnValue = false;

    int fd; // TCP Socket file descriptor
    int fd_local; // Local file descriptor

    time_t now;

    // Try to open local file
    if ((fd_local = open(DEV_FILE_PATH, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU)) < -1) {
        perror("open");
        goto END_FUNCTION;
    }

    // Init socket
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    inaddr = inet_addr("192.168.1.1");
    if (inaddr != INADDR_NONE) {
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    } else {
        hp = gethostbyname("192.168.1.1");
        if (hp == NULL) {
            perror("gethostbyname");
            goto CLOSE_FD_LOCAL;
        }
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(8890);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        goto CLOSE_FD_LOCAL;
    }

    now = time(nullptr);
    printf("Connecting to server...\n");
    if (connect(fd, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
        perror("connect");
        goto CLOSE_SOCKET_FD;
    }
    printf("Connected after %lus.\n", time(nullptr) - now);

    /*
    if (write(fd, headers, strlen(headers)) == -1) {
        return false;
    }
     */

    ssize_t readLen;
    char big_buffer[256];

    while ((readLen = read(fd, &big_buffer, 256)) > 0) {
        write(fd_local, &big_buffer, readLen);
    }
    returnValue = true;

    CLOSE_SOCKET_FD:
    close(fd);

    CLOSE_FD_LOCAL:
    close(fd_local);

    END_FUNCTION:
    return returnValue;
}