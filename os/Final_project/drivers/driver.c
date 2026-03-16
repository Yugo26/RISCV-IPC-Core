#include "os.h"

console_t console;

static void console_init() {
    deviceCB_t *console_dev = serial_init();
    if (console_dev) {
        device_open(console_dev, FLAG_INT_RX);
        console.dev = console_dev;
        lock_init(&console.lock);
    }else{
        while(1);
    }
}

void drivers_init() {
    console_init();
}