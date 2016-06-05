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
    bte_params.own_type = 0x00;
    bte_params.scan_type = 0x01;
    bte_params.filter_type = 0x00;
    bte_params.interval = htobs(0x0010);
    bte_params.window = htobs(0x0010);
    bte_params.to = 2000;

    btDevice_t * devices[BT_MAX_DISCOVERY_DEVICES];
    memset(devices, 0, BT_MAX_DISCOVERY_DEVICES * sizeof(btDevice_t *));

    bte_params.err = getBTDevices(devices);

    if (bte_params.err < 0) {
        writelog(LOG_ERROR, "Could not receive advertising devices - %s", strerror(errno));
        exit(303);
    }

    cleanBTHost(devices);
}

void startBTHost()
{
    bte_params.dev_id = hci_devid(BT_DEVICEADDR);

    bte_params.dd = hci_open_dev(bte_params.dev_id);
    if (bte_params.dd < 0) {
        writelog(LOG_ERROR, "Could not open device");
        exit(300);
    }

    bte_params.err = hci_le_set_scan_parameters(
            bte_params.dd,
            bte_params.scan_type,
            bte_params.interval,
            bte_params.window,
            bte_params.own_type,
            0x00,
            2500
    );

    if (bte_params.err < 0) {
        writelog(LOG_ERROR, "Could not open device - %s", strerror(errno));
        exit(301);
    }

    bte_params.err = hci_le_set_scan_enable(
            bte_params.dd,
            0x01,
            0x00,
            bte_params.to
    );

    if (bte_params.err < 0) {
        writelog(LOG_ERROR, "Enable scan failed - %s", strerror(errno));
        exit(302);
    }
}

void stopBTHost()
{
    bte_params.err = hci_le_set_scan_enable(bte_params.dd, 0x00, 0x00, 2500);
    if (bte_params.err < 0) {
        writelog(LOG_ERROR, "Disable scan failed - %s", strerror(errno));
        exit(304);
    }

    hci_close_dev(bte_params.dd);
}

static int getBTDevices(btDeviceTable_t devices)
{
    startBTHost();

    unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
    struct hci_filter nf, of;
    socklen_t olen;
    hci_event_hdr *hdr;
    int num, len;

    olen = sizeof(of);
    if (getsockopt(bte_params.dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
        writelog(LOG_ERROR, "Could not get socket options - %s", strerror(errno));
        return -1;
    }

    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(bte_params.dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
        writelog(LOG_ERROR, "Could not set socket options - %s", strerror(errno));
        return -1;
    }

    /* Wait for 10 report events */
    num = BT_MAX_REPORT_EVENTS;
    while (num--) {
        evt_le_meta_event *meta;
        le_advertising_info *info;
        char addr[18];

        while ((len = read(bte_params.dd, buf, sizeof(buf))) < 0) {
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

        if(deviceExists(devices, info->bdaddr)) {
            continue;
        }

        if(!deviceAdd(devices, info->bdaddr)) {
            writelog(LOG_ERROR, "Can not add device, exiting");
            exit(305);
        }
    }

    done:
    setsockopt(bte_params.dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

    stopBTHost();

    if (len < 0)
        return -1;

    return 0;
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

int deviceAdd(btDeviceTable_t devices, bdaddr_t bdaddr)
{
    int x;
    for(x = 0; x < BT_MAX_DISCOVERY_DEVICES; x++) {
        if(devices[x] == NULL) {
            devices[x] = (btDevice_t *) malloc(sizeof(btDevice_t));
            devices[x]->name = malloc(19 * sizeof(char));
            memset(devices[x]->name, 0, 19);
            ba2str(&bdaddr, devices[x]->name);

            writelog(LOG_DEFAULT, "Added device %s", devices[x]->name);

            devices[x]->addr = bdaddr;
            return 1;
        }
    }

    return 0;
}

void cleanBTHost(btDeviceTable_t devices)
{
    int x;
    for(x = 0; x < BT_MAX_DISCOVERY_DEVICES; x++) {
        if(devices[x] != NULL) {
            free(devices[x]->name);
        }
    }
}