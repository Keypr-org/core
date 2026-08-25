#include "FileHandler.h"
#include <chrono>
#include <filesystem>

/**
 * @brief Saves a file atomically by writing to a temporary swap file first and then renaming it to
 * the target filename.
 *
 * @param filename The target filename to save the content to.
 * @param content The content to be saved, represented as a span of bytes.
 * @throws FileNotFoundError If the directory of the target filename does not exist.
 * @throws FileWriteError If there is an error writing to the swap file or renaming it to the target
 * filename.
 */
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
    if (!swapFile.is_open()) {
        throw FileWriteError("Failed to open swap file for writing: " + swapFilename);
    }
    swapFile.write(reinterpret_cast<const char*>(content.data()), content.size());
    // Ensure all data has been written to disk
    swapFile.flush();
    swapFile.close();

    // ---- Atomically replace the target file with the swap file ----
    std::error_code error;
    std::filesystem::rename(swapFilename, filename, error);
    if (error) {
        std::filesystem::remove(swapFilename);
        throw FileWriteError("Failed to rename swap file to target file: " + error.message());
    }
}

/*
 * @brief Opens a file for reading or writing.
 *
 * @param filename The name of the file to open.
 * @param mode The mode in which to open the file (default is read-only binary).
 *
 * @return An ifstream object representing the opened file.
 */
std::ifstream FileHandler::openFile(const std::string& filename, std::ios_base::openmode mode) {
    std::ifstream file(filename, mode);
    if (!file.is_open()) {
        throw FileNotFoundError("Failed to open file: " + filename);
    }
    return file;
}

/*
 * @brief Checks if a file exists and is a file.
 *
 * @param filename The name of the file to check.
 *
 * @return true if the file exists and is a file, false otherwise.
 */
bool FileHandler::fileExists(const std::string& filename) {
    return std::filesystem::exists(filename) && std::filesystem::is_regular_file(filename);
}
