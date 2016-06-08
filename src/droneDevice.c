#include <stdlib.h>
#include <stdint.h>
#include "droneDevice.h"
#include "droneBTDriver.h"
#include "droneWIFIDriver.h"

droneDevice_t * device;

droneDevice_t * initDeviceStruct()
{
    device = (droneDevice_t *) malloc(sizeof(droneDevice_t));
    device->status = D_STANDBY;
    device->setType = &setConnectionType;
    device->getType = &getConnectionType;

    return device;
}

void setConnectionType(CONNECTION_TYPE type)
{
    device->type = type;
    device->initDriver = NULL;

    switch(type)
    {
        case BTE:
            device->initDriver = initBTDriver;
            break;
        case WIFI:
            device->initDriver = initWIFIDriver;
            break;
    }
}

CONNECTION_TYPE getConnectionType()
{
    return device->type;
}