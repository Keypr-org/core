/*
 * @brief A utility class for handling file operations such as reading, writing, and checking file
 * existence.
 *
 * @author Nolan Evard
 * @date 25.08.2026
 */
#pragma once

#include "Types.h"
#include <stdexcept>
#include <string>
#include <vector>

class FileHandler {
  private:
    FileHandler() = delete;
    FileHandler(const FileHandler&) = delete;
    FileHandler(const FileHandler&&) = delete;

  public:
    /**
     * @brief Saves a file atomically by writing to a temporary swap file first and then renaming it
     * to the target filename.
     *
     * @param filename The target filename to save the content to.
     * @param content The content to be saved, represented as a span of bytes.
     * @throws FileNotFoundError If the directory of the target filename does not exist.
     * @throws FileWriteError If there is an error writing to the swap file or renaming it to the
     * target filename.
     */
    static void saveFileAtomically(const std::string& filename, Bytes content);

    /*
     * @brief Reads the entire contents of a file into a vector of unsigned char.
     *
     * @param filename The name of the file to read.
     *
     * @return A vector containing the contents of the file.
     *
     * @throws FileNotFoundError If the file does not exist or cannot be opened.
     * @throws FileHandlerError If there is an error determining the file size.
     */
    static std::vector<unsigned char> readFile(const std::string& filename);

    /*
     * @brief Checks if a file exists and is a file.
     *
     * @param filename The name of the file to check.
     *
     * @return true if the file exists and is a file, false otherwise.
     */
    static bool fileExists(const std::string& filename);
};

// ----------  File handler exceptions ---------------

class FileHandlerError : public std::runtime_error {
  public:
    // Makes FileHandlerError inherit runtime_error constructors
    using std::runtime_error::runtime_error;
};

class FileNotFoundError : public FileHandlerError {
  public:
    using FileHandlerError::FileHandlerError;
};

class FileWriteError : public FileHandlerError {
  public:
    using FileHandlerError::FileHandlerError;
};

class FileReadError : public FileHandlerError {
  public:
    using FileHandlerError::FileHandlerError;
};
