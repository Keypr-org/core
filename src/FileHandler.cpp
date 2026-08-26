/*
 * @brief This file contains the implementation of the FileHandler class, which provides methods for
 * reading and writing files atomically.
 *
 * @author Nolan Evard
 * @date 26.08.2026
 *
 */
#include "FileHandler.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdio.h>
#endif

void FileHandler::saveFileAtomically(const std::string& filename, Bytes content) {
    // ---- Create a swap file with a timestamp so its_unique to write to first ----
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto target = std::filesystem::path(filename);
    const auto directory =
        target.parent_path().empty() ? std::filesystem::current_path() : target.parent_path();
    if (!std::filesystem::is_directory(directory)) {
        throw FileNotFoundError("Directory does not exist: " + directory.string());
    }
    const auto swapPath = directory / (target.stem().string() + std::to_string(timestamp) + ".tmp");

    // --- Write the content to the swap file ---
    std::ofstream swapFile(swapPath, std::ios::binary);
    if (!swapFile.is_open() || swapFile.fail()) {
        throw FileWriteError("Failed to open swap file for writing: " + swapPath.string());
    }
    swapFile.write(reinterpret_cast<const char*>(content.data()), content.size());

    if (swapFile.fail()) {
        // The error is not used but avoids remove from throwing an error
        std::error_code ec;
        std::filesystem::remove(swapPath, ec);
        throw FileWriteError("Failed to write to swap file: " + swapPath.string());
    }
    swapFile.flush();

    if (swapFile.fail()) {
        std::error_code ec;
        std::filesystem::remove(swapPath, ec);
        throw FileWriteError("Failed to flush swap file: " + swapPath.string());
    }

    swapFile.close();
    if (swapFile.fail()) {
        std::error_code ec;
        std::filesystem::remove(swapPath, ec);
        throw FileWriteError("Failed to close swap file: " + swapPath.string());
    }

    // ---- Atomically replace the target file with the swap file ----
    // Atomicity is not guaranteed by the STL filesystem library and replacing a file that already
    // exists might fail on windows. Thus the replace operation must be platform specific
#ifdef _WIN32

    if (!MoveFileExW(swapPath.c_str(), target.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        // Remove file on failure
        std::filesystem::remove(swapPath);
        throw FileWriteError("[Windows API]: Failed to move swap file to target file");
    }
#else
    if (rename(swapPath.c_str(), target.c_str()) != 0) {
        std::filesystem::remove(swapPath);
        throw FileWriteError("[POSIX API]: Failed to move swap file to target file");
    }
#endif
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
