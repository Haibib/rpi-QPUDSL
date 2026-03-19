// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    // if your system uses another name, add it.
	0
};
static const char* dev_dir = "/dev";

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    if ( !d ) return 0;
    for (int i = 0; ttyusb_prefixes[i]; i++) {
        if (strncmp(ttyusb_prefixes[i], d->d_name, strlen(ttyusb_prefixes[i])) == 0) {
            return 1;
        }
    }
    return 0;
}

static void free_namelist(struct dirent **namelist, int n) {
    if (!namelist) return;
    for (int i = 0; i < n; i++)
        free(namelist[i]);
    free(namelist);
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    struct dirent **namelist;
    int num = scandir(dev_dir, &namelist, filter, alphasort);
    if (num < 0) {
        panic("scandir failed");
    } else if (num == 0 || num > 1) {
        panic("unacceptable number of devices: %d \n", num);
    }
    char* dev_path = strdupf("%s/%s", dev_dir, namelist[0]->d_name);
    free_namelist(namelist, num);
    return dev_path;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    struct dirent **namelist;
    int num = scandir(dev_dir, &namelist, filter, alphasort);
    if (num < 0) {
        panic("scandir failed");
    } else if (num == 0 || num > 1) {
        panic("unacceptable number of devices: %d \n", num);
    }
    int last = 0;
    struct stat last_time;
    for (int i = 0; i < num; i++) {
        struct stat s;
        if (stat(namelist[i]->d_name, &s) < 0 ) {
            continue;
        }
        if (s.st_mtime > last_time.st_mtime) {
            last = i;
            last_time = s;
        }
    }
    char* dev_path = strdupf("%s/%s", dev_dir, namelist[last]->d_name);
    free_namelist(namelist, num);
    return dev_path;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    struct dirent **namelist;
    int num = scandir(dev_dir, &namelist, filter, alphasort);
    if (num < 0) {
        panic("scandir failed");
    } else if (num == 0 || num > 1) {
        panic("unacceptable number of devices: %d \n", num);
    }
    int first = 0;
    struct stat first_time;
    for (int i = 0; i < num; i++) {
        struct stat s;
        if (stat(namelist[i]->d_name, &s) < 0 ) {
            continue;
        }
        if (s.st_mtime < first_time.st_mtime) {
            first = i;
            first_time = s;
        }
    }
    char* dev_path = strdupf("%s/%s", dev_dir, namelist[first]->d_name);
    free_namelist(namelist, num);
    return dev_path;
}
