//
// Created by cehrig on 6/4/16.
//

#ifndef DRONE_CONNECT_DRONEBTADAPTER_H
#define DRONE_CONNECT_DRONEBTADAPTER_H

#define BT_MAX_DISCOVERY_DEVICES 255
#define BT_MAX_REPORT_EVENTS 20
#define BT_DEVICEADDR "00:15:83:D2:31:7F"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

typedef struct _btDevice
{
    bdaddr_t addr;
    char * name;
} btDevice_t;

typedef struct _btParam
{
    int err;
    int opt;
    int dd;
    int dev_id;
    uint8_t own_type;
    uint8_t scan_type;
    uint8_t filter_type;
    uint16_t interval;
    uint16_t window;
    uint16_t to;
} btParam_t;

typedef btDevice_t ** btDeviceTable_t;

void initBTDriver();
void startBTHost();
void stopBTHost();
void cleanBTHost(btDeviceTable_t);
static int getBTDevices(btDeviceTable_t);
int deviceAdd(btDeviceTable_t, bdaddr_t);

#endif //DRONE_CONNECT_DRONEBTADAPTER_H
