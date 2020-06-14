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

/**
 * @file FlashMock.cc - a simple FLASH device mocking mechanism.
 * 
 * @brief This class mocks FLASH device using vectors to keep track of data written and allowing
 *        user to read it back.
 * 
 *        It will respond to erase commands just re-initializing vectors keeping information on data
 *        written.
 * 
 *        Possible overlaps, write above high sector addresses or even replaces on data written is
 *        not taken into account and behaviour on these cases are not tested/guaranteed.
 */

#include <vector>
#include <string>

using namespace std;

/*
 * TODO(joao): This is plain hardcode, later on geometry information may be passed on initialization
 */
#define FLASHMOCK_NUMSECTORS 8
#define FLASHMOCK_FIRSTSECTOR_ADDRESS 0x0F4000
#define FLASHMOCK_SECTOR_SIZE 0x1000
#define FLASHMOCK_CHUNK_MAX 256
#define FLASHMOCK_BUFFER_CHUNK_MAX 256

struct flash_chunk {
    unsigned int address;
    vector<char> buffer;
};

class FlashMock  {

 public:
    FlashMock();
    ~FlashMock();

    /*
     * Read flash function
     * 
     * @param[in]  address FLASH address to be read from
     * @param[out] buffer  pointer to read data destination
     * @param[in]  size    number of bytes to read
     */
    void read(unsigned int address, void *buffer, unsigned int size);
    /*
     * Write flash function
     * 
     * @param[in] address FLASH address to be read from
     * @param[in] buffer  pointer from to write data origin
     * @param[in] size    number of data bytes to write
     */
    void write(unsigned int address, void *buffer, unsigned int size);
    /*
     * Erase sector at address
     * 
     * @param[in] sector_address  FLASH base address corresponding to be erased
     */
    void erase(unsigned int sector_address);
    /*
     * Introspection functions - used to monitor FLASH use statistics.
     */
    int get_read_count();
    int get_write_count();
    int get_erase_count();
    void print_sector_map();

 private:

    bool address_to_sector(unsigned int address, int *sector);

    int read_count;
    int write_count;
    int erase_count;

    vector<struct flash_chunk> flash[FLASHMOCK_NUMSECTORS];
};