# Lvgl-svg
Lvgl support svg
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
