//
// Created by cehrig on 6/4/16.
//

#ifndef DRONE_CONNECT_DRONEBTADAPTER_H
#define DRONE_CONNECT_DRONEBTADAPTER_H

#define BT_MAX_DISCOVERY_DEVICES 255
#define BT_MAX_REPORT_EVENTS 5
#define BT_DEVICEID 0

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

typedef struct _btDevice
{
    bdaddr_t addr;
    char * name;
    char * hname;
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
void resetBTHost();
btParam_t * startBTHost();
void startBTScanMode(btParam_t *);
void stopBTScanMode(btParam_t *);
void stopBTHost(btParam_t *);
void cleanBTHost(btDeviceTable_t);

static int getBTDevices(btParam_t *, btDeviceTable_t);
btDevice_t * selectBTDevice(btDeviceTable_t);
uint16_t connectBTDevice(btParam_t *, btDevice_t *);
void disconnectBTDevice(btParam_t *, uint16_t);

int deviceAdd(btDeviceTable_t, bdaddr_t, char *);
void deviceList(btDeviceTable_t);

#endif //DRONE_CONNECT_DRONEBTADAPTER_H
