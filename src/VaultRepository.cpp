#include "VaultRepository.h"
#include "FileHandler.h"
#include "RawVault.h"

bool VaultRepository::vaultExists(const std::string& filename) {
    if (!FileHandler::fileExists(filename))
        return false;
    try {
        RawVault::parse(FileHandler::readFile(filename));
    } catch (RawVaultParsingError& e) {
        return false;
    } catch (FileHandlerError& e) {
        return false;
    }
    return true;
}
