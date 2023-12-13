#include "svg.h"
#include <stdio.h>
#include <time.h> 


static void convert_color_depth(uint8_t * img, uint32_t px_cnt)
{
#if LV_COLOR_DEPTH == 32
    lv_color32_t * img_argb = (lv_color32_t *)img;
    lv_color_t c;
    lv_color_t * img_c = (lv_color_t *) img;
    uint32_t i;
    for(i = 0; i < px_cnt; i++) {
        c = lv_color_make(img_argb[i].ch.red, img_argb[i].ch.green, img_argb[i].ch.blue);
        img_c[i].ch.red = c.ch.blue;
        img_c[i].ch.blue = c.ch.red;
    }
#elif LV_COLOR_DEPTH == 16
    lv_color32_t * img_argb = (lv_color32_t *)img;
    lv_color_t c;
    uint32_t i;
    for(i = 0; i < px_cnt; i++) {
        c = lv_color_make(img_argb[i].ch.blue, img_argb[i].ch.green, img_argb[i].ch.red);
        img[i * 3 + 2] = img_argb[i].ch.alpha;
        img[i * 3 + 1] = c.full >> 8;
        img[i * 3 + 0] = c.full & 0xFF;
    }
#elif LV_COLOR_DEPTH == 8
    lv_color32_t * img_argb = (lv_color32_t *)img;
    lv_color_t c;
    uint32_t i;
    for(i = 0; i < px_cnt; i++) {
        c = lv_color_make(img_argb[i].ch.red, img_argb[i].ch.green, img_argb[i].ch.blue);
        img[i * 2 + 1] = img_argb[i].ch.alpha;
        img[i * 2 + 0] = c.full;
    }
#elif LV_COLOR_DEPTH == 1
    lv_color32_t * img_argb = (lv_color32_t *)img;
    uint8_t b;
    uint32_t i;
    for(i = 0; i < px_cnt; i++) {
        b = img_argb[i].ch.red | img_argb[i].ch.green | img_argb[i].ch.blue;
        img[i * 2 + 1] = img_argb[i].ch.alpha;
        img[i * 2 + 0] = b > 128 ? 1 : 0;
    }
#endif
}



void lv_svg_set_src_n(lv_obj_t *obj, const char *src){

    NSVGimage* Image;
    Image = nsvgParseFromFile(src, "px", 96.0f);
    if (!Image) {
        printf("Failed to parse SVG file\n");
        return;
    }

    int width = (int)(Image->width);
    int height = (int)(Image->height);

    unsigned char* imageData = (unsigned char*)malloc(width * height * 4);
    if (!imageData) {
        printf("Failed to allocate memory for image data.\n");
        nsvgDelete(Image);
        return;
    }

    NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
    if (!rasterizer) {
        printf("Failed to create rasterizer.\n");
        free(imageData);
        nsvgDelete(Image);
        return;
    }

    nsvgRasterize(rasterizer, Image, 0, 0, 1, imageData, width, height, width * 4);
    convert_color_depth(imageData,  width * height);

    lv_obj_set_size(obj, width, height);
    lv_canvas_set_buffer(obj, imageData, width, height, LV_IMG_CF_TRUE_COLOR_ALPHA);
}


void lv_svg_set_size_n(lv_obj_t *obj, const char *src, int px, int py){

    NSVGimage* Image;
	printf("=============================================\n");
    Image = nsvgParseFromFile(src, "px", 96.0f);
    if (!Image) {
        printf("Failed to parse SVG file\n");
        return;
    }

    int width = (int)(Image->width);
    int height = (int)(Image->height);

    unsigned char* imageData = (unsigned char*)malloc(width * height * 4);
    if (!imageData) {
        printf("Failed to allocate memory for image data.\n");
        nsvgDelete(Image);
        return;
    }

    NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
    if (!rasterizer) {
        printf("Failed to create rasterizer.\n");
        free(imageData);
        nsvgDelete(Image);
        return;
    }

    nsvgRasterize(rasterizer, Image, 0, 0, 1, imageData, width, height, width * 4);
    nsvgDelete(Image);
    int len;
    unsigned char *pngData = stbi_write_png_to_mem((unsigned char *) imageData, width * 4, width, height, 4, &len);
    free(imageData);
    uint8_t * img_data = NULL;
    int error = lodepng_decode32(&img_data, &width, &height, pngData, width * height * 4);
	free(pngData);
    if(error) {
        if(img_data != NULL) {
            lv_mem_free(img_data);
        }
        LV_LOG_WARN("error %" LV_PRIu32 ": %s\n", error, lodepng_error_text(error));
        return LV_RES_INV;
    }

    convert_color_depth(img_data,  width * height);

    lv_obj_set_size(obj, width, height);
    lv_canvas_set_buffer(obj, img_data, width, height, LV_IMG_CF_TRUE_COLOR_ALPHA);
	lv_obj_set_pos(obj, px, py);
	
}

void lv_svg_set_angle_n(lv_obj_t *obj, const char *src, int16_t angle){

    NSVGimage* Image;
	printf("=============================================\n");
    Image = nsvgParseFromFile(src, "px", 96.0f);
    if (!Image) {
        printf("Failed to parse SVG file\n");
        return;
    }

    int width = (int)(Image->width);
    int height = (int)(Image->height);

    unsigned char* imageData = (unsigned char*)malloc(width * height * 4);
    if (!imageData) {
        printf("Failed to allocate memory for image data.\n");
        nsvgDelete(Image);
        return;
    }

    NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
    if (!rasterizer) {
        printf("Failed to create rasterizer.\n");
        free(imageData);
        nsvgDelete(Image);
        return;
    }

    nsvgRasterize(rasterizer, Image, 0, 0, 1, imageData, width, height, width * 4);
    nsvgDelete(Image);

    convert_color_depth(imageData,  width * height);

    lv_obj_set_size(obj, width, height);
    lv_canvas_set_buffer(obj, imageData, width, height, LV_IMG_CF_TRUE_COLOR_ALPHA);

 	lv_color_t cbuf_tmp[width * height];
    
	memcpy(cbuf_tmp, imageData, sizeof(cbuf_tmp));
 	lv_img_dsc_t img;
    img.data = (void *)cbuf_tmp;
    img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    img.header.w = width;
    img.header.h = height;

	lv_canvas_fill_bg(obj, lv_palette_lighten(LV_PALETTE_GREY, 3), LV_OPA_COVER);
    lv_canvas_transform(obj, &img, angle, LV_IMG_ZOOM_NONE, 0, 0, width / 2, height / 2, true);
}

void lv_svg_set_zoom_n(lv_obj_t *obj, const char *src, float zoom){

    clock_t start = clock();

    NSVGimage* Image;
    Image = nsvgParseFromFile(src, "px", 96.0f);
    if (!Image) {
        printf("Failed to parse SVG file\n");
        return;
    }

    int width = (int)(Image->width * zoom);
    int height = (int)(Image->height * zoom);

    unsigned char* imageData = (unsigned char*)malloc(width * height * 4);
    if (!imageData) {
        printf("Failed to allocate memory for image data.\n");
        nsvgDelete(Image);
        return;
    }

    NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
    if (!rasterizer) {
        printf("Failed to create rasterizer.\n");
        free(imageData);
        nsvgDelete(Image);
        return;
    }

    nsvgRasterize(rasterizer, Image, 0, 0, zoom, imageData, width, height, width * 4);
    convert_color_depth(imageData,  width * height);

    lv_obj_set_size(obj, width, height);
    lv_canvas_set_buffer(obj, imageData, width, height, LV_IMG_CF_TRUE_COLOR_ALPHA);

	
};
