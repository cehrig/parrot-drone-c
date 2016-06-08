#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include "droneLog.h"
#include "droneWIFIDevices.h"

int ctl;

int get_socket()
{
    ctl = socket(AF_INET, SOCK_RAW, 1);

    if(ctl < 0) {
        return (errno) ? errno : 1;
    }

    return 0;
}

int get_socket_state()
{
    int error = 0, retval = 0;
    socklen_t len = sizeof(error);
    retval = getsockopt(ctl, SOL_SOCKET, SO_ERROR, &error, &len);

    if (retval != 0) {
        return errno;
    }

    if (error != 0) {
        return errno;
    }

    return 0;
}

wifi_device_t ** get_network_devices()
{
    int x, y = 0;
    struct ifreq ifr;
    wifi_device_t ** devices = (wifi_device_t **) malloc(WIFI_MAXDEVICES * sizeof(wifi_device_t *));

    for(x = 0; x <= WIFI_MAXDEVICES; x++) {
        ifr.ifr_ifindex = x;

        if(ioctl(ctl, SIOCGIFNAME, &ifr) < 0) {
            continue;
        }

        *(devices+(y)) = (wifi_device_t *) malloc(sizeof(wifi_device_t));
        (*(devices+(y)))->if_index = x;
        strncpy((*(devices+(y)))->if_name, ifr.ifr_name, IFNAMSIZ);

        y++;
    }

    *(devices+y) = NULL;

    return devices;
}