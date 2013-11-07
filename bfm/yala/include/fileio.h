#if ! defined(__FILEIO_H__)
#define __FILEIO_H__

#include <string>
#include <vector>
#include <dirent.h> // for MAX_PATH

// Just returns true or false depending on whether the named file exists or not.
bool FileExists(const char* const name);

// Load a given file into memory.
// Returns true if loaded successfully, otherwise false.
bool LoadFile(const char* const filename, std::vector<char>& content);

// Return a vector of the filenames contained within the given directory, not
// including the special files "." and "..".
// The vector of files is modified on successful return only.
// Returns true if the directory exists, otherwise false.
bool ListFiles(const char* const dirname, std::vector<std::string>& files);

#endif // __FILEIO_H__
