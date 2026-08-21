#include <gtest/gtest.h>

#include <CryptoService.h>

TEST(TestCryptoService, ConstructorNotImplemented_Success) {
  EXPECT_THROW(CryptoService service{};, std::runtime_error);
}
