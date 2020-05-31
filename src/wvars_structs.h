/**
MIT License

Copyright (c) 2020 Joao Rubens Santos Mano joaomanojr@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __WVARS_STRUCTS
#define __WVARS_STRUCTS

#define FS_NUM_SECTORS					8
#define FD_MAX							300

struct st_blockHeader{
	u8_t ver:2;
	u8_t size:2;
	u8_t newer:1;
	u8_t ext:3;
	u8_t hash;
	u16_t name;
};

struct st_fileDescriptor{
	u32_t address;
	struct st_blockHeader header;
	u8_t sector;
};

struct st_FsSector{
	u32_t base;
	u32_t current;
};

struct st_fileSystem{
	struct st_FsSector sector[FS_NUM_SECTORS];
	u32_t sector_size;
	u8_t active_sector;

	struct st_fileDescriptor *fd;
	u8_t fd_index;

};

/* Common naming convention using slot/port as name */
union un_fsName{
	struct{
		u8_t slot;
		u8_t port;
	} u8;
	u16_t u16;		
};

#endif

