#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>
#include <storage/storage.h>

#define EVENT_COUNT 9
#define TIME_TEXT_LEN 20

typedef struct {
    const char* label;
    bool recorded;
    DateTime time;
} TurnaroundEvent;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    uint8_t selected;
    uint8_t first_visible;
    TurnaroundEvent events[EVENT_COUNT];
    bool show_detail;
} TurnaroundApp;

static const char* event_names[EVENT_COUNT] = {
    "Plane stop",
    "Beacon off",
    "First PAX out",
    "Last PAX out",
    "Boarding start",
    "Boarding end",
    "Door close",
    "Beacon on",
    "Plane moved",
};

static void format_time(const DateTime* dt, char* out, size_t size) {
    snprintf(
        out,
        size,
        "%04d-%02d-%02d %02d:%02d:%02d",
        dt->year,
        dt->month,
        dt->day,
        dt->hour,
        dt->minute,
        dt->second);
}

static void draw_callback(Canvas* canvas, void* context) {
    TurnaroundApp* app = context;
    canvas_clear(canvas);

    // Text-only Ryanair-style header; no official logo artwork is embedded.
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "RYANAIR");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 64, 10, "TURNAROUND");
    canvas_draw_line(canvas, 0, 13, 127, 13);

    if(app->show_detail) {
        TurnaroundEvent* ev = &app->events[app->selected];
        char timestamp[TIME_TEXT_LEN];
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 26, ev->label);
        canvas_set_font(canvas, FontSecondary);
        if(ev->recorded) {
            format_time(&ev->time, timestamp, sizeof(timestamp));
            canvas_draw_str(canvas, 2, 40, timestamp);
            canvas_draw_str(canvas, 2, 53, "OK: record again");
        } else {
            canvas_draw_str(canvas, 2, 40, "Not recorded yet");
            canvas_draw_str(canvas, 2, 53, "OK: record timestamp");
        }
        canvas_draw_str(canvas, 2, 63, "LEFT: list  BACK: exit");
        return;
    }

    canvas_set_font(canvas, FontSecondary);
    for(uint8_t row = 0; row < 4; row++) {
        uint8_t idx = app->first_visible + row;
        if(idx >= EVENT_COUNT) break;

        char line[32];
        char time_short[10] = "--:--:--";
        if(app->events[idx].recorded) {
            snprintf(
                time_short,
                sizeof(time_short),
                "%02d:%02d:%02d",
                app->events[idx].time.hour,
                app->events[idx].time.minute,
                app->events[idx].time.second);
        }
        snprintf(line, sizeof(line), "%c%-15s %s",
                 (idx == app->selected) ? '>' : ' ',
                 app->events[idx].label,
                 time_short);
        canvas_draw_str(canvas, 2, 25 + (row * 9), line);
    }
    canvas_draw_line(canvas, 0, 60, 127, 60);
    canvas_draw_str(canvas, 2, 63, "OK: timestamp  RIGHT: details");
}

static void input_callback(InputEvent* event, void* context) {
    TurnaroundApp* app = context;
    if(event->type != InputTypePress && event->type != InputTypeLong) return;

    if(event->key == InputKeyBack && event->type == InputTypePress) {
        // The main loop detects the exit request via this flag.
        app->show_detail = false;
        app->selected = EVENT_COUNT;
        return;
    }

    if(app->selected >= EVENT_COUNT) return;

    if(event->key == InputKeyUp) {
        if(app->show_detail) return;
        if(app->selected > 0) app->selected--;
        if(app->selected < app->first_visible) app->first_visible = app->selected;
    } else if(event->key == InputKeyDown) {
        if(app->show_detail) return;
        if(app->selected + 1 < EVENT_COUNT) app->selected++;
        if(app->selected >= app->first_visible + 4) app->first_visible = app->selected - 3;
    } else if(event->key == InputKeyRight && event->type == InputTypePress) {
        app->show_detail = true;
    } else if(event->key == InputKeyLeft && event->type == InputTypePress) {
        app->show_detail = false;
    } else if(event->key == InputKeyOk && event->type == InputTypePress) {
        furi_hal_rtc_get_datetime(&app->events[app->selected].time);
        app->events[app->selected].recorded = true;
    }
}

int32_t ryanair_turnaround_timer_app(void* p) {
    UNUSED(p);

    TurnaroundApp* app = malloc(sizeof(TurnaroundApp));
    memset(app, 0, sizeof(TurnaroundApp));

    for(uint8_t i = 0; i < EVENT_COUNT; i++) {
        app->events[i].label = event_names[i];
    }

    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    while(app->selected < EVENT_COUNT) {
        furi_delay_ms(50);
        view_port_update(app->view_port);
    }

    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    free(app);
    return 0;
}
