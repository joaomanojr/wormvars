/* add some microcontroller crc32 functions to get rid of initial build problems */

#define CRC32_COMM_ONESHOT 0

#ifdef __cplusplus
extern "C" {
#endif

unsigned int crc32_calc(void * buffer, unsigned int size, unsigned int * crc32_0, int operation);

#ifdef __cplusplus
}
#endif