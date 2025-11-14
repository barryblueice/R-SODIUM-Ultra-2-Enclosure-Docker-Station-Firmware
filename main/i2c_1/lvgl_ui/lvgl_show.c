#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>

#include "lvgl_init.h"
#include "init_func/init_func.h"

typedef struct {
    int label_index;
    char display_text[64];
} label_update_msg_t;

lv_obj_t *flat_labels[TOTAL_LABELS];
lv_obj_t *pages[PAGE_COUNT];
lv_obj_t *labels[PAGE_COUNT][LABELS_PER_PAGE];
QueueHandle_t label_update_queue;

static int current_display_group = 0;

const char *texts[10] = {
    "", "", "", "", "",
    "", "", "", "", ""
};

LV_FONT_DECLARE(pixelmplus);
LV_IMG_DECLARE(logo);

void update_label_text(int label_index, const char *display_text) {
    if (label_update_queue != NULL) {
        label_update_msg_t msg;
        msg.label_index = label_index;
        strncpy(msg.display_text, display_text, sizeof(msg.display_text) - 1);
        msg.display_text[sizeof(msg.display_text) - 1] = '\0';
        xQueueSend(label_update_queue, &msg, portMAX_DELAY);
    }
}

void update_labels_task(lv_timer_t *timer) {
    label_update_msg_t msg;
    
    while (xQueueReceive(label_update_queue, &msg, 0) == pdTRUE) {
        if (msg.label_index >= 0 && msg.label_index < item_n) {
            lv_label_set_text(flat_labels[msg.label_index], msg.display_text);
        }
    }
}

void switch_display_group_task(lv_timer_t *timer) {
    lv_obj_add_flag(pages[current_display_group], LV_OBJ_FLAG_HIDDEN);
    current_display_group = (current_display_group + 1) % PAGE_COUNT;
    lv_obj_clear_flag(pages[current_display_group], LV_OBJ_FLAG_HIDDEN);
}


void _main_gui(lv_display_t *disp) {
    lv_coord_t w = lv_display_get_horizontal_resolution(disp);
    lv_coord_t h = lv_display_get_vertical_resolution(disp);

    for (int p = 0; p < PAGE_COUNT; p++) {
        pages[p] = lv_obj_create(lv_scr_act());
        lv_obj_set_size(pages[p], w, h);
        lv_obj_set_flex_flow(pages[p], LV_FLEX_FLOW_COLUMN);
        lv_obj_set_scroll_dir(pages[p], LV_DIR_VER);
        lv_obj_set_scrollbar_mode(pages[p], LV_SCROLLBAR_MODE_AUTO);
        lv_obj_set_style_pad_all(pages[p], 0, 0);
        lv_obj_set_style_pad_row(pages[p], 0, 0);

        if (p != 0) lv_obj_add_flag(pages[p], LV_OBJ_FLAG_HIDDEN);

        for (int i = 0; i < LABELS_PER_PAGE; i++) {
            labels[p][i] = lv_label_create(pages[p]);
            lv_label_set_long_mode(labels[p][i], LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_width(labels[p][i], w - 2);
            lv_label_set_text(labels[p][i], texts[p*LABELS_PER_PAGE + i]);
            lv_obj_set_style_text_font(labels[p][i], &pixelmplus, 0);
            lv_obj_set_style_text_line_space(labels[p][i], 0, 0);

            flat_labels[p*LABELS_PER_PAGE + i] = labels[p][i];
        }
    }

    label_update_queue = xQueueCreate(10, sizeof(label_update_msg_t));
    lv_timer_create(update_labels_task, 50, NULL);
    lv_timer_create(switch_display_group_task, 1000, NULL);
}

void _startup_logo(lv_display_t *disp) {
    lv_obj_t *img = lv_img_create(lv_scr_act());
    lv_img_set_src(img, &logo);

    lv_coord_t disp_w = lv_display_get_horizontal_resolution(disp);
    lv_coord_t disp_h = lv_display_get_vertical_resolution(disp);

    uint16_t scale_x = (uint16_t)((256.0 * disp_w) / logo.header.w);
    uint16_t scale_y = (uint16_t)((256.0 * disp_h) / logo.header.h);
    uint16_t scale = scale_x < scale_y ? scale_x : scale_y;

    lv_image_set_scale(img, scale);
    lv_obj_center(img);
}

void update_ui(int index, const char *text) {
    if (index < 0 || index >= item_n) return;
    lv_label_set_text(flat_labels[index], text);
    lv_obj_invalidate(flat_labels[index]);
}