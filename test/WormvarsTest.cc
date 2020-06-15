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
#include <algorithm>
#include "gtest/gtest.h"
#include "FlashMock.h"
#include "flash.h"
#include "wormvars.h"

// The fixture for testing wormvars
class WormvarsTest : public ::testing::Test {
 protected:
    // You can do set-up work for each test here.
    WormvarsTest();

    // You can do clean-up work that doesn't throw exceptions here.
    virtual ~WormvarsTest();

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    // Code here will be called immediately after the constructor (right
    // before each test).
    virtual void SetUp();

    // Code here will be called immediately after each test (right
    // before the destructor).
    virtual void TearDown();

    FlashMock *flash;
};

WormvarsTest::WormvarsTest() {
    flash = static_cast<FlashMock *>(flash_init());
    fs_init();
}

WormvarsTest::~WormvarsTest() {
    flash_finish();
}

void WormvarsTest::SetUp() {
}

void WormvarsTest::TearDown() {
}

TEST_F(WormvarsTest, WriteAndRead) {
    u16_t block1_name = 0x0100;
    u16_t block1_ext = 3;
    char buffer_in[10], buffer_out[10];

    /* Write a string (and its \0) onto variable and read it back */
    snprintf(buffer_in, sizeof(buffer_in), "%s", "wormvars");
    EXPECT_EQ(fs_write(block1_name, block1_ext, buffer_in, strlen(buffer_in)+1), 0);
    EXPECT_EQ(fs_read(block1_name, block1_ext, buffer_out, strlen(buffer_in)+1), 0);
    cout << "buffer_out is " << buffer_out << endl;
    EXPECT_EQ(strcmp(buffer_in, buffer_out), 0);

    /* Update string onto variable and read updated value */
    snprintf(buffer_in, sizeof(buffer_in), "%s", "test");
    EXPECT_EQ(fs_write(block1_name, block1_ext, buffer_in, strlen(buffer_in)+1), 0);
    EXPECT_EQ(fs_read(block1_name, block1_ext, buffer_out, strlen(buffer_in)+1), 0);
    cout << "buffer_out is " << buffer_out << endl;
    EXPECT_EQ(strcmp(buffer_in, buffer_out), 0);
    fs_thread(0);

    /* Verify that variable is retrieved after 'reboot' also */
    fs_init();
    EXPECT_EQ(fs_read(block1_name, block1_ext, buffer_out, strlen(buffer_in)+1), 0);
    cout << "'reboot' and buffer_out is " << buffer_out << endl;
    EXPECT_EQ(strcmp(buffer_in, buffer_out), 0);
}


#define kVarLength 28
#if kVarLength > 28
#error kVarLength must be 28 maximum: 32 bytes cell, header overhead is 4
#endif

#define kVarTotal 64
#define kVarBumped 16
#if kVarBumped > kVarTotal
#error kVarBumped must be less or equal of total variables
#endif

/* This is just a naive initial guess on relocation needs as each sector is filled
 * (128 offsets * 32 bytes = 4096 bytes sector). Notice that values greater than that may
 * result on stavation since currently only one wormed sector is freed at a time...
 */
#define kFsThreadBumps 128   // Number of bumps until call fs_thread()
#define kNumberOfBumps 10000

TEST_F(WormvarsTest, MassiveWriteAndRead) {
    u16_t block1_name = 0x0100;
    u16_t block1_ext = 0;
    vector <string> variables;

    /* https://stackoverflow.com/questions/440133/\
     * how-do-i-create-a-random-alpha-numeric-string-in-c
     */
    auto randchar = []() -> char {
        const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };

    string new_variable(kVarLength, 0);
    auto randvar = [=](string &new_variable) {
        std::generate_n(new_variable.begin(), kVarLength, randchar);
    };

    /* Create kVarTotal variables with different contents */
    for (auto offset = 0; offset < kVarTotal ; offset++) {
        u16_t var_name = block1_name + offset;
        // create randomic content
        randvar(new_variable);
        variables.push_back(new_variable);
        EXPECT_EQ(fs_write(var_name, block1_ext, new_variable.c_str(), kVarLength), 0);
    }

    // flash->print_sector_map();

    for (auto bump = 0; bump < kNumberOfBumps; bump++) {
        u16_t var_name = block1_name + (bump % kVarBumped);
        // create randomic content and overwrite prior generated contents
        randvar(new_variable);
        variables[bump % kVarBumped] = new_variable;
        EXPECT_EQ(fs_write(var_name, block1_ext, new_variable.c_str(), kVarLength), 0);

        // flash->print_sector_map();

        if (bump % kFsThreadBumps == 0) {
            fs_thread(0);
            // cout << "bump is " << bump << endl;
        }
    }

    /* Check contents of all kVarTotal variables */
    for (auto offset = 0; offset < kVarTotal; offset++) {
        u16_t var_name = block1_name + offset;
        char buffer_out[kVarLength];

        EXPECT_EQ(fs_read(var_name, block1_ext, buffer_out, kVarLength), 0);

        string read_data(buffer_out, kVarLength);
        EXPECT_EQ(read_data, variables[offset]);
    }

    cout << "--- Test statistics ---" << endl;
    cout << flash->get_write_count() << " FLASH writes." << endl;
    cout << flash->get_read_count() << " FLASH reads." << endl;
    cout << flash->get_erase_count() << " FLASH erases." << endl;

    flash->print_sector_map();

    /* Reboot */
    fs_init();

    /* Check contents of kVarTotal variables */
    for (auto offset = 0; offset < kVarTotal; offset++) {
        u16_t var_name = block1_name + offset;
        char buffer_out[kVarLength];

        EXPECT_EQ(fs_read(var_name, block1_ext, buffer_out, kVarLength), 0);

        string read_data(buffer_out, kVarLength);
        EXPECT_EQ(read_data, variables[offset]);
    }

    cout << "--- Test statistics after 'reboot' ---" << endl;
    cout << flash->get_write_count() << " FLASH writes." << endl;
    cout << flash->get_read_count() << " FLASH reads." << endl;
    cout << flash->get_erase_count() << " FLASH erases." << endl;

    flash->print_sector_map();
}
