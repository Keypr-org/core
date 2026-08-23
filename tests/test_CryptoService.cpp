#include <gtest/gtest.h>

#include <CryptoService.h>

TEST(TestCryptoService, Constructor_Success) {
  EXPECT_NO_THROW(CryptoService service{});
}
