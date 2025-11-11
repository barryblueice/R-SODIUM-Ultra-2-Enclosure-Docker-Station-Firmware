#include "lvgl.h"

#include "lvgl_init.h"

LV_FONT_DECLARE(fonts);
LV_IMG_DECLARE(logo);

lv_obj_t *labels[4];

void _main_gui(lv_display_t *disp)
{
    lv_coord_t w = lv_display_get_horizontal_resolution(disp);
    lv_coord_t h = lv_display_get_vertical_resolution(disp);

    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, w, h);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_pad_row(cont, 0, 0);

    const char *texts[] = {
        "行1：这是测试文字1aaaaaaaaaaaaaaa",
        "行2：这是测试文字2bbbbbbbbbbbbbbb",
        "行3：这是测试文字3ccccccccccccccc",
        "行4：这是测试文字4ddddddddddddddd",
    };
    for (int i = 0; i < 4; i++) {
        lv_obj_t *label = lv_label_create(cont);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_width(label, w - 2);
        lv_label_set_text(label, texts[i]);
        lv_obj_set_style_text_font(label, &fonts, 0);
        lv_obj_set_style_text_line_space(label, 0, 0);
    }
}

void _startup_logo(lv_display_t *disp)
{
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
