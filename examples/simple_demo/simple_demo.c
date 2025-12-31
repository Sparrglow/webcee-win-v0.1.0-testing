#include "webcee.h"
#include <stdio.h>

// Event Callback
void WCE_EVT_on_led_toggle(void) {
    printf("[Demo] LED Toggled!\n");
}

int main() {
    printf("Starting WebCee Simple Demo...\n");
    
    if (wce_init(8080) != 0) {
        printf("Failed to init server\n");
        return 1;
    }
    
    wce_start();
    
    printf("Server running at http://localhost:8080\n");
    printf("Press Enter to exit...\n");
    getchar();
    
    wce_stop();
    return 0;
}
