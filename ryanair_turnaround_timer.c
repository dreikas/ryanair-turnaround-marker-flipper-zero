#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>

#define EVENT_COUNT 8
#define TIME_SHORT_LEN 8

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
    bool running;
} TurnaroundApp;

static const char* event_names[EVENT_COUNT] = {
    "Plane stop", "First PAX out", "Last PAX out", "Boarding start",
    "Boarding end", "Door close", "Beacon on", "Plane moved",
};

/* Small monochrome aircraft mark beside the Ryanair wordmark. */
static void draw_brand_mark(Canvas* canvas) {
    canvas_draw_line(canvas, 6, 1, 6, 11);
    canvas_draw_line(canvas, 5, 4, 1, 7);
    canvas_draw_line(canvas, 7, 4, 11, 7);
    canvas_draw_line(canvas, 5, 8, 3, 10);
    canvas_draw_line(canvas, 7, 8, 9, 10);
    canvas_draw_dot(canvas, 6, 1);
}

static void draw_header(Canvas* canvas) {
    draw_brand_mark(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 15, 10, "RYANAIR");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 72, 9, "TURNAROUND");
    canvas_draw_line(canvas, 0, 13, 127, 13);
}

static void draw_callback(Canvas* canvas, void* context) {
    TurnaroundApp* app = context;
    canvas_clear(canvas);
    draw_header(canvas);

    if(app->show_detail) {
        TurnaroundEvent* ev = &app->events[app->selected];
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 2, 26, ev->label);
        canvas_set_font(canvas, FontSecondary);
        if(ev->recorded) {
            char date_line[11];
            char time_line[9];
            snprintf(date_line, sizeof(date_line), "%04u-%02u-%02u",
                     (unsigned int)(ev->time.year % 10000),
                     (unsigned int)(ev->time.month % 100),
                     (unsigned int)(ev->time.day % 100));
            snprintf(time_line, sizeof(time_line), "%02u:%02u:%02u",
                     (unsigned int)(ev->time.hour % 100),
                     (unsigned int)(ev->time.minute % 100),
                     (unsigned int)(ev->time.second % 100));
            canvas_draw_str(canvas, 2, 40, date_line);
            canvas_draw_str(canvas, 2, 51, time_line);
            canvas_draw_str(canvas, 2, 62, "OK retake  BACK list/hold exit");
        } else {
            canvas_draw_str(canvas, 2, 40, "Not recorded yet");
            canvas_draw_str(canvas, 2, 51, "OK: record timestamp");
            canvas_draw_str(canvas, 2, 62, "BACK list / hold to exit");
        }
        return;
    }

    canvas_set_font(canvas, FontSecondary);
    for(uint8_t row = 0; row < 4; row++) {
        uint8_t idx = app->first_visible + row;
        if(idx >= EVENT_COUNT) break;
        char line[32];
        char time_short[TIME_SHORT_LEN + 1] = "--:--:--";
        if(app->events[idx].recorded) {
            snprintf(time_short, sizeof(time_short), "%02u:%02u:%02u",
                     (unsigned int)(app->events[idx].time.hour % 100),
                     (unsigned int)(app->events[idx].time.minute % 100),
                     (unsigned int)(app->events[idx].time.second % 100));
        }
        snprintf(line, sizeof(line), "%c%-12.12s %s",
                 (idx == app->selected) ? '>' : ' ',
                 app->events[idx].label, time_short);
        canvas_draw_str(canvas, 1, 24 + (row * 9), line);
    }
    canvas_draw_line(canvas, 0, 58, 127, 58);
    canvas_draw_str(canvas, 2, 63, "OK stamp RIGHT info  hold BACK exit");
}

static void input_callback(InputEvent* event, void* context) {
    TurnaroundApp* app = context;

    /* Safety: only a long BACK press exits the app. A short BACK press
       returns from details to the event list; on the list it does nothing. */
    if(event->key == InputKeyBack) {
        if(event->type == InputTypeLong) {
            app->running = false;
        } else if(event->type == InputTypePress && app->show_detail) {
            app->show_detail = false;
        }
        return;
    }

    if(event->type != InputTypePress && event->type != InputTypeLong) return;
    if(app->selected >= EVENT_COUNT) return;

    if(event->key == InputKeyUp && !app->show_detail) {
        if(app->selected > 0) app->selected--;
        if(app->selected < app->first_visible) app->first_visible = app->selected;
    } else if(event->key == InputKeyDown && !app->show_detail) {
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
    app->running = true;
    for(uint8_t i = 0; i < EVENT_COUNT; i++) app->events[i].label = event_names[i];

    app->gui = furi_record_open(RECORD_GUI);
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    while(app->running) {
        furi_delay_ms(50);
        view_port_update(app->view_port);
    }

    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close(RECORD_GUI);
    free(app);
    return 0;
}
