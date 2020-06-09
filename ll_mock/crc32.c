/* CRC32 used here never detect any error on data written to FLASH. 
 *
 * This is unsafe for use in real life hardware that can be powered off anytime during FLASH write
 * accesses - please use some implementation here like one provided by zlib or a microcontroler
 * system peripheral.
 */

unsigned int crc32_calc(void * buffer, unsigned int size, unsigned int * crc32_0, int operation) {
    return 0;
}
