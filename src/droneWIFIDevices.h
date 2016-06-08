#ifndef PARROT_DRONE_C_DRONEWIFIDEVICES_H
#define PARROT_DRONE_C_DRONEWIFIDEVICES_H
#define WIFI_MAXDEVICES 5

#include <net/if.h>

typedef struct _wifi_device
{
    char if_name[IFNAMSIZ];
    unsigned int if_index;
} wifi_device_t;

typedef struct _wifi_data
{
    wifi_device_t ** devices;
} wifi_data_t;

extern int ctl;

int get_socket();
int get_socket_state();
wifi_device_t ** get_network_devices();

#endif
