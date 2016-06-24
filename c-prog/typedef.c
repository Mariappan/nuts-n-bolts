#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HAL_MAC_ADDR_LEN 6
typedef char hal_mac_addr_t[HAL_MAC_ADDR_LEN];

void copy_mac(hal_mac_addr_t mac)
{
    static hal_mac_addr_t tmp = {0x00, 0x37, 0xFF, 0x12, 0x34, 0x56 };

    memcpy(mac, tmp, HAL_MAC_ADDR_LEN);
    return;
}

int main() {
    hal_mac_addr_t mac;
    char *m = &mac;

    printf("MAC: %x %p\n", mac, mac);
    copy_mac(mac);
    printf("MAC: %x %p\n", mac, mac);

    printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
           m[0], m[1], m[2], m[3], m[4], m[5]);

    return EXIT_SUCCESS;
}
