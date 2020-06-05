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