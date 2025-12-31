#include "webcee.h"

void build_ui() {
    wce_container({
        wce_row({
            wce_card({
                wce_text("System Status");
                wce_progress("CPU Usage", cpu_usage);
            });
            
            wce_card({
                wce_text("Controls");
                wce_button("Restart", "on_restart");
            });
        });
        
        // Example of mixing logic
        wce_row({
            wce_text("Logs");
            wce_input("Filter", "filter");
        });
    });
}
