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

#include <string.h>

#include "type.h"
#define LC_CONF_INCLUDE "lc-addrlabels.h"
#include "pt.h"

#include "timer.h"
// #include "timer23xx.h"

#include "crc32.h"
// #include "channelName.h"
#include "flash.h"
#include "wormvars.h"

/* debug level */
// #define DEBUG_LEVEL        0
#include "debug.h"

#ifdef DEBUG_CONSOLE
// (debug) insert delays between fswrite steps so each of them can be analysed
// #define DEBUG_FSWRITE_SLOW
// (debug) insert delays between relocation read and write
// #define DEBUG_WRITE_DURING_RELOC
// (debug) forces often rellocation
// #define DEBUG_SMALL_SECTORS
#endif

/*****************************************************************************/
#define LONG_BLANK_PATTERN 0xFFFFFFFF

#define FS_BLOCK_SIZE 32
#define FDS_PER_SECTOR (FILESYSTEM_SECTOR_SIZE / FS_BLOCK_SIZE)
#define FS_MIN_BLANKSECTORS 2

#define FS_VERSION 0

#define FS_SIZE32 1
#define FS_SIZEINVALID 0

#define FS_BLOCK_NEWER 1
#define FS_BLOCK_CURRENT 0

/* (debug) enforces sector 'worming' forcing relocation of variables earlier */
#ifndef DEBUG_SMALL_SECTORS
#define FILESYSTEM_SECTOR_0 0x0F4000    /* correct values */
#define FILESYSTEM_SECTOR_1 0x0F5000
#define FILESYSTEM_SECTOR_2 0x0F6000
#define FILESYSTEM_SECTOR_3 0x0F7000
#define FILESYSTEM_SECTOR_4 0x0F8000
#define FILESYSTEM_SECTOR_5 0x0F9000
#define FILESYSTEM_SECTOR_6 0x0FA000
#define FILESYSTEM_SECTOR_7 0x0FB000
#define FILESYSTEM_SECTOR_SIZE 4096
#else
#define FILESYSTEM_SECTOR_0 0x0F4F00    /* debug values -- 8 blocks */
#define FILESYSTEM_SECTOR_1 0x0F5F00
#define FILESYSTEM_SECTOR_2 0x0F6F00
#define FILESYSTEM_SECTOR_3 0x0F7F00
#define FILESYSTEM_SECTOR_4 0x0F8F00
#define FILESYSTEM_SECTOR_5 0x0F9F00
#define FILESYSTEM_SECTOR_6 0x0FAF00
#define FILESYSTEM_SECTOR_7 0x0FBF00
#define FILESYSTEM_SECTOR_SIZE 256
#endif

const u32_t Sector_base[]={
    FILESYSTEM_SECTOR_0,
    FILESYSTEM_SECTOR_1,
    FILESYSTEM_SECTOR_2,
    FILESYSTEM_SECTOR_3,
    FILESYSTEM_SECTOR_4,
    FILESYSTEM_SECTOR_5,
    FILESYSTEM_SECTOR_6,
    FILESYSTEM_SECTOR_7
};

/* reserve FS_MIN_BLANKSECTORS and active sector to avoid relocation problems */
#if FD_MAX > (FS_NUM_SECTORS - FS_MIN_BLANKSECTORS - 1) * FDS_PER_SECTOR
#error FD_MAX is too high: may led to rellocation problems...
#endif

struct {
    struct timer timer;
    struct st_fileDescriptor fd[FD_MAX];

    struct {
        u32_t address[FS_NUM_SECTORS];
        u8_t sector;
        u16_t fd_index;
    } curr;

    struct {
        u8_t buffer[FS_BLOCK_SIZE];
        struct st_fileDescriptor fd;
        u8_t sector;
        u16_t fd_index;
    } reloc;

    struct pt pt;
} Fs;

#ifdef DEBUG_FSWRITE_SLOW
u16_t Debug_sleep = 10;
#endif

/*****************************************************************************/
#define SECTOR_IS_FULL(sector_number) (Fs.curr.address[sector_number] >= \
    Sector_base[sector_number] + FILESYSTEM_SECTOR_SIZE)

/*****************************************************************************/
#define SECTOR_IS_BLANK(sector_number) \
    (Fs.curr.address[sector_number] == Sector_base[sector_number])

/*****************************************************************************/
#define DEBUG_PBLOCK_THRESHOLD        0
#define DEBUG_PRINT_BLOCK(ram_offset)\
{\
    struct st_blockHeader * p_dbh;\
    u8_t z;\
    p_dbh = (struct st_blockHeader *)ram_offset;\
    dprintf(DEBUG_PBLOCK_THRESHOLD, "\nblock: v=%d s=%d n=%d e=%d ", p_dbh->ver, p_dbh->size, \
        p_dbh->newer, p_dbh->ext);\
    dprintf(DEBUG_PBLOCK_THRESHOLD, ".hash=0x%02x .name=0x%04x\ndata= ", p_dbh->hash, p_dbh->name);\
    for (z = sizeof(struct st_blockHeader); z < FS_BLOCK_SIZE; z++) {\
        dprintf(DEBUG_PBLOCK_THRESHOLD, "0x%02x ", ram_offset[z]);\
    }\
}

#define BLOCK_IS_BLANK -1
#define BLOCK_IS_INVALID -2
#define BLOCK_IS_ILLEGALSIZE -3
#define BLOCK_IS_ERRORED -4
static int checkBlock(u8_t *buffer) {
    struct st_blockHeader * header;
    u8_t hash, newer;
    u32_t crc32;

    /* check header for blank or illegal size */
    if (*(u32_t *)buffer == LONG_BLANK_PATTERN) {
        dprintf(0, "\ncheckBlock(-1): blank block header.");
        return BLOCK_IS_BLANK;
    } else {
        header = (struct st_blockHeader *)buffer;
        if (header->size == FS_SIZEINVALID) {
            dprintf(0, "\ncheckBlock(-2): zero-sized block (invalidated).");
            return BLOCK_IS_INVALID;
        } else if (header->size!= FS_SIZE32) {
            dprintf(0, "\ncheckBlock(-3): block has illegal size.");
            return BLOCK_IS_ILLEGALSIZE;
        }
    }

    /* saves hash and newer before crc calculations */
    hash = header->hash;
    newer = header->newer;
    /* calculations excepts newer and hash fields */
    header->hash = 0;
    header->newer = 0;
    crc32 = crc32_calc(buffer, FS_BLOCK_SIZE, &crc32, CRC32_COMM_ONESHOT);
    /* restores hash and newer fields */
    header->hash = hash;
    header->newer = newer;

    if (hash != (u8_t)crc32) {
        dprintf(1, "\ncheckBlock(-4): Image header hash failed (0x%02x:0x%02x).", hash,
            (u8_t)crc32);
        return BLOCK_IS_ERRORED;
    }

    return 0;
}


/*****************************************************************************/
static u32_t initFileDescriptors(u8_t sector, u32_t *sector_current) {
    u8_t read_buffer[FS_BLOCK_SIZE];
    struct st_blockHeader *p_blockHeader = (struct st_blockHeader *)read_buffer;
    u32_t address = Sector_base[sector];
    int blockCheck;
    u16_t i;

    for (address = Sector_base[sector] ; address < (Sector_base[sector] + FILESYSTEM_SECTOR_SIZE) ;
        address+= FS_BLOCK_SIZE) {
        *sector_current = address;
        flash_read(address, read_buffer, FS_BLOCK_SIZE);
        blockCheck = checkBlock(read_buffer);

        if (blockCheck == 0) {
            dprintf(1, "\ninitFd: addr= 0x%06x. ", address);
            // DEBUG_PRINT_BLOCK(read_buffer);
            dprintf(1, "valid: n=0x%04x e=%d (%d)", p_blockHeader->name, p_blockHeader->ext,
                p_blockHeader->newer);
            for (i = 0 ; i < Fs.curr.fd_index ; i++) {
                if ((Fs.fd[i].header.name == p_blockHeader->name) &&
                    (Fs.fd[i].header.ext == p_blockHeader->ext)) {
                    break;
                }
            }

            /* found an fd with same info as this block */
            if (i < Fs.curr.fd_index) {
                dprintf(1, "\nsame as fd[%d] on 0x%06x : ", i, (&Fs.fd[i])->address);
                dprintf(1, "\nn=0x%04x e=%d (%d)", (&Fs.fd[i])->header.name,
                    (&Fs.fd[i])->header.ext, (&Fs.fd[i])->header.newer);

                if ((Fs.fd[i].header.newer == FS_BLOCK_CURRENT) &&
                    (p_blockHeader->newer == FS_BLOCK_NEWER)) {
                    dprintf(1, "is older, invalidating.");

                    /* invalidate former block (older) */
                    Fs.fd[i].header.size = FS_SIZEINVALID;
                    while (flash_ready() != 1) timer_nopsleep(100);
                    flash_write(Fs.fd[i].address, (u8_t *)&Fs.fd[i].header,
                        sizeof(struct st_blockHeader));

                    /* fd <- new_block info */
                    Fs.fd[i].address = address;
                    Fs.fd[i].sector = sector;
                    memcpy((u8_t *)&Fs.fd[i].header, read_buffer, sizeof(struct st_blockHeader));

                } else if ((Fs.fd[i].header.newer == FS_BLOCK_NEWER) &&
                        (p_blockHeader->newer == FS_BLOCK_CURRENT)) {
                    dprintf(1, "is newer.");

                    /* invalidate new discovered block */
                    p_blockHeader->size = FS_SIZEINVALID;
                    while (flash_ready() != 1) timer_nopsleep(100);
                    flash_write(address, read_buffer, sizeof(struct st_blockHeader));
                } else {
                    dprintf(1, "unexpected. keeping first found (may fallback to older info) ...");

                    /* invalidate new discovered block */
                    p_blockHeader->size = FS_SIZEINVALID;
                    while (flash_ready() != 1) timer_nopsleep(100);
                    flash_write(address, read_buffer, sizeof(struct st_blockHeader));
                }
                // timer_msleep(1000); // debug

            } else {
                if (Fs.curr.fd_index < FD_MAX) {
                    Fs.fd[Fs.curr.fd_index].address = address;
                    Fs.fd[Fs.curr.fd_index].sector = sector;
                    memcpy((u8_t *)&Fs.fd[Fs.curr.fd_index].header, read_buffer,
                        sizeof(struct st_blockHeader));
                    Fs.curr.fd_index++;
                } else {
                    dprintf(1, "\ninitFd (-1): no Fd's available.");
                    return -1;
                }
            }
        } else if (blockCheck == BLOCK_IS_BLANK) {
            if (address == Sector_base[sector]) {
                dprintf(1, "\nblank sector, exiting.");
                return 0;
            } else {
                dprintf(1, "\nwritten sector, exiting.");
                return 1;
            }
        } else {
            dprintf(1, "\ninvalid, continuing search.");
        }
    }

    dprintf(-1, "\nfull sector, exiting.");
    /* Sum FS_BLOCK_SIZE, pointing to:
     * - top sector address or
     * - after a last invalid block */
    *sector_current+= FS_BLOCK_SIZE;
    return 2;
}

/*****************************************************************************/
static void updateBlockHash(u8_t *block_data, u8_t block_size, u8_t size, u8_t newer) {
    struct st_blockHeader * header;
    u32_t crc32;

    /* initialize all header but name and ext(ension)*/
    header = (struct st_blockHeader *)block_data;
    header->ver = FS_VERSION;
    header->size = size;
    header->hash = 0;
    header->newer = 0;

    crc32 = crc32_calc(block_data, block_size, &crc32, CRC32_COMM_ONESHOT);
    header->hash = (u8_t)crc32;
    header->newer = newer;
}

/*****************************************************************************/
static int updateSector(void) {
    u8_t nextSector;

    if (SECTOR_IS_FULL(Fs.curr.sector)) {
        nextSector = Fs.curr.sector;
        do {
            nextSector = (nextSector + 1) % FS_NUM_SECTORS;
            if (!SECTOR_IS_FULL(nextSector)) {
                Fs.curr.sector = nextSector;
                dprintf(2, "\nupdateFsPointer: sector %d is the new active sector", Fs.curr.sector);
                fs_thread(1);
                return 1;
            }
        } while (nextSector!= Fs.curr.sector);

        dprintf(3, "\nupdateFsPointer: all sectors are full, panic.");
        return -1;
    }
    return 0;
}

/*****************************************************************************/
int fs_write(u16_t blockName, u8_t blockExt, const void *block_data, u8_t block_len) {
    struct st_blockHeader *p_blockHeader;
    u8_t write_buffer[FS_BLOCK_SIZE]={0xFF};
    u16_t i;
#ifdef DEBUG_FSWRITE_SLOW
    u8_t j;
#endif

    if (block_len > FS_BLOCK_SIZE - sizeof(struct st_blockHeader)) {
        dprintf(1, "\nfs_write: block size (%d) is too long, max= %ld", block_len,
            FS_BLOCK_SIZE - sizeof(struct st_blockHeader));
        return -1;
    }

    if (updateSector() < 0) {
        dprintf(1, "\nfs_write: filesystem not ready.");
        return -2;
    }

    /* fill write buffer */
    p_blockHeader = (struct st_blockHeader *)write_buffer;
    p_blockHeader->name = blockName;
    p_blockHeader->ext = blockExt;
    memcpy(&write_buffer[sizeof(struct st_blockHeader)], block_data, block_len);

    /* previous block (fd) must have newer== FS_BLOCK_CURRENT prior to operation. */
    for (i=0 ; i < Fs.curr.fd_index ; i++) {
        if ((Fs.fd[i].header.name == p_blockHeader->name) &&
            (Fs.fd[i].header.ext == p_blockHeader->ext)) {
            dprintf(1, "\nfs_write: found fd[%d] with same name and extension. ", i);
            if (Fs.fd[i].header.newer == FS_BLOCK_NEWER) {
                Fs.fd[i].header.newer = FS_BLOCK_CURRENT;
                while (flash_ready() != 1) timer_nopsleep(100);
                flash_write(Fs.fd[i].address, (u8_t *)&Fs.fd[i].header,
                    sizeof(struct st_blockHeader));
                dprintf(1, " now FS_BLOCK_CURRENT.");
            }
            break;
        }
    }

    /* update block hash */
    updateBlockHash(write_buffer, FS_BLOCK_SIZE, FS_SIZE32, FS_BLOCK_NEWER);
    // DEBUG_PRINT_BLOCK(write_buffer); // debug

    /* write to flash */
    dprintf(1, "\nfs_write: writing h=%ld %d(%d) bytes at 0x%06x.",
        sizeof(struct st_blockHeader), block_len, FS_BLOCK_SIZE, Fs.curr.address[Fs.curr.sector]);
    while (flash_ready() != 1) timer_nopsleep(100);
    flash_write(Fs.curr.address[Fs.curr.sector], write_buffer, FS_BLOCK_SIZE);

#ifdef DEBUG_FSWRITE_SLOW
    for (j = 0 ; j < 10 ; j++) {
        timer_msleep(Debug_sleep);
        dprintf(0, "+");
    }
#endif

    /* previous block (fd) with same name/extension: must be invalidated */
    if (i < Fs.curr.fd_index) {
        Fs.fd[i].header.size = FS_SIZEINVALID;
        while (flash_ready() != 1) timer_nopsleep(100);
        flash_write(Fs.fd[i].address, (u8_t *)&Fs.fd[i].header, 1);
    }

#ifdef DEBUG_FSWRITE_SLOW
    for (j = 0 ; j < 10 ; j++) {
        timer_msleep(Debug_sleep);
        dprintf(0, "!");
    }
#endif

    if (i < FD_MAX) {
        dprintf(1, "\nfs_write: updating fd[%d] :", i);
        Fs.fd[i].address = Fs.curr.address[Fs.curr.sector];
        Fs.fd[i].sector = Fs.curr.sector;
        memcpy((u8_t *)&Fs.fd[i].header, write_buffer, sizeof(struct st_blockHeader));

        Fs.fd[i].header.newer = FS_BLOCK_CURRENT;
        while (flash_ready() != 1) timer_nopsleep(100);
        flash_write(Fs.fd[i].address, (u8_t *)&Fs.fd[i].header, 1);
        if (i == Fs.curr.fd_index) {
            dprintf(1, " new.");
            Fs.curr.fd_index++;
        } else {
            dprintf(1, " replaced.");
        }
    } else {
        dprintf(1, "\nfs_write (-1): no fd's available.");
        return -1;
    }

    /* update current pointer */
    Fs.curr.address[Fs.curr.sector] += FS_BLOCK_SIZE;
    return 0;
}

/*****************************************************************************/
int fs_read(u16_t blockName, u8_t blockExt, void *block_data, u8_t block_len) {
    u8_t read_buffer[FS_BLOCK_SIZE];
    u16_t i;

    dprintf(0, "\nfs_read: searching name.ext 0x%04x.%d,", blockName, blockExt);
    for (i = 0 ; i < Fs.curr.fd_index ; i++) {
        if ((Fs.fd[i].header.name == blockName) && (Fs.fd[i].header.ext == blockExt)) {
            dprintf(0, " found at fd[%d]. reading addr=0x%06x, ", i, Fs.fd[i].address);
            while (flash_ready() != 1) timer_nopsleep(100);
            flash_read(Fs.fd[i].address, read_buffer, FS_BLOCK_SIZE);
            break;
        }
    }

    if (i < Fs.curr.fd_index) {
        dprintf(0, " memcpy.\n");
        DEBUG_PRINT_BLOCK(read_buffer);
        memcpy(block_data, &read_buffer[sizeof(struct st_blockHeader)], block_len);
        return 0;
    } else {
        dprintf(0, " not found.\n");
        return -1;
    }
}

/*****************************************************************************/
void fs_init(void) {
    unsigned char i, blankSector = 0, sectorAssigned = 0;
    int sectorSt;
    /* should check for header->type.block and register only one type.block... */
    dprintf(2, "fs_init: initializing.\n");

    for (i = 0 ; i < FS_NUM_SECTORS ; i++) {
        dprintf(2, "fs_init: sector %d at 0x%06x :", i, Sector_base[i]);
        sectorSt = initFileDescriptors(i, &Fs.curr.address[i]);
        if (sectorSt < 0) {
            dprintf(2, " there are fd's not recovered.\n");
        } else if (sectorSt == 0) {
            dprintf(2, " blank sector.\n");
            blankSector = i;
        } else if (sectorSt == 1) {
            dprintf(2, " written sector (our \"target\").\n");
            Fs.curr.sector = i;
            sectorAssigned = 1;
        } else if (sectorSt == 2) {
            dprintf(2, " full sector.\n");
        } else {
            dprintf(2, " unknown \"good\" state %d, ignoring!\n", sectorSt);
        }
    }
    /* decide where next data will go... */
    if (sectorAssigned == 0) {
        Fs.curr.sector = blankSector;
    }

    // Name_fs.active_sector = initActiveSector();

    timer_set(&Fs.timer, 5 * CLOCK_SECOND);
    PT_INIT(&Fs.pt);
}

/*****************************************************************************/
inline static int findRelocSector(u8_t *reloc_sector) {
    u8_t blankSectors = 0;
    u8_t sector, fd_min, reloc = 0;
    u16_t fd;
    struct {
        u8_t fd_count;
        u8_t full;
    } reloc_map[FS_NUM_SECTORS] = {{0}};

    /* first we look for full and blank sectors */
    for (sector = 0; sector < FS_NUM_SECTORS ; sector++) {
        dprintf(1, "\nfindRelocSector: sector %d ", sector);
        if (sector == Fs.curr.sector) {
            /* current sector should not be relocated. */
            dprintf(1, "is the current sector.");

        } else if (SECTOR_IS_BLANK(sector)) {
            dprintf(1, "is blank.");
            blankSectors++;
            if (blankSectors == FS_MIN_BLANKSECTORS) {
                dprintf(0, "\nfindRelocSector: blankSectors >= FS_MIN_BLANKSECTORS, don't need "
                    "reloc yet.");
                return -1;
            }
        } else if (SECTOR_IS_FULL(sector)) {
            dprintf(1, "is full.");
            reloc_map[sector].full = 1;
        }
    }

    /* make a histogram crossing full sector and active fd's info */
    for (fd = 0; fd < Fs.curr.fd_index ; fd++) {
        dprintf(1, "\nfindRelocSector: fd%d at sector %d", fd, Fs.fd[fd].sector);
        if (reloc_map[Fs.fd[fd].sector].full == 1) {
            dprintf(1, "++");
            reloc_map[Fs.fd[fd].sector].fd_count++;
        }
    }
    /* reloc "winner" will be the full sector with less active fd's */
    fd_min = FDS_PER_SECTOR;
    for (sector = 0; sector < FS_NUM_SECTORS ; sector++) {
        dprintf(1, "\nfindRelocSector: sector %d has %d fds (%d)", sector,
            reloc_map[sector].fd_count, reloc_map[sector].full);
        if (reloc_map[sector].full == 1 && reloc_map[sector].fd_count < fd_min) {
            dprintf(1, "(new reloc candidate).");
            fd_min = reloc_map[sector].fd_count;
            reloc = sector;
        }
    }

    *reloc_sector = reloc;
    return 0;
}

/*****************************************************************************/
PT_THREAD(fs_thread(u8_t reloc_flag)) {
    PT_BEGIN(&Fs.pt);

    PT_WAIT_UNTIL(&Fs.pt, reloc_flag == 1 || timer_expired(&Fs.timer));
    PT_YIELD(&Fs.pt);

    if (findRelocSector(&Fs.reloc.sector) < 0) {
        dprintf(2, "\nfs_thread: findRelocSector() says we are ok.");
        timer_restart(&Fs.timer);
        PT_RESTART(&Fs.pt);
    }

    dprintf(2, "\nfs_thread: initiating relocation at sector %d. (%d)",
        Fs.reloc.sector, reloc_flag == 1);

    for (Fs.reloc.fd_index = 0; Fs.reloc.fd_index < Fs.curr.fd_index;
        Fs.reloc.fd_index++) {
        /* fd points to an address within sector to be relocated */
        if (Fs.fd[Fs.reloc.fd_index].sector == Fs.reloc.sector) {
            dprintf(2, "\nfs_thread: reading fd[%d] (0x%06x) for relocation.",
                Fs.reloc.fd_index, Fs.fd[Fs.reloc.fd_index].address);
            /* keep track of fd address and bufferize block */
            memcpy(&Fs.reloc.fd, &Fs.fd[Fs.reloc.fd_index],
                sizeof(struct st_fileDescriptor));
            PT_WAIT_WHILE(&Fs.pt, flash_ready() != 1);
            flash_read(Fs.reloc.fd.address, Fs.reloc.buffer, FS_BLOCK_SIZE);

#ifdef DEBUG_FSWRITE_SLOW
            Debug_sleep = 200;  // makes fs_write() even slower on relocation...
#endif

#ifdef DEBUG_WRITE_DURING_RELOC
            dprintf(2, "Waiting between fs_read() and fs_write(): ");
            timer_set(&Fs.timer, 2 * CLOCK_SECOND);
            PT_WAIT_UNTIL(&Fs.pt, timer_expired(&Fs.timer));
            dprintf(2, "-");
            timer_set(&Fs.timer, 2 * CLOCK_SECOND);
            PT_WAIT_UNTIL(&Fs.pt, timer_expired(&Fs.timer));
            dprintf(2, "-");
            timer_set(&Fs.timer, 2 * CLOCK_SECOND);
            PT_WAIT_UNTIL(&Fs.pt, timer_expired(&Fs.timer));
            dprintf(2, "-");
            timer_set(&Fs.timer, 5 * CLOCK_SECOND);
#else
            PT_YIELD(&Fs.pt);
#endif

            /* if this block stills at same place rellocation is needed --
             * assumes static location for fd's
             */
            if (Fs.fd[Fs.reloc.fd_index].address== Fs.reloc.fd.address) {
                dprintf(2, "\nfs_thread: writing fd[%d] from 0x%06x on active "
                    "sector%d 0x%06x.", Fs.reloc.fd_index,
                    Fs.fd[Fs.reloc.fd_index].address,
                    Fs.curr.sector, Fs.curr.address[Fs.curr.sector]);
                while (fs_write(Fs.reloc.fd.header.name, Fs.reloc.fd.header.ext,
                        &Fs.reloc.buffer[sizeof(struct st_blockHeader)],
                        FS_BLOCK_SIZE - sizeof(struct st_blockHeader)) < 0) {
                    PT_YIELD(&Fs.pt);
                }
            } else {
                dprintf(2, "\nfs_thread: fd[%d] (0x%06x) has been writen after "
                    "reading -- relocation aborted.", Fs.reloc.fd_index,
                    Fs.fd[Fs.reloc.fd_index].address);
            }
            PT_YIELD(&Fs.pt);
        }
    }

    /* By now all fd's should be relocated -- erase sector */
    PT_WAIT_WHILE(&Fs.pt, flash_ready() != 1);
    dprintf(2, "\nfs_thread: erasing sector %d (0x%06x).", Fs.reloc.sector,
        Sector_base[Fs.reloc.sector]);
    flash_erase(Sector_base[Fs.reloc.sector]);

    /* Initialize sector current pointer */
    Fs.curr.address[Fs.reloc.sector]= Sector_base[Fs.reloc.sector];
    timer_restart(&Fs.timer);

    PT_END(&Fs.pt);
}
