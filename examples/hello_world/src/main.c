#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#include "webcee.h"

// --- Event Handlers ---
void WCE_EVT_save_settings(void) {
    printf("[CMD] Settings Saved:\n");
    printf("  Hostname: %s\n", WCE_MODEL_hostname);
    printf("  SSH Enabled: %s\n", WCE_MODEL_ssh_enabled);
    printf("  Power Mode: %s\n", WCE_MODEL_pwr_mode);
}

void WCE_EVT_reboot_system(void) {
    printf("[CMD] System Rebooting...\n");
}

void WCE_EVT_toggle_led1(void) {
    printf("[CMD] LED 1 Toggled\n");
}

void WCE_EVT_toggle_led2(void) {
    printf("[CMD] LED 2 Toggled\n");
}

// --- Lifecycle ---
void WCE_HOOK_onload(void) {
    printf("Client connected\n");
}

void WCE_HOOK_onunload(void) {
    printf("Client disconnected\n");
}

// --- Main Loop ---
int main(void) {
    if (wce_init(8080) != 0) return 1;
    wce_start();
    
    printf("Pure C UI Demo running on http://localhost:8080\n");
    
    int uptime = 0;
    while(1) {
        uptime++;
        
        // Update Stats
        char buf[32];
        
        // CPU (Random 10-90%)
        int cpu = rand() % 80 + 10;
        snprintf(buf, 32, "%d%%", cpu);
        wce_data_set("sys_cpu_width", buf);
        
        // Mem (Random 30-60%)
        int mem = rand() % 30 + 30;
        snprintf(buf, 32, "%d%%", mem);
        wce_data_set("sys_mem_width", buf);
        
        // Uptime
        snprintf(buf, 32, "%d", uptime);
        wce_data_set("sys_uptime", buf);
        
        // Load
        snprintf(buf, 32, "%.2f", (float)(rand()%100)/100.0 + 0.5);
        wce_data_set("sys_load", buf);

        sleep(1);
    }
    
    wce_stop();
    return 0;
}
