#ifndef SVG_H
#define SVG_H

#include <string.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"
#include "/opt/zhoux/workspace/lvgl_demo/lvgl/src/lvgl.h"


void lv_svg_set_src_n(lv_obj_t *obj, const char *src);

void lv_svg_set_size_n(lv_obj_t *obj, const char *src, int width, int height);

void lv_svg_set_angle_n(lv_obj_t *obj, const char *src, int16_t angle);

void lv_svg_set_zoom_n(lv_obj_t *obj, const char *src, float zoom);



#endif // !SVG_H
