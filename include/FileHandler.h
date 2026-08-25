#pragma once

#include "Types.h"
#include <fstream>
#include <string>

class FileHandler {
  private:
    FileHandler() = delete;
    FileHandler(const FileHandler&) = delete;

  public:
    static void saveFileAtomically(const std::string& filename, Bytes content);
    static std::ifstream openFile(const std::string& filename,
                                  std::ios_base::openmode mode = std::ios::in | std::ios::binary);
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
