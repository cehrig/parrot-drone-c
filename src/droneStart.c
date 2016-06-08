#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <strings.h>
#include <errno.h>
#include "droneLog.h"
#include "droneDevice.h"

int main(int argc, char **argv)
{
    char deviceType[2];
    uint8_t ideviceType = 0;
    void (*_runConnection)();
    droneDevice_t * device = initDeviceStruct();

    writelog(LOG_DEFAULT, "Please choose drone connection type");
    writelog(LOG_DEFAULT, "[1] for Bluetooth");
    writelog(LOG_DEFAULT, "[2] for Wi-Fi");

    memset(deviceType, 0, 2);
    scanf("%s", deviceType);
    ideviceType = strtol(deviceType, NULL, 10)-1;

    switch(ideviceType)
    {
        case 0:
        case 1:
            break;
        default:
            writelog(LOG_ERROR, "Wrong device type, exiting..");
            exit(200);
    }

    device->setType(ideviceType);

    if(!device->initDriver) {
        writelog(LOG_ERROR, "No Driver for selected connection type found");
        exit(201);
    }

    device->initDriver();

    return 0;
}