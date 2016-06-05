#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "droneLog.h"
#include "droneBTDriver.h"

btParam_t bte_params;

/**
 * Cross reference: http://git.kernel.org/cgit/bluetooth/bluez.git/tree/tools/hcitool.c?id=7e018c6e02e39faa3b12f144390be4942bd316e6
 */

void initBTDriver()
{
    btDevice_t * devices[BT_MAX_DISCOVERY_DEVICES];
    memset(devices, 0, BT_MAX_DISCOVERY_DEVICES * sizeof(btDevice_t *));

    /**
     * Scan BTE Devices
     */
    btParam_t * bte_params = startBTHost();
    startBTScanMode(bte_params);

    if (getBTDevices(bte_params, devices) < 0) {
        writelog(LOG_ERROR, "Could not receive advertising devices - %s", strerror(errno));
        exit(303);
    }

    stopBTScanMode(bte_params);
    stopBTHost(bte_params);

    /**
     * Connect to BTE Device
     */
    btParam_t * sbte_params = startBTHost();
    btDevice_t * device = selectBTDevice(devices);
    connectBTDevice(sbte_params, device);
    stopBTHost(sbte_params);

    cleanBTHost(devices);
}

void resetBTHost()
{
    int ctl;

    if ((ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)) < 0) {
        perror("Can't open HCI socket.");
        exit(1);
    }

    if(ioctl(ctl, HCIDEVDOWN, BT_DEVICEID) < 0) {
        writelog(LOG_DEFAULT, "Can not down device - %s", strerror(errno));
    }

    if(ioctl(ctl, HCIDEVUP, BT_DEVICEID) < 0) {
        if (errno == EALREADY)
            return;
        writelog(LOG_DEFAULT, "Can not up device - %s", strerror(errno));
    }
    close(ctl);

    writelog(LOG_DEBUG, "Device %d reset done", hci_get_route(NULL));
}

btParam_t * startBTHost()
{
    resetBTHost();

    btParam_t * bte_params = (btParam_t *) malloc(sizeof(btParam_t));
    bte_params->own_type = 0x00;
    bte_params->scan_type = 0x01;
    bte_params->filter_type = 0x00;
    bte_params->interval = htobs(0x0010);
    bte_params->window = htobs(0x0010);
    bte_params->to = 2000;
    bte_params->dev_id = BT_DEVICEID;

    bte_params->dd = hci_open_dev(bte_params->dev_id);
    if (bte_params->dd < 0) {
        writelog(LOG_ERROR, "Could not open device");
        exit(300);
    }

    return bte_params;
}

void stopBTHost(btParam_t * bte_params)
{
    writelog(LOG_DEFAULT, "CLOSE: %d", hci_close_dev(bte_params->dd));
    free(bte_params);
}

void startBTScanMode(btParam_t * bte_params)
{
    bte_params->err = hci_le_set_scan_parameters(
            bte_params->dd,
            bte_params->scan_type,
            bte_params->interval,
            bte_params->window,
            bte_params->own_type,
            0x00,
            2500
    );

    if (bte_params->err < 0) {
        writelog(LOG_ERROR, "Could not open device - %s", strerror(errno));
        exit(301);
    }

    bte_params->err = hci_le_set_scan_enable(
            bte_params->dd,
            0x01,
            0x00,
            bte_params->to
    );

    if (bte_params->err < 0) {
        writelog(LOG_ERROR, "Enable scan failed - %s", strerror(errno));
        exit(302);
    }
}

void stopBTScanMode(btParam_t * bte_params)
{
    bte_params->err = hci_le_set_scan_enable(bte_params->dd, 0x00, 0x00, 2500);
    if (bte_params->err < 0) {
        writelog(LOG_ERROR, "Disable scan failed - %s", strerror(errno));
        exit(304);
    }
}

static int getBTDevices(btParam_t * bte_params, btDeviceTable_t devices)
{
    unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
    struct hci_filter nf, of;
    socklen_t olen;
    hci_event_hdr *hdr;
    int num, len;

    olen = sizeof(of);
    if (getsockopt(bte_params->dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
        writelog(LOG_ERROR, "Could not get socket options - %s", strerror(errno));
        return -1;
    }

    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(bte_params->dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
        writelog(LOG_ERROR, "Could not set socket options - %s", strerror(errno));
        return -1;
    }

    /* Wait for 10 report events */
    num = BT_MAX_REPORT_EVENTS;
    while (num--) {
        evt_le_meta_event *meta;
        le_advertising_info *info;
        char addr[18];
        char hname[248];
        memset(hname, 0, 248);

        while ((len = read(bte_params->dd, buf, sizeof(buf))) < 0) {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            goto done;
        }

        hdr = (void *) (buf + 1);
        ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
        len -= (1 + HCI_EVENT_HDR_SIZE);

        meta = (void *) ptr;

        info = (le_advertising_info *) (meta->data + 1);
        ba2str(&info->bdaddr, addr);

        if (hci_read_remote_name(bte_params->dd, &info->bdaddr, sizeof(hname),
                                 hname, 0) < 0)
            strcpy(hname, "[unknown]");

        if(deviceExists(devices, info->bdaddr)) {
            continue;
        }

        if(!deviceAdd(devices, info->bdaddr, hname)) {
            writelog(LOG_ERROR, "Can not add device, exiting");
            exit(305);
        }
    }

    done:
    setsockopt(bte_params->dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

    if (len < 0)
        return -1;

    return 0;
}

void connectBTDevice(btParam_t * bte_params, btDevice_t * device)
{
    uint16_t handle;

    if(hci_le_create_conn(
            bte_params->dd,
            htobs(0x0004),
            htobs(0x0004),
            0x00,
            LE_PUBLIC_ADDRESS,
            device->addr,
            LE_PUBLIC_ADDRESS,
            htobs(0x000F),
            htobs(0x000F),
            htobs(0x0000),
            htobs(0x0C80),
            htobs(0x0001),
            htobs(0x0001),
            &handle,
            25000
    ) < 0) {
        writelog(LOG_ERROR, "Can not connect: %s", strerror(errno));
    }

    uint8_t reason;
    bte_params->err = hci_disconnect(bte_params->dd, handle, reason, 5000);

}

btDevice_t * selectBTDevice(btDeviceTable_t devices)
{
    char deviceID[4];
    int ideviceId;

    writelog(LOG_DEFAULT, "Choose Device ID to pair with device:");
    deviceList(devices);

    memset(deviceID, 0, 4);
    scanf("%s", deviceID);
    ideviceId = strtol(deviceID, NULL, 10)-1;

    return devices[ideviceId];
}

int deviceExists(btDeviceTable_t devices, bdaddr_t bdaddr)
{
    char addr[18];
    ba2str(&bdaddr, addr);

    int x;
    for(x = 0; x < BT_MAX_DISCOVERY_DEVICES; x++) {
        if(devices[x] != NULL && !strcmp(addr, devices[x]->name)) {
            return 1;
        }
    }

    return 0;
}

int deviceAdd(btDeviceTable_t devices, bdaddr_t bdaddr, char * hname)
{
    int x;
    for(x = 0; x < BT_MAX_DISCOVERY_DEVICES; x++) {
        if(devices[x] == NULL) {
            devices[x] = (btDevice_t *) malloc(sizeof(btDevice_t));
            devices[x]->name = malloc(19 * sizeof(char));
            memset(devices[x]->name, 0, 19);
            ba2str(&bdaddr, devices[x]->name);

            devices[x]->hname = (char *) malloc(249 * sizeof(char));
            strcpy(devices[x]->hname, hname);

            writelog(LOG_DEFAULT, "Found device %s: %s", devices[x]->name, devices[x]->hname);

            devices[x]->addr = bdaddr;
            return 1;
        }
    }

    return 0;
}

void deviceList(btDeviceTable_t devices)
{
    int x;
    for(x = 0; x < BT_MAX_DISCOVERY_DEVICES; x++) {
        if(devices[x] != NULL) {
            writelog(LOG_DEFAULT, "\t[%d] %s  BD Address: %s", x+1, devices[x]->hname, devices[x]->name);
        }
    }
}

void cleanBTHost(btDeviceTable_t devices)
{
    int x;
    for(x = 0; x < BT_MAX_DISCOVERY_DEVICES; x++) {
        if(devices[x] != NULL) {
            free(devices[x]->name);
            free(devices[x]->hname);
        }
    }
}