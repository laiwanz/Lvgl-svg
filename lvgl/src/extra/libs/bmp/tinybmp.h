
#ifndef TINY_BMP_H
#define TINY_BMP_H

#include <string>

#include "bitmap_def.h"
class tinybmp
{
public:
	tinybmp(int width, int height);
	tinybmp(std::string file_name);
	virtual ~tinybmp();
	
public:
	void import_image(std::string file_name);
	void export_image(std::string file_name);

public:
	unsigned char** get_image_buf() const;
	
protected:
	unsigned char** create_bitmap_24bit(int height, int width);
	
public:
	int get_width();
	int get_height();
	
protected:
	BITMAP_HEADER m_bh;
	BITMAP_INFO_HEADER m_bih;
	unsigned char** m_image_buf;
	
protected:
	void init_bmp_headers(int width, int height);
};
#endif // !TINY_BMP_H