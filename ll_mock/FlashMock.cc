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

#include <iostream>
#include <iomanip>
#include <cstring>
#include "FlashMock.h"

using namespace std;

FlashMock::FlashMock() : read_count(0), write_count(0), erase_count(0) {
    cout << "This is FlashMock Constructor" << endl;
}

FlashMock::~FlashMock() {
    cout << "This is FlashMock Destructor" << endl;
}

bool FlashMock::address_to_sector(unsigned int address, int *sector) {
    int l_sector;

    address -= FLASHMOCK_FIRSTSECTOR_ADDRESS;
    l_sector = address / FLASHMOCK_SECTOR_SIZE;

    if (l_sector >= FLASHMOCK_NUMSECTORS)
        return false;

    *sector = l_sector;
    return true;
}

void FlashMock::read(unsigned int address, void *buffer, unsigned int size) {
    int sector;

    if (!address_to_sector(address, &sector)) {
        cout << "flash_read() sector is out of range" << sector << endl;
        return;
    }

    read_count++;

    char *c_buffer = static_cast<char *>(buffer);
    for (auto &chunk : flash[sector]) {
        if (chunk.address == address) {
            for (auto i = 0; i < size; i++)
                if (i < chunk.buffer.size())
                    *c_buffer++ = chunk.buffer[i];
                else
                    *c_buffer++ = 0xFF;
            return;
        }
    }
    // If not found return 'clean' chunk
    memset(buffer, 0xFF, size);
}

void FlashMock::write(unsigned int address, void *buffer, unsigned int size) {
    int sector;

    if (size > FLASHMOCK_BUFFER_CHUNK_MAX) {
        cout << "ERROR: flash_write() size " << size << " is too big." << endl;
        return;
    }

    if (!address_to_sector(address, &sector)) {
        cout << "ERROR: flash_write() sector is out of range" << sector << endl;
        return;
    }

    if (flash[sector].size() > FLASHMOCK_CHUNK_MAX) {
        cout << "ERROR: flash_write() maximum chunk hit, aborting" << endl;
        return;
    }

    write_count++;

    char *c_buffer = static_cast<char *>(buffer);
    for (auto &chunk : flash[sector]) {
        if (chunk.address == address) {
            /* Must reuse existing chunk */
            for (auto i = 0; i < size; i++)
                if (i < chunk.buffer.size())
                    /* This mocks FLASH behaviour more accurately:
                     * we can only write zeroes on an already written offset.
                     */
                    chunk.buffer[i] &= *c_buffer++;
                else
                    chunk.buffer.push_back(*c_buffer++);
            return;
        }
    }

    /* Create a new chunk */
    struct flash_chunk new_chunk;
    new_chunk.address = address;
    for (auto i = 0; i < size; i++)
        if (i < new_chunk.buffer.size())
            new_chunk.buffer[i] = *c_buffer++;
        else
            new_chunk.buffer.push_back(*c_buffer++);

    flash[sector].push_back(new_chunk);
}

void FlashMock::erase(unsigned int sector_address) {
    int sector;

    if (!address_to_sector(sector_address, &sector)) {
        cout << "ERROR: flash_erase() sector is out of range: " << sector << endl;
        return;
    }

    flash[sector].clear();
    erase_count++;
}

// Introspection functions
int FlashMock::get_read_count() {
    return read_count;
}

int FlashMock::get_write_count() {
    return write_count;
}

int FlashMock::get_erase_count() {
    return erase_count;
}

void FlashMock::print_sector_map() {
    cout << "-------------------------------------------------" << endl;
    cout << "|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |" << endl;
    cout << "-------------------------------------------------" << endl;
    for (auto &it : flash)
        cout << "| " << setw(3) << it.size() << " ";
    cout << "|" << endl;
    cout << "-------------------------------------------------" << endl;
}