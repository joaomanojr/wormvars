/* flash mocking functions */
#include "FlashMock.h"
#include "flash.h"

FlashMock *Flash;

/*****************************************************************************/
void *flash_init(void)
{
    Flash = new FlashMock;
    return Flash;
}

/*****************************************************************************/
void flash_finish(void)
{
    delete Flash;
}

/*****************************************************************************/
void flash_read(unsigned int address, void *buffer, unsigned int size)
{
    Flash->read(address, buffer, size);
}

/*****************************************************************************/
void flash_write(unsigned int address, void *buffer, unsigned int size)
{
    Flash->write(address, buffer, size);
}

/*****************************************************************************/
void flash_erase(unsigned int address, unsigned int num_sectors)
{
    Flash->erase(address, num_sectors);
}

/*****************************************************************************/
int flash_ready(void)
{
    return 1;
}
