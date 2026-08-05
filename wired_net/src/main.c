#include <stdio.h>
#include "pico/stdlib.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "wizchip_spi.h"

#define SOCKET_LEDS 0
#define MY_PORT   5000

static wiz_NetInfo g_net_info = {
    .mac  = {0x00, 0x08, 0xDC, 0x00, 0x00, 0x01},
    .ip   = {192, 168, 50, 101},
    .sn   = {255, 255, 255, 0},
    .gw   = {192, 168, 50, 1},
    .dns  = {8, 8, 8, 8},
    .dhcp = NETINFO_STATIC
};

int main() {
    stdio_init_all();
    sleep_ms(3000);

    wizchip_spi_initialize();
    ctlnetwork(CN_SET_NETINFO, (void*)&g_net_info);

    uint8_t buf[256];
    socket(SOCKET_LEDS, Sn_MR_UDP, MY_PORT, 0);

    printf("Actual IP: %d.%d.%d.%d\n",
    g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3]);

    while (1) {
        uint8_t src_ip[4];
        uint16_t src_port;
        int32_t len = recvfrom(SOCKET_LEDS, buf, sizeof(buf), src_ip, &src_port);
        if (len > 0) {
            // your OSC parsing / LED command handling goes here
            printf("Got %ld bytes from %d.%d.%d.%d:%d\n",
                   len, src_ip[0], src_ip[1], src_ip[2], src_ip[3], src_port);
        }
    }
}