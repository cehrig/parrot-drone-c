//
// Created by cehrig on 6/4/16.
//

#ifndef DRONE_CONNECT_DRONEDEVICE_H
#define DRONE_CONNECT_DRONEDEVICE_H

typedef enum
{
    D_STANDBY,
    D_CONNECTING,
    D_LINKUP,
    D_LINKDOWN,
    D_DISCONNECTED
} DRONE_STATUS;

typedef enum
{
    BTE,
    WIFI
} CONNECTION_TYPE;

typedef struct _droneDevice
{
    uint8_t status;
    uint8_t type;
    void (*setType)(CONNECTION_TYPE);
    CONNECTION_TYPE (*getType)();
    void (*initDriver)();
} droneDevice_t;

extern droneDevice_t devices;

droneDevice_t * initDeviceStruct();
void freeDeviceStruct(droneDevice_t *);
void setConnectionType(CONNECTION_TYPE);
CONNECTION_TYPE getConnectionType();

#endif //DRONE_CONNECT_DRONEDEVICE_H
