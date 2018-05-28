//
// Created by Zhensheng on 2018/5/28.
//

#include <cstdio>
#include <cstring>
#include "classes/TrafficMonitor.h"
#include "classes/I2CLcd1602.h"

using namespace std;

static size_t readStreamFromBuffer(char *lineBuffer);
static size_t readStreamFromStdin(char *lineBuffer);

static char *textPointer;

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s I2C_BUS_ID I2C_ADDRESS MICRO_SECOND_DELAY TEXT\nExample: %s 0 500 27 \"Hello World!\"\n", argv[0], argv[0]);
        return 1;
    }

    if (strlen(argv[1]) >= 50) {
        fprintf(stderr, "I2C_BUS_ID too long, must < 50 bytes.\n");
        return 1;
    }

    char *I2CBusIdPointer = argv[1], *I2CAddressPointer = argv[2], *delayPointer = argv[3];
    textPointer = argv[4];

    auto streamReader = readStreamFromBuffer;
    if (textPointer[0] == '-' && textPointer[1] == '\0') {
        printf("Read from stdin.\n");
        streamReader = readStreamFromStdin;
    }

    char i2cBusFile[64];
    int length = sprintf(i2cBusFile, "/dev/i2c-%s", I2CBusIdPointer);
    i2cBusFile[length] = '\0';

    int address;
    sscanf(I2CAddressPointer, "%x", &address);

    unsigned int ms;
    sscanf(delayPointer, "%u", &ms);
    __useconds_t us = ms * 1000;

    printf("I2C Bus: %s, I2C Address: %x\n", i2cBusFile, address);

    I2CLcd1602 il(i2cBusFile, (unsigned char) address);


    void (*clearLcd[2])(I2CLcd1602 *il);
    clearLcd[0] = [] (I2CLcd1602 *il) { il->clear(); };
    clearLcd[1] = [] (I2CLcd1602 *il) {};

    void (*printLineCallable[2])(I2CLcd1602 *il, const char *str);
    printLineCallable[0] = [] (I2CLcd1602 *il, const char *str) { il->toLine1(str); };
    printLineCallable[1] = [] (I2CLcd1602 *il, const char *str) { il->toLine2(str); };

    void (*delay[2])(__useconds_t us);
    delay[0] = [] (__useconds_t us) {};
    delay[1] = [] (__useconds_t us) { usleep(us); };


    char lineBuffer[32];

    int tag = 0;

    size_t readLength = streamReader(lineBuffer);
    while (readLength > 0) {
        printf("TAG: %d\n", tag);

        printf("Write: %s\n", lineBuffer);
        clearLcd[tag](&il);
        printLineCallable[tag](&il, lineBuffer);

        readLength = streamReader(lineBuffer);

        delay[tag & (bool) readLength](us); // If readLength == 0, skip delay

        tag = !tag;
    }

    return 0;
}

static size_t readStreamFromBuffer(char *lineBuffer) {
    static size_t readCounter = 0;

    strncpy(lineBuffer, textPointer + readCounter, 16);
    lineBuffer[16] = '\0';
    size_t readLength = strlen(lineBuffer);

    readCounter += readLength;

    return readLength;
}

static size_t readStreamFromStdin(char *lineBuffer) {
    size_t readLength = fread(lineBuffer, 1, 16, stdin);
    lineBuffer[readLength] = '\0';
    // printf("%lu byte(s) read.\n", (unsigned long) readLength);
    // printf("%s", lineBuffer);

    return readLength;
}