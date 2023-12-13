#include <iostream>
#include <fstream>
#include "tinybmp.h"

tinybmp::tinybmp(int width, int height)
	:m_image_buf(nullptr)
{
	init_bmp_headers(width, height);
	create_bitmap_24bit(height, width);	
}

tinybmp::tinybmp(std::string file_name)
		:m_image_buf(nullptr)
{
	import_image(file_name);
}

tinybmp::~tinybmp()
{
	if(m_image_buf != nullptr)
	{
		delete [] m_image_buf;
		m_image_buf = nullptr ;
	}
}

void tinybmp::import_image(std::string file_name)
{	
	std::ifstream fin;
	fin.open(file_name.c_str(), std::ios::binary);

	fin.read((char*)&m_bh, sizeof(BITMAP_HEADER));
	if(m_bh.type !=0x4D42)
	{
	   fin.close();
	   std::cerr << "[ERROR] tiny.bitmap::tiny_bitmap() Invalid Image File" <<std::endl;
	   exit(1);
	}
	
	fin.read((char*)&m_bih, sizeof(BITMAP_INFO_HEADER));
	
	if(m_bih.bits != 24)
	{
	   fin.close();
	   std::cerr << "[ERROR] tiny.bitmap::tiny_bitmap() can handle 24bits BMP Image" <<std::endl;
	   exit(1);
	}
	
	if (m_image_buf != nullptr)
		delete [] m_image_buf;
	
	m_image_buf = create_bitmap_24bit(m_bih.height, m_bih.width);

	char padding_buf[4];
	for(int i=0 ; i<m_bih.height ; i++)
	{
	   fin.read((char*)(m_image_buf[m_bih.height-1-i]),m_bih.width * 3);
	   if((m_bih.width % 4) != 0)
	   {
		   fin.read(padding_buf, (m_bih.width % 4));
	   }
	}

	fin.close();	
}

void tinybmp::export_image(std::string file_name)//resize
{
	std::ofstream fout;

	fout.open(file_name.c_str(), std::ios::binary);

	fout.write((char*)&m_bh, sizeof(BITMAP_HEADER));
	fout.write((char*)&m_bih, sizeof(BITMAP_INFO_HEADER));

	char padding_buf[4] = {0,0,0,0};
	for(int i=0 ; i<m_bih.height ; i++)
	{
		fout.write((char*)(m_image_buf[m_bih.height-1-i]), m_bih.width * 3);
		
		if((m_bih.width % 4) !=0)
		{
			fout.write(padding_buf, (m_bih.width % 4));
		}
	}
}

unsigned char** tinybmp::create_bitmap_24bit(int height, int width)
{
	unsigned char** image_buf;
	image_buf = new unsigned char*[height];
	for(int i=0 ; i < height ; i++)
	{
		image_buf[i] = new unsigned char[width*3];
	}

	return image_buf;
}

unsigned char** tinybmp::get_image_buf() const
{
	return m_image_buf;
}

int tinybmp::get_width()
{
	if(m_image_buf != nullptr)
		return m_bih.width;
	else
		return -1;
}

int tinybmp::get_height()
{
	if(m_image_buf != nullptr)
		return m_bih.height;
	else
		return -1;
}

void tinybmp::init_bmp_headers(int width, int height)
{
	m_bih.size = 0x28;
	m_bih.width = width;
	m_bih.height = height;
	m_bih.planes = 0x01;
	m_bih.bits = 0x18;
    m_bih.image_size = (m_bih.bits/8) * 3 * m_bih.width * m_bih.height;
    m_bih.x_resolution = 96 * 39.3701;				
    m_bih.y_resolution = 96 * 39.3701;	
	
	m_bh.type = 0x4d42;
	m_bh.size = (m_bih.bits/8) * 3 * m_bih.width * m_bih.height + 0x26;
	m_bh.offset = 0x36;
}