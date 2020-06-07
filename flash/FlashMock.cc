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

    // cout << "flash_read() sector is " << sector << endl;
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
    // If not found return clean chunk
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
    // cout << "flash_write() sector is " << sector << endl;

    if (flash[sector].size() > FLASHMOCK_CHUNK_MAX) {
        cout << "ERROR: flash_write() maximum chunk hit, aborting" << endl;
        return;
    }

    write_count++;

    struct flash_chunk this_write;
    char *c_buffer = static_cast<char *>(buffer);
    this_write.address = address;
    for (auto i = 0; i < size; i++)
        if (i < this_write.buffer.size())
            this_write.buffer[i] = *c_buffer++;
        else 
            this_write.buffer.push_back(*c_buffer++);

    flash[sector].push_back(this_write);

    // cout << "flash_write(" << address << ", " << buffer << ", " << size << ")" << endl;
}

void FlashMock::erase(unsigned int address, unsigned int num_sectors) {
    int erase_begin;

    if (!address_to_sector(address, &erase_begin)) {
        cout << "ERROR: flash_erase() sector is out of range: " << erase_begin << endl;
        return;
    }

    int erase_end = erase_begin + num_sectors;
    if (erase_end >= FLASHMOCK_NUMSECTORS)
        erase_end = FLASHMOCK_NUMSECTORS - 1;

    for (auto i = erase_begin; i < erase_end; i++) {
        flash[i].clear();
        erase_count++;
    }

    // cout << "flash_erase(" << address << ", " << num_sectors << ")" << endl;
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