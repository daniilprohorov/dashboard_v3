#include "dashboard.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "board.h"
#include "sdkconfig.h"
#include "lvgl.h"
#include "palette.h"

LV_FONT_DECLARE(font_montserrat_100);

#define RPM_LIMIT    CONFIG_DASH_RPM_LIMIT
#define RPM_MAX      ((RPM_LIMIT + 999) / 1000 * 1000) // bar scale: limit rounded up to a tick
#define RPM_WARN     (RPM_LIMIT - 500)                  // bar blinks from here on
#define BLINK_MS     50
#define BAR_H        100
#define BAR_MARGIN   25
#define PRIMARY_H    260
#define SECONDARY_H  120
#define FOOTER_H     (BOARD_LCD_V_RES - BAR_H - PRIMARY_H - SECONDARY_H)
#define CELL_MARGIN  5
#define BORDER_W     3
#define CAPTION_LIFT 28 // caption sits fully above the box's top border; reserve room for it

static const char *const captions[DASH_COUNT] = {
    "RPM", "Gear", "Speed",
    "W t", "O t", "O lvl", "F cns", "Range", "F lvl", "E t",
    "Odometer", "Errors msg", "Time",
};

static lv_obj_t *values[DASH_COUNT];
static lv_obj_t *boxes[DASH_COUNT];
static lv_obj_t *rpm_bar;
static bool rpm_warn;

// Tabular-figure wrapper: every digit advances by the widest digit and is centred in
// that slot, so changing values don't make the text jitter. Kerning around digits is off.
typedef struct {
    lv_font_t font;
    const lv_font_t *base;
    uint16_t digit_w;
} tabular_font_t;

static tabular_font_t tab_100, tab_40;

static bool is_digit(uint32_t c)
{
    return c >= '0' && c <= '9';
}

static bool tabular_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *dsc, uint32_t letter, uint32_t next)
{
    const tabular_font_t *t = (const tabular_font_t *)font;
    if (is_digit(letter) || is_digit(next)) {
        next = 0;
    }
    if (!t->base->get_glyph_dsc(font, dsc, letter, next)) {
        return false;
    }
    if (is_digit(letter)) {
        dsc->ofs_x += (int16_t)((t->digit_w - dsc->adv_w) / 2);
        dsc->adv_w = t->digit_w;
    }
    return true;
}

static const lv_font_t *tabular_init(tabular_font_t *t, const lv_font_t *base)
{
    t->font = *base; // shares glyph data and bitmap callback with the base font
    t->font.get_glyph_dsc = tabular_glyph_dsc;
    t->base = base;
    t->digit_w = 0;
    for (uint32_t c = '0'; c <= '9'; c++) {
        lv_font_glyph_dsc_t g;
        if (base->get_glyph_dsc(base, &g, c, 0) && g.adv_w > t->digit_w) {
            t->digit_w = g.adv_w;
        }
    }
    return &t->font;
}

static void make_cell(lv_obj_t *scr, int field, int x, int y, int w, int h, const lv_font_t *font)
{
    lv_obj_t *box = lv_obj_create(scr);
    lv_obj_remove_style_all(box);
    lv_obj_set_pos(box, x + CELL_MARGIN, y + CELL_MARGIN + CAPTION_LIFT);
    lv_obj_set_size(box, w - 2 * CELL_MARGIN, h - 2 * CELL_MARGIN - CAPTION_LIFT);
    lv_obj_set_style_border_width(box, BORDER_W, 0);
    lv_obj_set_style_border_color(box, lv_color_white(), 0);
    lv_obj_set_scrollable(box, false);
    lv_obj_set_clickable(box, false);

    lv_obj_t *val = lv_label_create(box);
    lv_obj_set_style_text_font(val, font, 0);
    lv_label_set_text_static(val, "--");
    lv_obj_align(val, LV_ALIGN_CENTER, 0, 0);
    values[field] = val;
    boxes[field] = box;

    // Sibling, not child: the box would clip it. Fully above the border so a fill isn't notched.
    lv_obj_t *cap = lv_label_create(scr);
    lv_obj_set_style_text_font(cap, &lv_font_montserrat_24, 0);
    lv_label_set_text_static(cap, captions[field]);
    lv_obj_align_to(cap, box, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

static void make_row(lv_obj_t *scr, int first, int count, int y, int h, const lv_font_t *font)
{
    for (int i = 0; i < count; i++) {
        int x0 = BOARD_LCD_H_RES * i / count;
        int x1 = BOARD_LCD_H_RES * (i + 1) / count;
        make_cell(scr, first + i, x0, y, x1 - x0, h, font);
    }
}

static void make_rpm_bar(lv_obj_t *scr)
{
    const int bar_w = BOARD_LCD_H_RES - 2 * BAR_MARGIN;
    const int fill_h = 50;
    const int fill_y = BAR_H - fill_h - 4;
    const int tick_h = 12;
    const int tick_y = fill_y - tick_h - 2;

    for (int i = 0; i <= RPM_MAX / 1000; i++) {
        int x = BAR_MARGIN + bar_w * i / (RPM_MAX / 1000);

        lv_obj_t *tick = lv_obj_create(scr);
        lv_obj_remove_style_all(tick);
        lv_obj_set_style_bg_color(tick, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(tick, LV_OPA_COVER, 0);
        lv_obj_set_size(tick, BORDER_W, tick_h);
        lv_obj_set_pos(tick, x - BORDER_W / 2, tick_y);

        lv_obj_t *lbl = lv_label_create(scr);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_24, 0);
        lv_label_set_text_fmt(lbl, "%d", i);
        lv_obj_align_to(lbl, tick, LV_ALIGN_OUT_TOP_MID, 0, -2);
    }

    rpm_bar = lv_bar_create(scr);
    lv_obj_remove_style_all(rpm_bar);
    lv_obj_set_pos(rpm_bar, BAR_MARGIN, fill_y);
    lv_obj_set_size(rpm_bar, bar_w, fill_h);
    lv_bar_set_range(rpm_bar, 0, RPM_MAX);
    lv_obj_set_style_border_width(rpm_bar, BORDER_W, LV_PART_MAIN);
    lv_obj_set_style_border_color(rpm_bar, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(rpm_bar, lv_color_white(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(rpm_bar, LV_OPA_COVER, LV_PART_INDICATOR);
}

// Near the rev limiter the bar fill blinks red/off (black screen shows through); otherwise it stays white.
static void blink_tick(lv_timer_t *t)
{
    static bool warn, on = true;
    bool want_on = !rpm_warn || !on;
    if (rpm_warn != warn) {
        warn = rpm_warn;
        lv_obj_set_style_bg_color(rpm_bar, warn ? PALETTE_RED : lv_color_white(), LV_PART_INDICATOR);
        want_on = true;
    }
    if (want_on != on) {
        on = want_on;
        lv_obj_set_style_bg_opa(rpm_bar, on ? LV_OPA_COVER : LV_OPA_TRANSP, LV_PART_INDICATOR);
    }
}

void dashboard_create(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_scrollable(scr, false);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(scr, lv_color_white(), 0);

    const lv_font_t *big = tabular_init(&tab_100, &font_montserrat_100);
    const lv_font_t *small = tabular_init(&tab_40, &lv_font_montserrat_40);

    make_rpm_bar(scr);
    int y = BAR_H;
    make_row(scr, DASH_RPM, 3, y, PRIMARY_H, big);
    y += PRIMARY_H;
    make_row(scr, DASH_WTMP, 7, y, SECONDARY_H, small);
    y += SECONDARY_H;
    make_row(scr, DASH_ODO, 3, y, FOOTER_H, small);
    lv_timer_create(blink_tick, BLINK_MS, NULL);
}

// Relabel only on change: every set_text invalidates the label, and redraws are the CPU cost.
static void set_text(int field, const char *text)
{
    if (strcmp(lv_label_get_text(values[field]), text) != 0) {
        lv_label_set_text(values[field], text);
    }
}

static void set_int(int field, long v)
{
    char buf[16];
    snprintf(buf, sizeof buf, "%ld", v);
    set_text(field, buf);
}

static void set_tenths(int field, unsigned v)
{
    char buf[16];
    snprintf(buf, sizeof buf, "%u.%u", v / 10, v % 10);
    set_text(field, buf);
}

// Cell background fill; text stays white. FILL_NONE = black (transparent box).
typedef enum { FILL_NONE, FILL_RED, FILL_YELLOW, FILL_GREEN, FILL_BLUE } fill_t;

static void set_fill(int field, fill_t fill)
{
    static fill_t current[DASH_COUNT];
    if (current[field] == fill) {
        return;
    }
    current[field] = fill;
    lv_obj_t *box = boxes[field];
    switch (fill) {
    case FILL_RED:    lv_obj_set_style_bg_color(box, PALETTE_RED, 0); break;
    case FILL_YELLOW: lv_obj_set_style_bg_color(box, PALETTE_YELLOW, 0); break;
    case FILL_GREEN:  lv_obj_set_style_bg_color(box, PALETTE_GREEN, 0); break;
    case FILL_BLUE:   lv_obj_set_style_bg_color(box, PALETTE_BLUE, 0); break;
    case FILL_NONE:   break;
    }
    lv_obj_set_style_bg_opa(box, fill == FILL_NONE ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
}

// Coolant and oil share the same bands.
static fill_t engine_temp_fill(int t)
{
    return t < 80 ? FILL_BLUE : t > 100 ? FILL_RED : FILL_NONE;
}

// Unknown field: "-" and no fill. Returns whether the field is known.
static bool known(const dash_data_t *d, int field)
{
    if (d->valid & DASH_BIT(field)) {
        return true;
    }
    set_text(field, "-");
    set_fill(field, FILL_NONE);
    return false;
}

void dashboard_update(const dash_data_t *d)
{
    uint32_t rpm = 0;
    if (known(d, DASH_RPM)) {
        rpm = d->rpm;
        set_int(DASH_RPM, rpm);
    }
    rpm_warn = rpm >= RPM_WARN;
    lv_bar_set_value(rpm_bar, rpm > RPM_MAX ? RPM_MAX : (int32_t)rpm, LV_ANIM_OFF);

    if (known(d, DASH_GEAR)) {
        if (d->gear == 0) {
            set_text(DASH_GEAR, "N");
        } else if (d->gear < 0) {
            set_text(DASH_GEAR, "R");
        } else {
            set_int(DASH_GEAR, d->gear);
        }
    }

    if (known(d, DASH_SPEED)) {
        set_int(DASH_SPEED, d->speed);
    }
    if (known(d, DASH_WTMP)) {
        set_int(DASH_WTMP, d->wtmp);
        set_fill(DASH_WTMP, engine_temp_fill(d->wtmp));
    }
    if (known(d, DASH_OTMP)) {
        set_int(DASH_OTMP, d->otmp);
        set_fill(DASH_OTMP, engine_temp_fill(d->otmp));
    }
    if (known(d, DASH_OLVL)) {
        set_int(DASH_OLVL, d->olvl);
    }
    if (known(d, DASH_FCNS)) {
        set_tenths(DASH_FCNS, d->fcns_x10);
        set_fill(DASH_FCNS, d->fcns_x10 < 70 ? FILL_GREEN : d->fcns_x10 < 100 ? FILL_YELLOW : FILL_RED);
    }
    if (known(d, DASH_RANGE)) {
        set_int(DASH_RANGE, d->range);
        set_fill(DASH_RANGE, d->range < 100 ? FILL_RED : FILL_NONE);
    }
    if (known(d, DASH_FLVL)) {
        set_tenths(DASH_FLVL, d->flvl_x10);
        set_fill(DASH_FLVL, d->flvl_x10 < 100 ? FILL_RED : FILL_NONE);
    }
    if (known(d, DASH_ETMP)) {
        set_int(DASH_ETMP, d->etmp);
        set_fill(DASH_ETMP, d->etmp > 30 ? FILL_RED : d->etmp < -10 ? FILL_BLUE : FILL_NONE);
    }
    if (known(d, DASH_ODO)) {
        set_int(DASH_ODO, d->odometer);
    }
    set_text(DASH_ERR, (d->errors && d->errors[0]) ? d->errors : "-");

    if (known(d, DASH_TIME)) {
        char hhmm[8];
        snprintf(hhmm, sizeof hhmm, "%02u:%02u", d->hour, d->minute);
        set_text(DASH_TIME, hhmm);
    }
}

// ---- demo ----

#define DEMO_TICK_MS   50
#define DEMO_PERIOD_MS 12000 // full up-and-down cycle

static int32_t lerp(int32_t lo, int32_t hi, uint32_t k) // k in 0..1000
{
    return lo + (hi - lo) * (int32_t)k / 1000;
}

static void demo_tick(lv_timer_t *t)
{
    static uint32_t elapsed;
    static uint64_t odo_m = 255003ULL * 1000;
    elapsed = (elapsed + DEMO_TICK_MS) % DEMO_PERIOD_MS;

    // Triangle wave 0 → 1000 → 0.
    uint32_t half = DEMO_PERIOD_MS / 2;
    uint32_t k = elapsed < half ? elapsed * 1000 / half : (DEMO_PERIOD_MS - elapsed) * 1000 / half;

    dash_data_t d = {
        .valid = DASH_ALL,
        .rpm = lerp(800, RPM_LIMIT, k),
        .gear = (int8_t)(k == 0 ? 0 : (k * 6 + 999) / 1000),
        .speed = lerp(0, 240, k),
        .wtmp = lerp(20, 110, k),
        .otmp = lerp(20, 130, k),
        .olvl = lerp(0, 100, k),
        .fcns_x10 = lerp(0, 250, k),
        .range = lerp(0, 800, k),
        .flvl_x10 = lerp(0, 600, k),
        .etmp = lerp(-20, 40, k),
    };

    // Odometer integrates speed, time compressed ×100 so it visibly moves.
    odo_m += (uint64_t)d.speed * 100 * DEMO_TICK_MS / 3600;
    d.odometer = odo_m / 1000;
    d.errors = d.wtmp >= 105 ? "Overheat" : NULL;

    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    d.hour = tm.tm_hour;
    d.minute = tm.tm_min;

    dashboard_update(&d);
}

void dashboard_demo_start(void)
{
    lv_timer_create(demo_tick, DEMO_TICK_MS, NULL);
}
