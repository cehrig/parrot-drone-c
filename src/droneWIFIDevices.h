#ifndef PARROT_DRONE_C_DRONEWIFIDEVICES_H
#define PARROT_DRONE_C_DRONEWIFIDEVICES_H
#define WIFI_MAXDEVICES 5

#include <net/if.h>

typedef struct _wifi_device
{
    char if_name[IFNAMSIZ];
    unsigned int if_index;
    short if_flags;
} wifi_device_t;

typedef struct _wifi_data
{
    int ctl;
    wifi_device_t ** devices;
} wifi_data_t;

extern int ctl;

wifi_data_t * init_network();
void shutdown_network(wifi_data_t *);
int get_socket(wifi_data_t *);
int get_socket_state(wifi_data_t *);
wifi_device_t ** get_network_devices();
int is_up_and_running(wifi_device_t *);

#endif
