#include <pebble.h>

#include "modules/comm/comm.h"
#include "modules/state.h"
#include "windows/draw_map.h"
#include "windows/main_window.h"

static void init() {
    state_init();
    comm_init();
    draw_map_init();
    main_window_push();
}

static void deinit() {
    state_deinit();
    draw_map_deinit();
}

int main() {
    init();
    app_event_loop();
    deinit();
}
