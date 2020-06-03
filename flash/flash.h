/* Function prototypes mocking actual FLASH device */

void flash_read(unsigned int address, void *buffer, unsigned int size);
void flash_write(unsigned int address, void *buffer, unsigned int size);
void flash_erase(unsigned int sector, unsigned int nun_sectors);
int flash_ready(void);
