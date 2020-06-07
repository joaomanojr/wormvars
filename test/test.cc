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

protected:
    FlashMock *flash;
};

// using ::testing::Return;


FlashMockTest::FlashMockTest() {
    flash = static_cast<FlashMock *>(flash_init());

    std::cout << "FlashMock initialized " << flash->get_read_count() << " reads."<< std::endl;
}

FlashMockTest::~FlashMockTest() {
    flash_finish();
}

void FlashMockTest::SetUp() {}

void FlashMockTest::TearDown() {}

TEST_F(FlashMockTest, FlashMockTestBasicAccess) {

    unsigned char test_in[32] = {0, 1, 2, 3, 4};
    unsigned char dummy_buffer[32] = {0};

    EXPECT_EQ(flash->get_read_count(), 0);

    flash->write(0x0F4000, test_in, 5);
    flash->read(0x0F4000, dummy_buffer, 5);
    EXPECT_EQ(flash->get_read_count(), 1);

    for (auto i = 0; i < 5; i++)
        EXPECT_EQ(dummy_buffer[i], test_in[i]);

    flash->read(0x0F5000, dummy_buffer, sizeof(dummy_buffer));
    EXPECT_EQ(flash->get_read_count(), 2);

    for (auto i = 0; i < sizeof(dummy_buffer); i++)
        EXPECT_EQ(dummy_buffer[i], 0xFF);

    flash->erase(0x0F4000, 2);
    flash->read(0x0F4000, dummy_buffer, 5);
    EXPECT_EQ(flash->get_read_count(), 3);

    for (auto i = 0; i < 5; i++)
        EXPECT_EQ(dummy_buffer[i], 0xFF);
}
