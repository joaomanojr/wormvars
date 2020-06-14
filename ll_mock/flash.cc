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
void flash_erase(unsigned int sector_address)
{
    Flash->erase(sector_address);
}

/*****************************************************************************/
int flash_ready(void)
{
    return 1;
}
