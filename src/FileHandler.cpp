#include "FileHandler.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>

void FileHandler::saveFileAtomically(const std::string& filename, Bytes content) {
    // ---- Create a swap file with a timestamp so its_unique to write to first ----
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::string directory = std::filesystem::path(filename).parent_path().string();
    if (!std::filesystem::is_directory(directory)) {
        throw FileNotFoundError("Directory does not exist: " + directory);
    }
    const std::string swapFilename = directory + "/.swap_" + std::to_string(timestamp) + ".tmp";

    // --- Write the content to the swap file ---
    std::ofstream swapFile(swapFilename, std::ios::binary);
    if (!swapFile.is_open() || swapFile.fail()) {
        throw FileWriteError("Failed to open swap file for writing: " + swapFilename);
    }
    swapFile.write(reinterpret_cast<const char*>(content.data()), content.size());

    if (swapFile.fail()) {
        std::filesystem::remove(swapFilename);
        throw FileWriteError("Failed to write to swap file: " + swapFilename);
    }
    // Ensure all data has been written to disk
    swapFile.flush();

    if (swapFile.fail()) {
        std::filesystem::remove(swapFilename);
        throw FileWriteError("Failed to flush swap file: " + swapFilename);
    }

    swapFile.close();
    if (swapFile.fail()) {
        std::filesystem::remove(swapFilename);
        throw FileWriteError("Failed to close swap file: " + swapFilename);
    }

    // ---- Atomically replace the target file with the swap file ----
    std::error_code error;
    std::filesystem::rename(swapFilename, filename, error);
    if (error) {
        std::filesystem::remove(swapFilename);
        throw FileWriteError("Failed to rename swap file to target file: " + error.message());
    }
}

std::vector<unsigned char> FileHandler::readFile(const std::string& filename) {
    auto file = std::ifstream(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw FileNotFoundError("Failed to open file: " + filename);
    }

    auto end = file.tellg();

    // tellg() returns negative value on failure
    if (end < 0) {
        throw FileHandlerError("Failed to determine file size: " + filename);
    }

    std::size_t size = static_cast<std::size_t>(end);

    std::vector<unsigned char> content(size);

    // Move pointer back to the beginning of the file
    file.seekg(0);

    // Read file contents using a pointer on the data of the vector
    if (!file.read(reinterpret_cast<char*>(content.data()), size)) {
        throw FileReadError("Failed to read file contents: " + filename);
    }

    file.close();

    return content;
}

bool FileHandler::fileExists(const std::string& filename) {
    return std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename);
}
