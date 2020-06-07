/* Function prototypes mocking actual FLASH device */

#ifdef __cplusplus
extern "C" {
#endif

void *flash_init(void);
void flash_finish(void);

void flash_read(unsigned int address, void *buffer, unsigned int size);
void flash_write(unsigned int address, void *buffer, unsigned int size);
void flash_erase(unsigned int address, unsigned int num_sectors);
int flash_ready(void);

#ifdef __cplusplus
}
#endif
