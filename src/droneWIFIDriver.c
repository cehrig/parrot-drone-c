#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "droneLog.h"
#include "droneWIFIDriver.h"
#include "droneWIFIDevices.h"

wifi_data_t * net;

void initWIFIDriver()
{
    net = init_network();
    select_device();
    shutdown_network(net);
}

int select_device()
{
    if(*(net->devices) == NULL) {
        writelog(LOG_ERROR, "No network interfaces found...");
        exit(400);
    }

    wifi_device_t ** test = net->devices;

    while(*test != NULL)
    {
        writelog(LOG_DEBUG, "Name: %s \t Status: %s", (*test)->if_name, (is_up_and_running(*test)) ? "UP":"DOWN");
        test++;
    }
}

