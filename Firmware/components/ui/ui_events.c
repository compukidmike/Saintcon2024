#include <string.h>

#include "ui_events.h"

const char *ui_event_type_map[] = {
#define X(val) #val,
    UI_EVENT_TYPE_LIST
#undef X
};

ui_event_type_t get_ui_event_type(const char *type_str) {
    for (size_t i = 0; i < sizeof(ui_event_type_map) / sizeof(ui_event_type_map[0]); i++) {
        if (strcmp(ui_event_type_map[i], type_str) == 0) {
            return i;
        }
    }
    return UI_EVENT_NONE;
}
