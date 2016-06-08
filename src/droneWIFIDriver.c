#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "droneLog.h"
#include "droneWIFIDriver.h"
#include "droneWIFIDevices.h"

void initWIFIDriver()
{
    writelog(LOG_DEFAULT, "%s", strerror(get_socket()));
    get_network_devices();
}

