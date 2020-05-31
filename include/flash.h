/* add some microcontroller flash functions to get rid of initial build problems */

static void flash_read(unsigned int address, void *buffer, unsigned int size) { }
static void flash_write(unsigned int address, void *buffer, unsigned int size) { }
static void flash_erase(unsigned int sector, unsigned int nun_sectors) { }

static int flash_ready(void) {
    return 1;
}
