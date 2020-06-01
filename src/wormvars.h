/**
MIT License

Copyright (c) 2020 Joao Rubens Santos Mano Junior joaomanojr@gmail.com

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

#include "type.h"
#define LC_CONF_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "wvars_structs.h"

/* type extensions */
#define CHANNELNAME_EXT 0
#define PASSHASH_EXT 1


extern int fs_write(u16_t blockName, u8_t blockExt, void *block_data, u8_t block_len);
extern int fs_read(u16_t blockName, u8_t blockExt, void *block_data, u8_t block_len);
extern void fs_init(void);
extern PT_THREAD(fs_thread(u8_t reloc_flag));
