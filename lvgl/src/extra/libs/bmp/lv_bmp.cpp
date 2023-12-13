/**
 * @file lv_bmp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "../../../lvgl.h"
#define STB_IMAGE_IMPLEMENTATION
#include "/opt/zhoux/workspace/lvgl_demo/lvgl/src/extra/libs/stb_image/stb_image.h"
#include "./tinybmp.h"
// #include "./bmpread.h"
// #include <GL/gl.h>

#if LV_USE_BMP

#include <string>
// #include <string>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef unsigned char  BYTE;  
typedef unsigned short WORD;  
typedef unsigned long  DWORD;  
typedef long LONG; 

typedef struct {
    lv_fs_file_t f;
    unsigned int px_offset; 
    int px_width; 
    int px_height; 
    unsigned int bpp; 
    int row_size_bytes;
} bmp_dsc_t;

typedef struct {
    uint16_t signature;    // BMP文件的标识，固定为0x4D42（'BM'的ASCII码）
    uint32_t file_size;    // BMP文件的大小，以字节为单位
    uint32_t reserved;     // 保留字段，必须设置为0
    uint32_t data_offset;  // 数据偏移量，表示位图数据相对于文件起始位置的偏移量
} BMP_FILE_HEADER;


typedef struct {
    uint32_t header_size;      // 信息头大小，固定为40字节
    int32_t width;             // 图像的宽度，以像素为单位
    int32_t height;            // 图像的高度，以像素为单位
    uint16_t planes;           // 色彩平面数，固定为1
    uint16_t bit_depth;        // 每个像素的位深度，表示每个像素的颜色信息所占的位数
    uint32_t compression;      // 压缩方式，表示图像数据的压缩类型
    uint32_t image_size;       // 位图数据的大小，以字节为单位
    int32_t horizontal_res;    // 水平分辨率，表示每米的像素数
    int32_t vertical_res;      // 垂直分辨率，表示每米的像素数
    uint32_t colors_used;      // 调色板中实际使用的颜色数，如果为0，则表示使用所有颜色
    uint32_t colors_important; // 对图像显示有重要影响的颜色数，如果为0，则表示所有颜色都重要
} BMP_INFO_HEADER;

#define BMP_SIGNATURE 0x4D42

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_res_t decoder_info(lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header);
static lv_res_t decoder_open(lv_img_decoder_t * dec, lv_img_decoder_dsc_t * dsc);


static lv_res_t decoder_read_line(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc,
                                  lv_coord_t x, lv_coord_t y, lv_coord_t len, uint8_t * buf);

static void decoder_close(lv_img_decoder_t * dec, lv_img_decoder_dsc_t * dsc);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_bmp_init(void)
{
    lv_img_decoder_t * dec = lv_img_decoder_create();
    lv_img_decoder_set_info_cb(dec, decoder_info);
    lv_img_decoder_set_open_cb(dec, decoder_open);
    lv_img_decoder_set_read_line_cb(dec, decoder_read_line);
    lv_img_decoder_set_close_cb(dec, decoder_close);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


/**
 * If the display is not in 32 bit format (ARGB888) then covert the image to the current color depth
 * @param img the ARGB888 image
 * @param px_cnt number of pixels in `img`
 */
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



/**
 * Get info about a PNG image
 * @param src can be file name or pointer to a C array
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_res_t decoder_info(lv_img_decoder_t * decoder, const void * src, lv_img_header_t * header)
{
    // printf("Decoder BMP info\n");
    LV_UNUSED(decoder);

    lv_img_src_t src_type = lv_img_src_get_type(src);          /*Get the source type*/

    /*If it's a BMP file...*/
    if(src_type == LV_IMG_SRC_FILE) {
        const char * fn = static_cast<const char* >(src);
        if(strcmp(lv_fs_get_ext(fn), "bmp") == 0) {              /*Check the extension*/
            /*Save the data in the header*/
            lv_fs_file_t f;
            lv_fs_res_t res = lv_fs_open(&f, static_cast<const char* >(src), LV_FS_MODE_RD);
            if(res != LV_FS_RES_OK) return LV_RES_INV;
            uint8_t headers[54];

            lv_fs_read(&f, headers, 54, NULL);
            uint32_t w;
            uint32_t h;
            memcpy(&w, headers + 18, 4);
            memcpy(&h, headers + 22, 4);
            header->w = w;
            header->h = h;
            header->always_zero = 0;
            lv_fs_close(&f);
#if LV_COLOR_DEPTH == 32
            uint16_t bpp;
            memcpy(&bpp, headers + 28, 2);
            header->cf = bpp == 32 ? LV_IMG_CF_TRUE_COLOR_ALPHA : LV_IMG_CF_TRUE_COLOR;
#else
            header->cf = LV_IMG_CF_TRUE_COLOR;
#endif
            return LV_RES_OK;
        }
    }
    /* BMP file as data not supported for simplicity.
     * Convert them to LVGL compatible C arrays directly. */
    // else if(src_type == LV_IMG_SRC_VARIABLE) {
    //     return LV_RES_INV;
    // }
    else if (src_type == LV_IMG_SRC_VARIABLE) {

        const lv_img_dsc_t* img_dsc = static_cast<const lv_img_dsc_t* >(src);
        const uint32_t data_size = img_dsc->data_size;

        if (data_size < sizeof(BMP_FILE_HEADER)) {
            return LV_RES_INV;
        }

        const BMP_FILE_HEADER* file_header = (const BMP_FILE_HEADER*)img_dsc->data;
        const BMP_INFO_HEADER* info_header = (const BMP_INFO_HEADER*)(img_dsc->data + sizeof(BMP_FILE_HEADER));

        if (file_header->signature != BMP_SIGNATURE) {
            return LV_RES_INV;
        }

        header->always_zero = 0;

        if (img_dsc->header.cf) {
            header->cf = img_dsc->header.cf;       /* Save the color format */
        }
        else {
            header->cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
        }

        if (img_dsc->header.w) {
            header->w = img_dsc->header.w;         /* Save the image width */
        }
        else {
            header->w = info_header->width;
        }

        if (img_dsc->header.h) {
            header->h = img_dsc->header.h;         /* Save the image height */
        }
        else {
            header->h = info_header->height;
        }

        return LV_RES_OK;
    }
    return LV_RES_INV;         /*If didn't succeeded earlier then it's an error*/
}


/**
 * Open a BMP image and return the decided image
 * @param src can be file name or pointer to a C array
 * @param style style of the image object (unused now but certain formats might use it)
 * @return pointer to the decoded image or `LV_IMG_DECODER_OPEN_FAIL` if failed
 */
static lv_res_t decoder_open(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);
    // uint8_t * img_data = NULL;

    /*If it's a BMP file...*/
    if(dsc->src_type == LV_IMG_SRC_FILE) {
        const char * fn = static_cast<const char* >(dsc->src);

        if(strcmp(lv_fs_get_ext(fn), "bmp") != 0) {
            return LV_RES_INV;       /*Check the extension*/
        }

        bmp_dsc_t b;
        memset(&b, 0x00, sizeof(b));

        lv_fs_res_t res = lv_fs_open(&b.f, fn, LV_FS_MODE_RD);
        if(res == LV_RES_OK) return LV_RES_INV;

        uint8_t header[54];
        lv_fs_read(&b.f, header, 54, NULL);
 

        if(0x42 != header[0] || 0x4d != header[1]) {
            lv_fs_close(&b.f);
            return LV_RES_INV;
        }

        memcpy(&b.px_offset, header + 10, 4);
        memcpy(&b.px_width, header + 18, 4);
        memcpy(&b.px_height, header + 22, 4);
        memcpy(&b.bpp, header + 28, 2);  //bitCount 

        b.row_size_bytes = ((b.bpp * b.px_width + 31) / 32) * 4;

        bool color_depth_error = false;
        if(LV_COLOR_DEPTH == 32 && (b.bpp != 32 && b.bpp != 24)) {
            LV_LOG_WARN("LV_COLOR_DEPTH == 32 but bpp is %d (should be 32 or 24)", b.bpp);
            color_depth_error = true;
        }
        else if(LV_COLOR_DEPTH == 16 && b.bpp != 16) {
            LV_LOG_WARN("LV_COLOR_DEPTH == 16 but bpp is %d (should be 16)", b.bpp);
            color_depth_error = true;
        }
        else if(LV_COLOR_DEPTH == 8 && b.bpp != 8) {
            LV_LOG_WARN("LV_COLOR_DEPTH == 8 but bpp is %d (should be 8)", b.bpp);
            color_depth_error = true;
        }

        if(color_depth_error) {
            dsc->error_msg = "Color depth mismatch";
            lv_fs_close(&b.f);
            return LV_RES_INV;
        }

        dsc->user_data = lv_mem_alloc(sizeof(bmp_dsc_t));
        LV_ASSERT_MALLOC(dsc->user_data);
        if(dsc->user_data == NULL) return LV_RES_INV;
        memcpy(dsc->user_data, &b, sizeof(b));

        int width, height, channels;
        unsigned char* Data = stbi_load(fn, &width, &height, &channels, 4);
        convert_color_depth(Data, width * height);
        printf("width: %d, height: %d\n", width, height);

        dsc->img_data = Data;
        return LV_RES_OK;
    }
    /* BMP file as data not supported for simplicity.
     * Convert them to LVGL compatible C arrays directly. */
    else if(dsc->src_type == LV_IMG_SRC_VARIABLE) {
        printf("BMP decide not supported");
        return LV_RES_INV;
    }

    return LV_RES_INV;    /*If not returned earlier then it failed*/
}


static lv_res_t decoder_read_line(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc,
                                  lv_coord_t x, lv_coord_t y, lv_coord_t len, uint8_t * buf)
{
    LV_UNUSED(decoder);

    bmp_dsc_t * b = static_cast<bmp_dsc_t *>(dsc->user_data);
    y = (b->px_height - 1) - y; /*BMP images are stored upside down*/
    uint32_t p = b->px_offset + b->row_size_bytes * y;
    p += x * (b->bpp / 8);
    lv_fs_seek(&b->f, p, LV_FS_SEEK_SET);
    lv_fs_read(&b->f, buf, len * (b->bpp / 8), NULL);

#if LV_COLOR_DEPTH == 16 && LV_COLOR_16_SWAP == 1
    for(unsigned int i = 0; i < len * (b->bpp / 8); i += 2) {
        buf[i] = buf[i] ^ buf[i + 1];
        buf[i + 1] = buf[i] ^ buf[i + 1];
        buf[i] = buf[i] ^ buf[i + 1];
    }

#elif LV_COLOR_DEPTH == 32
    if(b->bpp == 32) {
        lv_coord_t i;
        for(i = 0; i < len; i++) {
            uint8_t b0 = buf[i * 4];
            uint8_t b1 = buf[i * 4 + 1];
            uint8_t b2 = buf[i * 4 + 2];
            uint8_t b3 = buf[i * 4 + 3];
            lv_color32_t * c = (lv_color32_t *)&buf[i * 4];
            c->ch.red = b2;
            c->ch.green = b1;
            c->ch.blue = b0;
            c->ch.alpha = b3;
        }
    }
    if(b->bpp == 24) {
        lv_coord_t i;

        for(i = len - 1; i >= 0; i--) {
            uint8_t * t = &buf[i * 3];
            lv_color32_t * c = (lv_color32_t *)&buf[i * 4];
            c->ch.red = t[2];
            c->ch.green = t[1];
            c->ch.blue = t[0];
            c->ch.alpha = 0xff;
        }
    }
#endif

    return LV_RES_OK;
}


/**
 * Free the allocated resources
 */
static void decoder_close(lv_img_decoder_t * decoder, lv_img_decoder_dsc_t * dsc)
{
    LV_UNUSED(decoder);
    bmp_dsc_t * b = static_cast<bmp_dsc_t *>(dsc->user_data);
    if(dsc->img_data) {
        lv_mem_free((uint8_t *)dsc->img_data);
        dsc->img_data = NULL;
    }
    lv_fs_close(&b->f);
    lv_mem_free(dsc->user_data);

}




#endif /*LV_USE_BMP*/
