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

FlashMock::FlashMock() {
    cout << "This is FlashMock Constructor" << endl;
}

FlashMock::~FlashMock() {
    cout << "This is FlashMock Destructor" << endl;
}

void FlashMock::read(unsigned int address, void *buffer, unsigned int size)
{
    cout << "flash_read(" << address << ", " << buffer << ", " << size << ")" << endl;
    // A clean flash
    memset(buffer, 0xFF, size);
    read_count++;
}

void FlashMock::write(unsigned int address, void *buffer, unsigned int size) {
    cout << "flash_write(" << address << ", " << buffer << ", " << size << ")" << endl;
    write_count++;
}

void FlashMock::erase(unsigned int sector, unsigned int num_sectors) {
    cout << "flash_erase(" << sector << ", " << num_sectors << ")" << endl;
    erase_count++;
}

// Introspection functions
int FlashMock::get_read_count() {
    return read_count;
}
int FlashMock::get_write_count() {
    return write_count;
}
int FlashMock::get_erase_count(){
    return erase_count;
}