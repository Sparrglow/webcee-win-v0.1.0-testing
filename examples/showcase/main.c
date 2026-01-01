#include "webcee.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SERVER_PORT 8080

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#endif

extern void wce_ui_main(void);

// --- State ---
static int counter = 0;
static int cpu_load = 0;
static int mem_load = 0;

// --- Logic ---

void update_dashboard() {
    char buf[32];
    
    // Update Counter
    sprintf(buf, "%d", counter);
    wce_data_set("counter_val", buf);
    
    // Simulate CPU/Mem fluctuations
    cpu_load = (cpu_load + (rand() % 15) - 5);
    if (cpu_load < 0) cpu_load = 5;
    if (cpu_load > 100) cpu_load = 95;
    
    mem_load = (mem_load + (rand() % 50) - 20);
    if (mem_load < 200) mem_load = 200;
    if (mem_load > 4096) mem_load = 4096;
    
    sprintf(buf, "%d%%", cpu_load);
    wce_data_set("cpu_val", buf);
    
    sprintf(buf, "%d MB", mem_load);
    wce_data_set("mem_val", buf);
}

void on_inc() {
    counter++;
    update_dashboard();
    printf("[Action] Counter incremented: %d\n", counter);
}

void on_reset() {
    counter = 0;
    update_dashboard();
    wce_data_set("user_input", ""); 
    printf("[Action] System Reset\n");
}

int main() {
    srand((unsigned int)time(NULL));
    printf("Starting WebCee Showcase...\n");
    
    if (wce_init(SERVER_PORT) != 0) {
        printf("Failed to init WebCee\n");
        return 1;
    }
    
    // Register callbacks
    wce_register_function("on_inc", on_inc);
    wce_register_function("on_reset", on_reset);
    
    // Initial data
    cpu_load = 15;
    mem_load = 1024;
    update_dashboard();
    wce_data_set("user_input", "Hello WebCee!");
    
    // Build UI
    wce_ui_main();
    
    printf("Server running at http://localhost:%d\n", SERVER_PORT);
    if (wce_start() != 0) {
        printf("Failed to start server\n");
        return 1;
    }
    
    printf("Press Enter to stop server...\n");
    getchar();
    
    wce_stop();
    return 0;
}
