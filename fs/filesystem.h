// Copyright 2011 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// FatFS wrappers.

#ifndef AVRLIB_FS_FILE_SYSTEM_H_
#define AVRLIB_FS_FILE_SYSTEM_H_

#include <string.h>

#include "avrlib/avrlib.h"

#include "avrlib/third_party/ff/ff.h"

namespace avrlib {

enum FileSystemResult {
  FS_OK = 0,
  FS_DISK_ERROR,
  FS_EXCEPTION,
  FS_DRIVE_NOT_READY,
  FS_FILE_NOT_FOUND,
  FS_PATH_NOT_FOUND,
  FS_INVALID_NAME,
  FS_ACCESS_DENIED,
  FS_FILE_EXISTS,
  FS_INVALID_OBJECT,
  FS_WRITE_PROTECTED,
  FS_INVALID_DRIVE,
  FS_VOLUME_NOT_INITIALIZED,
  FS_NO_FAT_VOLUME,
  FS_FORMAT_FAILED,
  FS_TIMEOUT,
  FS_LOCKED,
  FS_NOT_ENOUGH_MEMORY,
  FS_TOO_MANY_FILES
};

class FileSystem {
 public:
  FileSystem() { }
  
  static FileSystemResult Init();
  static FileSystemResult Init(uint16_t timeout_ms);
  
  static FileSystemResult Unlink(const char* file_name);
  static FileSystemResult Mkdir(const char* dir_name);
  static FileSystemResult Mkdirs(const char* path);
  static FileSystemResult Chmod(
      const char* file_name,
      uint8_t value,
      uint8_t mask);
  static FileSystemResult Rename(const char* old_name, const char* new_name);
  static FileSystemResult Mkfs();
  
  static uint32_t GetFreeSpace();
  
 private:
  static FATFS fs_;
  
  DISALLOW_COPY_AND_ASSIGN(FileSystem);
};

}  // namespace avrlib

#endif   // AVRLIB_FS_FILE_SYSTEM_H_
