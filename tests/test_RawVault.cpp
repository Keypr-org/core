#include <gtest/gtest.h>

#include "RawVault.h"

class RawVaultTest : public ::testing::Test {};

// ------------- TESTS --------------------

/*
 * Data smaller than the minimum vault file size throws a RawVaultParsingError
 */

/*
 * Data that does not contain header magic numbers throws a RawVaultParsingError
 */
