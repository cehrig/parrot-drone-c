//
// Created by cehrig on 6/4/16.
//

#ifndef DRONE_CONNECT_DRONEBTADAPTER_H
#define DRONE_CONNECT_DRONEBTADAPTER_H

#define BT_MAX_DISCOVERY_DEVICES 255

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

typedef struct _btDevice
{
    bdaddr_t addr;
    char * name;
} btDevice_t;

typedef btDevice_t ** btDeviceTable_t;

void initBTDriver();
static int print_advertising_devices(int, uint8_t, btDeviceTable_t);
void addDevice(btDeviceTable_t, bdaddr_t);

#endif //DRONE_CONNECT_DRONEBTADAPTER_H
