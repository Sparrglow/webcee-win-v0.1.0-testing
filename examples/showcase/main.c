#include "webcee.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 8080

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_ms(x) usleep((x)*1000)
#endif

extern void wce_ui_main(void);

static int counter = 0;

void update_counter() {
    char buf[32];
    sprintf(buf, "%d", counter);
    wce_data_set("counter_val", buf);
}

void on_inc() {
    counter++;
    update_counter();
    printf("Counter incremented to %d\n", counter);
}

void on_reset() {
    counter = 0;
    update_counter();
    wce_data_set("user_input", ""); // Clear input
    printf("Reset performed\n");
}

int main() {
    printf("Starting WebCee Showcase...\n");
    
    if (wce_init(SERVER_PORT) != 0) {
        printf("Failed to init WebCee\n");
        return 1;
    }
    
    // Register callbacks
    wce_register_function("on_inc", on_inc);
    wce_register_function("on_reset", on_reset);
    
    // Initial data
    update_counter();
    wce_data_set("user_input", "Type here...");
    
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
