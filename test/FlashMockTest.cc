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

#define FLASH_SECTOR_TO_ADDR(sector) (FLASHMOCK_FIRSTSECTOR_ADDRESS + \
    sector * FLASHMOCK_SECTOR_SIZE)

// The fixture for testing class FlashMock
class FlashMockTest : public ::testing::Test {
 protected:
    // You can do set-up work for each test here.
    FlashMockTest();

    // You can do clean-up work that doesn't throw exceptions here.
    virtual ~FlashMockTest();

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

// using ::testing::Return;


FlashMockTest::FlashMockTest() {
}

FlashMockTest::~FlashMockTest() {
}

void FlashMockTest::SetUp() {
    flash = static_cast<FlashMock *>(flash_init());
}

void FlashMockTest::TearDown() {
    flash_finish();
}

TEST_F(FlashMockTest, BasicAccess) {
    const unsigned int first_offset = FLASH_SECTOR_TO_ADDR(1) + 128;
    const unsigned int second_offset = FLASH_SECTOR_TO_ADDR(2) + 64;
    unsigned char test_in[32] = {0, 1, 2, 3, 4};
    unsigned char dummy_buffer[32] = {0};

    /* Read and write on a fixed offset */
    flash->write(first_offset, test_in, 5);
    flash->read(first_offset, dummy_buffer, 5);
    EXPECT_EQ(flash->get_write_count(), 1);
    EXPECT_EQ(flash->get_read_count(), 1);
    for (auto i = 0; i < 5; i++)
        EXPECT_EQ(dummy_buffer[i], test_in[i]);

    /* Read an not previously written offset must return blank (0xFF) */
    flash->read(second_offset, dummy_buffer, sizeof(dummy_buffer));
    EXPECT_EQ(flash->get_read_count(), 2);
    for (auto i = 0; i < sizeof(dummy_buffer); i++)
        EXPECT_EQ(dummy_buffer[i], 0xFF);
}

TEST_F(FlashMockTest, EraseMidSector) {
    const unsigned int first_offset = FLASH_SECTOR_TO_ADDR(1);
    const unsigned int second_offset = FLASH_SECTOR_TO_ADDR(2) + 64;
    const unsigned int third_offset = FLASH_SECTOR_TO_ADDR(3) + 128;
    const unsigned int fourth_offset = FLASH_SECTOR_TO_ADDR(4) + 64;
    unsigned char test_in[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    unsigned char dummy_buffer[32] = {0};

    /* Read and write on sectors 1..4 */
    flash->write(first_offset, test_in, 5);
    flash->write(second_offset, &test_in[2], 6);
    flash->write(third_offset, &test_in[1], 2);
    flash->write(fourth_offset, &test_in[4], 4);
    EXPECT_EQ(flash->get_write_count(), 4);

    /* Erase sectors 2 and 3 */
    flash->erase(second_offset, 2);
    EXPECT_EQ(flash->get_erase_count(), 2);

    /* Sectors 2 and 3 are erased */
    flash->read(second_offset, dummy_buffer, 6);
    for (auto i = 0; i < 6; i++)
        EXPECT_EQ(dummy_buffer[i], 0xFF);
    flash->read(third_offset, dummy_buffer, 2);
    for (auto i = 0; i < 2; i++)
        EXPECT_EQ(dummy_buffer[i], 0xFF);

    /* Sectors 1 and 4 must not be affected */
    flash->read(first_offset, dummy_buffer, 5);
    for (auto i = 0; i < 5; i++)
        EXPECT_EQ(dummy_buffer[i], test_in[i]);
    flash->read(fourth_offset, dummy_buffer, 4);
    for (auto i = 0; i < 4; i++)
        EXPECT_EQ(dummy_buffer[i], test_in[i+4]);
}

TEST_F(FlashMockTest, RewriteOffset) {
    const unsigned int offset = FLASH_SECTOR_TO_ADDR(2) + 64;
    unsigned char test_in0[] = {0xFF, 0xF0, 0x0F, 0x00};
    unsigned char test_in1[] = {0x55, 0xAA, 0xBB, 0xFF};
    unsigned char dummy_buffer[4] = {0};

    /* write some data on offset, then write again new data on same offset. */
    flash->write(offset, test_in0, 4);
    flash->write(offset, test_in1, 4);

    /* read data corresponds to zeroed bits on the two patterns: saying another way an AND mask */
    flash->read(offset, dummy_buffer, 4);
    for (auto i = 0; i < 4; i++)
        EXPECT_EQ(dummy_buffer[i], test_in0[i] & test_in1[i]);
}
