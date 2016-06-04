#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "droneLog.h"
#include "droneBTDriver.h"

/**
 * Cross reference: http://git.kernel.org/cgit/bluetooth/bluez.git/tree/tools/hcitool.c?id=7e018c6e02e39faa3b12f144390be4942bd316e6
 */

void initBTDriver()
{
    writelog(LOG_DEFAULT, "Searching for BTE devices");

    int err, opt, dd, dev_id;
    uint8_t own_type = 0x00;
    uint8_t scan_type = 0x01;
    uint8_t filter_type = 0;
    uint16_t interval = htobs(0x0010);
    uint16_t window = htobs(0x0010);

    dev_id = hci_devid("00:15:83:D2:31:7F");

    dd = hci_open_dev(dev_id);
    if (dd < 0) {
        writelog(LOG_ERROR, "Could not open device");
        exit(300);
    }

    err = hci_le_set_scan_parameters(dd, scan_type, interval, window,
                                     own_type, 0x00, 2500);
    if (err < 0) {
        writelog(LOG_ERROR, "Could not open device - %s", strerror(errno));
        exit(301);
    }

    err = hci_le_set_scan_enable(dd, 0x01, 0x00, 2500);
    if (err < 0) {
        writelog(LOG_ERROR, "Enable scan failed - %s", strerror(errno));
        exit(1);
    }

    btDevice_t * devices[BT_MAX_DISCOVERY_DEVICES];
    memset(devices, 0, BT_MAX_DISCOVERY_DEVICES * sizeof(btDevice_t *));

    err = print_advertising_devices(dd, filter_type, devices);
    if (err < 0) {
        writelog(LOG_ERROR, "Could not receive advertising devices - %s", strerror(errno));
        exit(1);
    }

    err = hci_le_set_scan_enable(dd, 0x00, 0x00, 2500);
    if (err < 0) {
        writelog(LOG_ERROR, "Disable scan failed - %s", strerror(errno));
        exit(1);
    }

    hci_close_dev(dd);
}

static int print_advertising_devices(int dd, uint8_t filter_type, btDeviceTable_t devices)
{
    unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
    struct hci_filter nf, of;
    socklen_t olen;
    hci_event_hdr *hdr;
    int num, len;

    olen = sizeof(of);
    if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0) {
        writelog(LOG_ERROR, "Could not get socket options - %s", strerror(errno));
        return -1;
    }

    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0) {
        writelog(LOG_ERROR, "Could not set socket options - %s", strerror(errno));
        return -1;
    }

    /* Wait for 10 report events */
    num = 10;
    while (num--) {
        evt_le_meta_event *meta;
        le_advertising_info *info;
        char addr[18];

        while ((len = read(dd, buf, sizeof(buf))) < 0) {
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
        printf("%s\n", addr);
    }

    done:
    setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

    if (len < 0)
        return -1;

    return 0;
}

int deviceExists(btDeviceTable_t devices, bdaddr_t bdaddr)
{

}

void deviceAdd(btDeviceTable_t devices, bdaddr_t bdaddr)
{
    int x;
    for(x = 0; x < BT_MAX_DISCOVERY_DEVICES; x++) {
        if(x == NULL) {
            devices[x] = bdaddr;
        }
    }
}