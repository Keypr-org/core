#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "VaultHeader.h"
#include <algorithm>
#include <sodium.h>

class VaultHeaderTest : public ::testing::Test {
  protected:
    std::array<uint8_t, VAULT_HEADER_BYTES> testData;

    void SetUp() override {
        // Initialize testData with valid values
        std::string magicString = VAULT_MAGIC_BYTES;
        std::copy(magicString.begin(), magicString.end(), testData.begin());
        uint32_t formatVersion = VAULT_FORMAT_CURRENT_VERSION;
        std::copy(reinterpret_cast<uint8_t*>(&formatVersion),
                  reinterpret_cast<uint8_t*>(&formatVersion) + sizeof(formatVersion),
                  testData.begin() + FORMAT_VERSION_OFFSET);
        std::array<uint8_t, ARGON2_SALT_SIZE> argon2Salt;
        randombytes_buf(argon2Salt.data(), argon2Salt.size());
        std::copy(argon2Salt.begin(), argon2Salt.end(), testData.begin() + ARGON2_SALT_OFFSET);
        uint64_t argon2OpLimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
        std::copy(reinterpret_cast<uint8_t*>(&argon2OpLimit),
                  reinterpret_cast<uint8_t*>(&argon2OpLimit) + sizeof(argon2OpLimit),
                  testData.begin() + ARGON2_OPSLIMIT_OFFSET);
        uint64_t argon2MemLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;
        std::copy(reinterpret_cast<uint8_t*>(&argon2MemLimit),
                  reinterpret_cast<uint8_t*>(&argon2MemLimit) + sizeof(argon2MemLimit),
                  testData.begin() + ARGON2_MEMLIMIT_OFFSET);
    }
};

// ------------- TESTS --------------------
/*
 * Data that represents a valid vault does not throw and is correctly parsed
 */
TEST_F(VaultHeaderTest, ValidDataParsesCorrectly) {
    EXPECT_NO_THROW({
        VaultHeader header = VaultHeader::parse(testData);
        EXPECT_EQ(header.formatVersion(), VAULT_FORMAT_CURRENT_VERSION);
        EXPECT_THAT(
            header.argon2Salt(),
            ::testing::ElementsAreArray(testData.begin() + ARGON2_SALT_OFFSET,
                                        testData.begin() + ARGON2_SALT_OFFSET + ARGON2_SALT_SIZE));
        EXPECT_EQ(header.argon2OpLimit(), crypto_pwhash_OPSLIMIT_INTERACTIVE);
        EXPECT_EQ(header.argon2MemLimit(), crypto_pwhash_MEMLIMIT_INTERACTIVE);
    });
}

/*
 * Data that does not contain header magic numbers throws a VaultHeaderParsingError
 */
TEST_F(VaultHeaderTest, InvalidMagicBytesThrows) {
    testData[MAGIC_BYTES_OFFSET] = 0x00; // Corrupt the magic bytes
    EXPECT_THROW(VaultHeader::parse(testData), VaultHeaderParsingError);
}

/*
 * Data that contains an unknown format version throws a VaultHeaderParsingError
 */
TEST_F(VaultHeaderTest, UnknownFormatVersionThrows) {
    uint32_t unknownFormatVersion = 9999;
    std::copy(reinterpret_cast<uint8_t*>(&unknownFormatVersion),
              reinterpret_cast<uint8_t*>(&unknownFormatVersion) + sizeof(unknownFormatVersion),
              testData.begin() + FORMAT_VERSION_OFFSET);
    EXPECT_THROW(VaultHeader::parse(testData), VaultHeaderParsingError);
}

/*
 * Data that contains argon2OpsLimit smaller than crypto_pwhash_OPSLIMIT_MIN throws a
 * VaultHeaderParsingError
 */
TEST_F(VaultHeaderTest, Argon2OpsLimitTooSmallThrows) {
    uint64_t tooSmallOpsLimit = crypto_pwhash_OPSLIMIT_MIN - 1;
    std::copy(reinterpret_cast<uint8_t*>(&tooSmallOpsLimit),
              reinterpret_cast<uint8_t*>(&tooSmallOpsLimit) + sizeof(tooSmallOpsLimit),
              testData.begin() + ARGON2_OPSLIMIT_OFFSET);
    EXPECT_THROW(VaultHeader::parse(testData), VaultHeaderParsingError);
}

/*
 * Data that contains argon2OpsLimit greater than crypto_pwhash_OPSLIMIT_MAX throws a
 * VaultHeaderParsingError
 */
TEST_F(VaultHeaderTest, Argon2OpsLimitTooLargeThrows) {
    uint64_t tooLargeOpsLimit = crypto_pwhash_OPSLIMIT_MAX + 1;
    std::copy(reinterpret_cast<uint8_t*>(&tooLargeOpsLimit),
              reinterpret_cast<uint8_t*>(&tooLargeOpsLimit) + sizeof(tooLargeOpsLimit),
              testData.begin() + ARGON2_OPSLIMIT_OFFSET);
    EXPECT_THROW(VaultHeader::parse(testData), VaultHeaderParsingError);
}

/*
 * Data that contains argon2MemLimit smaller than crypto_pwhash_MEMLIMIT_MIN throws a
 * VaultHeaderParsingError
 */
TEST_F(VaultHeaderTest, Argon2MemLimitTooSmallThrows) {
    uint64_t tooSmallMemLimit = crypto_pwhash_MEMLIMIT_MIN - 1;
    std::copy(reinterpret_cast<uint8_t*>(&tooSmallMemLimit),
              reinterpret_cast<uint8_t*>(&tooSmallMemLimit) + sizeof(tooSmallMemLimit),
              testData.begin() + ARGON2_MEMLIMIT_OFFSET);
    EXPECT_THROW(VaultHeader::parse(testData), VaultHeaderParsingError);
}

/*
 * Data that contains argon2MemLimit greater than crypto_pwhash_MEMLIMIT_MAX throws a
 * VaultHeaderParsingError
 */
TEST_F(VaultHeaderTest, Argon2MemLimitTooLargeThrows) {
    uint64_t tooLargeMemLimit = crypto_pwhash_MEMLIMIT_MAX + 1;
    std::copy(reinterpret_cast<uint8_t*>(&tooLargeMemLimit),
              reinterpret_cast<uint8_t*>(&tooLargeMemLimit) + sizeof(tooLargeMemLimit),
              testData.begin() + ARGON2_MEMLIMIT_OFFSET);
    EXPECT_THROW(VaultHeader::parse(testData), VaultHeaderParsingError);
}

/*
 * Valid VaultHeader can be serialized and deserialized back to the same values
 */
TEST_F(VaultHeaderTest, SerializeAndDeserialize) {
    EXPECT_NO_THROW({
        VaultHeader originalHeader = VaultHeader::parse(testData);
        std::vector<uint8_t> serializedData = VaultHeader::serialize(originalHeader);
        EXPECT_EQ(serializedData.size(), VAULT_HEADER_BYTES);
        VaultHeader deserializedHeader = VaultHeader::parse(serializedData);
        EXPECT_EQ(deserializedHeader.formatVersion(), originalHeader.formatVersion());
        EXPECT_THAT(deserializedHeader.argon2Salt(),
                    ::testing::ElementsAreArray(originalHeader.argon2Salt()));
        EXPECT_EQ(deserializedHeader.argon2OpLimit(), originalHeader.argon2OpLimit());
        EXPECT_EQ(deserializedHeader.argon2MemLimit(), originalHeader.argon2MemLimit());
    });
}
