#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include "droneLog.h"
#include "droneWIFIDevices.h"

int ctl;

wifi_data_t * init_network()
{
    wifi_data_t * net = (wifi_data_t *) malloc(sizeof(wifi_data_t));
    get_socket(net);
    get_network_devices(net);

    return net;
}

void shutdown_network(wifi_data_t * net)
{
    if(net->devices != NULL) {
        if(*(net->devices) != NULL) {
            wifi_device_t ** iterator = net->devices;
            while(*iterator != NULL) {
                free(*iterator);
                iterator++;
            }
        }
        free(net->devices);
    }

    close(net->ctl);
    free(net);
}

int get_socket(wifi_data_t * net)
{
    net->ctl = socket(AF_INET, SOCK_RAW, 1);

    if(net->ctl < 0) {
        return (errno) ? errno : 1;
    }

    return 0;
}

int get_socket_state(wifi_data_t * net)
{
    int error = 0, retval = 0;
    socklen_t len = sizeof(error);
    retval = getsockopt(net->ctl, SOL_SOCKET, SO_ERROR, &error, &len);

    if (retval != 0) {
        return errno;
    }

    if (error != 0) {
        return errno;
    }

    return 0;
}

wifi_device_t ** get_network_devices(wifi_data_t * net)
{
    int x, y = 0;
    struct ifreq ifr, ifrf;
    memset(&ifr, 0, sizeof(struct ifreq));

    wifi_device_t ** devices = (wifi_device_t **) malloc(WIFI_MAXDEVICES * sizeof(wifi_device_t *));

    for(x = 0; x <= WIFI_MAXDEVICES; x++) {
        ifr.ifr_ifindex = x;

        if(ioctl(net->ctl, SIOCGIFNAME, &ifr) < 0) {
            continue;
        }

        memset(&ifrf, 0, sizeof(struct ifreq));
        strncpy(ifrf.ifr_name, ifr.ifr_name, IFNAMSIZ);
        ioctl(net->ctl, SIOCGIFFLAGS, &ifrf);

        *(devices+(y)) = (wifi_device_t *) malloc(sizeof(wifi_device_t));
        (*(devices+(y)))->if_index = x;
        (*(devices+(y)))->if_flags = ifrf.ifr_flags;
        strncpy((*(devices+(y)))->if_name, ifr.ifr_name, IFNAMSIZ);

        y++;
    }

    *(devices+y) = NULL;
    net->devices = devices;

    return devices;
}

int is_up_and_running(wifi_device_t * device)
{
    return (device->if_flags & IFF_UP
            && device->if_flags & IFF_RUNNING);
}