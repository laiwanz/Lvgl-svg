# Lvgl-svg
In the current V8.3 Version, SVG display is not currently supported, The program use the NanoSVG to displays SVG in the simplest way, but it has smoe drawbacks:
- The scaling operation is not efficient
- Vector drawing is not used
- Will lose some of the features of SVG
### NanoSVG
#### Parser
NanoSVG is a simple stupid single-hander-file SVG parse. The output of the parser is a list of cubic bezier shapes. 
#### Rasterizer
The parser libary is accompanied with really simpler SVG rasterizer 
### Example Usage
display the SVG 
```cpp
    #include "lvgl_demo/lvgl/src/extra/libs/svg/svg.c"
    
    lv_obj_t * svgCanvas = lv_canvas_create(lv_src_act());
    lv_svg_set_src_n(svgCanvas, "./23.svg");
    
```
scale the SVG
```cpp
	#include "lvgl_demo/lvgl/src/extra/libs/svg/svg.c"

	lv_obj_t * svgCanvas = lv_canvas_create(lv_src_act());
    	lv_svg_set_zoom_n(svgCanvas, "./23.svg", 0.5);
    
```

### concrete realization
In the first place, use the NanoSVG to parser the SVG file form the picture address, Thus obstained the image information of the SVG, the Image data contains the width and height of the picture, and specific grahic information,
and then employ the nanorasterizer to resterize graphic data, like RGA8888, finally draw the bitmap data into the canvas which lvgl supported.
```cpp
 void lv_svg_set_src(lv_obj_t *obj, const char *src){

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
```
### Future outlook
- Conjugate vector API from V9 of LVGL
- Improve rasterization efficiency
- SVG animation support
