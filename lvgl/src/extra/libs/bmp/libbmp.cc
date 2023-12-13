/* Copyright (C)
* 2015 - ZengYajian
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include "libbmp.h"

file_operator::file_operator()
{
	fd = NULL;
}

file_operator::file_operator(const file_operator& src)
{
	memcpy(&tag, &src.tag, sizeof(struct tag));
	memcpy(&hdr, &src.hdr, sizeof(struct header));
	memcpy(&pallet, &src.pallet, sizeof(struct color_table));

	bytes_per_line = src.bytes_per_line;
}

file_operator::~file_operator()
{
	if (fd) {
		fflush(fd);
		fclose(fd);
	}
}

int file_operator::init(const char *fname, const char *mode)
{
	fd = fopen(fname, mode);
	if (!fd) {
		perror("fopen error");
		return -1;
	}

	strncpy(name, fname, MAX_NAME_LEN);
	return 0;
}

char * file_operator::get_fname(void)
{
	return name;
}

inline int _get_short_int(char **p)
{
	int tmp;
	char *ptr = *p;

	tmp  = ((*ptr++) & 0xff);
	tmp += ((*ptr++) & 0xff)<<8;

	*p = ptr;
	return tmp;
}

inline int _get_int(char **p)
{
	int tmp;
	char *ptr = *p;

	tmp  = ((*ptr++) & 0xff);
	tmp += ((*ptr++) & 0xff)<<8;
	tmp += ((*ptr++) & 0xff)<<16;
	tmp += ((*ptr++) & 0xff)<<24;

	*p = ptr;
	return tmp;
}

int file_operator::get_image_tag(struct tag *t)
{
	char tmp[BMP_TAG_LEN];
	char *p = tmp;

	if (!fd) return -1;

	fseek(fd, BMP_TAG_OFFSET, SEEK_SET);
	fread(p, BMP_TAG_LEN, 1, fd);

	/* get bmp tag */
	t->type = _get_short_int(&p);
	t->size = _get_int(&p);
	_get_short_int(&p);	/* reserved1 */
	_get_short_int(&p);	/* reserved2 */
	t->offset = _get_int(&p);

	return 0;
}

int file_operator::get_image_hdr(struct header *hdr)
{
	char tmp[BMP_HDR_LEN];
	char *p = tmp;

	if (!fd) return -1;

	fseek(fd, BMP_HDR_OFFSET, SEEK_SET);
	fread(p, BMP_HDR_LEN, 1, fd);

	/* get bmp header */
	hdr->size = _get_int(&p);
	hdr->width = _get_int(&p);
	hdr->height = _get_int(&p);
	hdr->planes = _get_short_int(&p);
	hdr->bitcount = _get_short_int(&p);
	hdr->compression = _get_int(&p);
	hdr->image_size = _get_int(&p);
	hdr->x_bixpelspermeter = _get_int(&p);
	hdr->y_bixpelspermeter = _get_int(&p);
	hdr->color_used = _get_int(&p);
	hdr->color_important = _get_int(&p);

	bytes_per_line = BYTES_PER_LINE(hdr->width, hdr->bitcount);

	return 0;
}

int file_operator::get_color_table(struct color_table *table, int bitcount)
{
	int len = 1 << bitcount;

	fseek(fd, BMP_COLOR_TABLE_OFFSET, SEEK_SET);
	fread((void *)table, len*4, 1, fd);

	return 0;
}

int file_operator::get_one_line(char *buf, int line)
{
	int offset = tag.offset + (bytes_per_line * line);

	fseek(fd, offset, SEEK_SET);
	fread((void *)buf, bytes_per_line, 1, fd);

	return 0;
}

int file_operator::get_one_line(char *buf, int line, int line_ffset, int len)
{
	int offset = tag.offset + (bytes_per_line * line) + line_ffset;

	fseek(fd, offset, SEEK_SET);
	fread((void *)buf, len, 1, fd);

	return 0;
}

int file_operator::set_one_line(char *buf, int line)
{
	int offset = tag.offset + (bytes_per_line * line);

	fseek(fd, offset, SEEK_SET);
	fwrite((void *)buf, bytes_per_line, 1, fd);

	return 0;
}

#define SET_SHORT(ptr, data) \
do { \
	*(ptr++) = (char)(data & 0xff); \
	*(ptr++) = (char)((data>>8) & 0xff); \
} while(0)

#define SET_LONG(ptr, data) \
do { \
	*(ptr++) = (char)(data & 0xff); \
	*(ptr++) = (char)((data>>8) & 0xff); \
	*(ptr++) = (char)((data>>16) & 0xff); \
	*(ptr++) = (char)((data>>24) & 0xff); \
} while(0)

int file_operator::set_image_tag(struct tag *t)
{
	char buf[BMP_TAG_LEN];
	char *p = buf;

	memset(p, 0, BMP_TAG_LEN);

	SET_SHORT(p, t->type);
	 SET_LONG(p, t->size);
	SET_SHORT(p, 0);
	SET_SHORT(p, 0);
	 SET_LONG(p, t->offset);

	fwrite((void *)buf, BMP_TAG_LEN, 1, fd);
	fsync();

	return 0;
}

int file_operator::set_image_hdr(struct header *hdr)
{
	char buf[BMP_HDR_LEN];
	char *p = buf;

	memset(p, 0, BMP_HDR_LEN);
	SET_LONG(p, hdr->size);
	SET_LONG(p, hdr->width);
	SET_LONG(p, hdr->height);
   SET_SHORT(p, hdr->planes);
   SET_SHORT(p, hdr->bitcount);
	SET_LONG(p, hdr->compression);
	SET_LONG(p, hdr->image_size);
	SET_LONG(p, hdr->x_bixpelspermeter);
	SET_LONG(p, hdr->y_bixpelspermeter);
	SET_LONG(p, hdr->color_used);
	SET_LONG(p, hdr->color_important);

	fwrite((void *)buf, BMP_HDR_LEN, 1, fd);
	fsync();

	return 0;
}

int file_operator::set_color_table(struct color_table *table, int bitcount)
{
	int len = 1 << bitcount;

	fseek(fd, BMP_COLOR_TABLE_OFFSET, SEEK_SET);
	fwrite((void *)table, len*4, 1, fd);
	fsync();

	return 0;
}

int file_operator::fsync(void)
{
	fflush(fd);
	return 0;
}

int file_operator::dump_image_info(void)
{
	printf("* image: %s\n", get_fname());
	printf("              type: 0x%04x\n", tag.type);
	printf("              size: %d bytes\n", tag.size);
	printf("            offset: 0x%08x\n", tag.offset);
	printf("--------------------------\n");
	printf("              size: %d\n", hdr.size);
	printf("             width: %d\n", hdr.width);
	printf("            height: %d\n", hdr.height);
	printf("            planes: %d\n", hdr.planes);
	printf("          bitcount: %d\n", hdr.bitcount);
	printf("       compression: %d\n", hdr.compression);
	printf("        image_size: %d\n", hdr.image_size);
	printf(" x_bixpelspermeter: %d\n", hdr.x_bixpelspermeter);
	printf(" y_bixpelspermeter: %d\n", hdr.y_bixpelspermeter);
	printf("        color_used: %d\n", hdr.color_used);
	printf("   color_important: %d\n", hdr.color_important);

	printf("--------------------------\n");
	printf("    bytes per line: %d\n", bytes_per_line);
	printf(" total data length: %d\n", bytes_per_line * hdr.height);

	return 0;
}

int file_operator::dump_color_table(void)
{
	int len = 1 << hdr.bitcount;
	int i;

	if (hdr.bitcount>8)
		return 0;

	for (i=0; i<len; i++) {
		printf("[%3d, %3u, %3u, %3u] ",
			pallet.item[i].blue,
			pallet.item[i].green,
			pallet.item[i].red,
			pallet.item[i].reserved);

		if (((i+1)&0x3) == 0)
			printf("\n");
	}

	return 0;
}



int file_operator::load_image(void)
{
	get_image_tag(&tag);
	get_image_hdr(&hdr);

	if (hdr.bitcount <= 8)
		get_color_table(&pallet, hdr.bitcount);

	return 0;
}

int libbmp::open_image(const char *fname)
{
	raw = new file_operator();
	raw->init(fname, "rb+");
	raw->load_image();

	return 0;
}

int libbmp::dump_image_info(void)
{
	raw->dump_image_info();
	return 0;
}

int libbmp::dump_color_table(void)
{
	raw->dump_color_table();
	return 0;
}

int libbmp::raw_to_gray(void)
{
	class file_operator *gray = new file_operator(*raw);
	int tab_size = 1 << gray->hdr.bitcount;
	unsigned char color;

	/* reg to gray */
	for (int i=0; i<tab_size; i++) {
		color = (unsigned char) ( 0.299*(gray->pallet.item[i].red) +
								  0.587*(gray->pallet.item[i].green) +
								  0.114*(gray->pallet.item[i].blue) );

		gray->pallet.item[i].red = color;
		gray->pallet.item[i].green = color;
		gray->pallet.item[i].blue = color;
	}

	gray->init("gray.bmp", "wb+");
	gray->set_image_tag(&gray->tag);
	gray->set_image_hdr(&gray->hdr);
	gray->set_color_table(&gray->pallet, gray->hdr.bitcount);

	char *p = (char *)malloc(gray->bytes_per_line);
	for (int i = 0; i < gray->hdr.height; i++) {
		raw->get_one_line(p, i);
		gray->set_one_line(p, i);
	}

	free(p);
	delete gray;
	return 0;
}

int libbmp::translation(int x, int y)
{
	int trans_width, trans_height;
	int line_offset;
	int line_start;

	trans_width  = raw->hdr.width  - abs(x);
	trans_height = raw->hdr.height - abs(y);

	if (trans_width<=0 || trans_height<=0)
		return -1;

	line_offset = (x<0)?abs(x):0;
	line_start  = (y<0)?0:y;

	class file_operator *trans = new file_operator(*raw);
	trans->hdr.width = trans_width;
	trans->hdr.height = trans_height;
	trans->bytes_per_line = BYTES_PER_LINE(trans_width, raw->hdr.bitcount);
	trans->hdr.image_size = trans->bytes_per_line * trans->hdr.height;
	trans->tag.size = trans->hdr.image_size + trans->tag.offset;

	trans->init("translation.bmp", "wb+");
	trans->set_image_tag(&trans->tag);
	trans->set_image_hdr(&trans->hdr);
	trans->set_color_table(&trans->pallet, trans->hdr.bitcount);

	char *buf = (char *)malloc(trans->bytes_per_line);
	memset(buf, 0, trans->bytes_per_line);

	for (int i=0; i<trans_height; i++) {
		raw->get_one_line(buf, line_start++, line_offset, trans_width);
		trans->set_one_line(buf, i);
	}

	free(buf);
	delete trans;
	return 0;
}
