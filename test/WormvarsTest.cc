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


TEST_F(WormvarsTest, MassiveWriteAndRead) {
    unsigned char buffer_in[16];
    unsigned char buffer_out[16];
    u16_t block1_name = 0x0100;
    u16_t block1_ext = 0;

    /* Create 64 variables with different contents */
    for (auto offset = 0; offset < 64 ; offset++) {
        u16_t var_name = block1_name + offset;
        for (auto i = 0; i < sizeof(buffer_in); i++)
            buffer_in[i] = offset + i;
        EXPECT_EQ(fs_write(var_name, block1_ext, buffer_in, sizeof(buffer_in)), 0);
    }

    flash->print_sector_map();

    /* check 1217... */
    /* Write 16 first variables in a product of 16 -- will bump other vars around */
    for (auto bump = 0; bump < 2048 ; bump++) {
        u16_t var_name = block1_name + (bump % 16);
        memset(buffer_in, static_cast<unsigned char>(bump), sizeof(buffer_in));
        EXPECT_EQ(fs_write(var_name, block1_ext, buffer_in, sizeof(buffer_in)), 0);

        flash->print_sector_map();
        cout << flash->get_erase_count() << "erases issued." << endl;

        /* This is just a naive initial guess on relocation needs as each sector is
         * filled (128 offsets * 32 bytes = 4096 bytes sector)
         */
        if (bump % 128 == 0) {
            fs_thread(0);
            cout << "bump is " << bump << endl;
        }
    }
#if 0
    /* Check contents of first 16 variables */
    for (auto offset = 0; offset < 16; offset++) {
        u16_t var_name = block1_name + offset;
        u8_t expected = static_cast<unsigned char>(1217) - 16 + offset;

        EXPECT_EQ(fs_read(var_name, block1_ext, buffer_out, sizeof(buffer_out)), 0);
        for (auto i = 0; i < sizeof(buffer_out); i++)
            EXPECT_EQ(buffer_out[i], expected);
    }
#endif

#if 0
    /* The remainder 48 variables will hold its values after all these relocations */
    for (auto offset = 16; offset < 64 ; offset++) {
        u16_t var_name = block1_name + offset;
        for (auto i = 0; i < sizeof(buffer_in); i++)
            buffer_in[i] = offset + i;

        EXPECT_EQ(fs_read(var_name, block1_ext, buffer_out, sizeof(buffer_out)), 0);
        for (auto i = 0; i < sizeof(buffer_out); i++)
            EXPECT_EQ(buffer_in[i], buffer_out[i]);
    }
#endif

    cout << "--- Test statistics ---" << endl;
    cout << flash->get_write_count() << " FLASH writes." << endl;
    cout << flash->get_read_count() << " FLASH reads." << endl;
    cout << flash->get_erase_count() << " FLASH erases." << endl;

    flash->print_sector_map();

#if 0
    /* Reboot */
    fs_init();

    /* Our 64 variables must be there with same values */
    for (auto offset = 0; offset < 16; offset++) {
        u16_t var_name = block1_name + offset;
        u8_t expected = static_cast<unsigned char>(1217) - 16 + offset;

        EXPECT_EQ(fs_read(var_name, block1_ext, buffer_out, sizeof(buffer_out)), 0);
        for (auto i = 0; i < sizeof(buffer_out); i++)
            EXPECT_EQ(buffer_out[i], expected);
    }
    for (auto offset = 16; offset < 64 ; offset++) {
        u16_t var_name = block1_name + offset;
        for (auto i = 0; i < sizeof(buffer_in); i++)
            buffer_in[i] = offset + i;

        EXPECT_EQ(fs_read(var_name, block1_ext, buffer_out, sizeof(buffer_out)), 0);
        for (auto i = 0; i < sizeof(buffer_out); i++)
            EXPECT_EQ(buffer_in[i], buffer_out[i]);
    }

    cout << "--- Test statistics after 'reboot' ---" << endl;
    cout << flash->get_write_count() << " FLASH writes." << endl;
    cout << flash->get_read_count() << " FLASH reads." << endl;
    cout << flash->get_erase_count() << " FLASH erases." << endl;

    flash->print_sector_map();
#endif
}