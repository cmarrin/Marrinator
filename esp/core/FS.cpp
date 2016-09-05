/*-------------------------------------------------------------------------
This source file is a part of m8rscript

For the latest info, see http://www.marrin.org/

Copyright (c) 2016, Chris Marrin
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice, 
	  this list of conditions and the following disclaimer.
	  
    - Redistributions in binary form must reproduce the above copyright 
	  notice, this list of conditions and the following disclaimer in the 
	  documentation and/or other materials provided with the distribution.
	  
    - Neither the name of the <ORGANIZATION> nor the names of its 
	  contributors may be used to endorse or promote products derived from 
	  this software without specific prior written permission.
	  
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
-------------------------------------------------------------------------*/

#include "FS.h"

#include "Esp.h"

using namespace esp;

FS* FS::_sharedFS = nullptr;

FS* FS::sharedFS()
{
    if (!_sharedFS) {
        _sharedFS = new FS();
    }
    return _sharedFS;
}

FS::FS()
{
    memset(&_spiffsFileSystem, 0, sizeof(_spiffsFileSystem));
    _spiffsWorkBuf = new uint8_t[SPIFFS_CFG_LOG_PAGE_SZ() * 2];
    assert(_spiffsWorkBuf);

    memset(&_config, 0, sizeof(_config));
    _config.hal_read_f = spiffsRead;
    _config.hal_write_f = spiffsWrite;
    _config.hal_erase_f = spiffsErase;
}

FS::~FS()
{
    SPIFFS_unmount(&_spiffsFileSystem);
    delete _spiffsWorkBuf;
}

DirectoryEntry* FS::directory()
{
    if (!mounted()) {
        return nullptr;
    }
    return new DirectoryEntry(&_spiffsFileSystem);
}

bool FS::mount()
{
    os_printf("Mounting SPIFFS...\n");
    int32_t result = internalMount();
    if (result != SPIFFS_OK) {
        if (result == SPIFFS_ERR_NOT_A_FS) {
            os_printf("ERROR: Not a valid SPIFFS filesystem. Please format.\n");
        } else {
            os_printf("ERROR: SPIFFS mount failed, error=%d\n", result);
        }
        return false;
    }
    if (!mounted()) {
        os_printf("ERROR: SPIFFS filesystem failed to mount\n");
        return false;
    }

    os_printf("Checking file system...\n");
    result = SPIFFS_check(&_spiffsFileSystem);
    if (result != SPIFFS_OK) {
        os_printf("ERROR: Consistency check failed during SPIFFS mount, error=%d\n", result);
        return false;
    } else {
        os_printf("SPIFFS mounted successfully\n");
    }
    return true;
}

bool FS::mounted() const
{
    return SPIFFS_mounted(const_cast<spiffs_t*>(&_spiffsFileSystem));
}

void FS::unmount()
{
    if (mounted()) {
        SPIFFS_unmount(&_spiffsFileSystem);
    }
}

bool FS::format()
{
    if (!mounted()) {
        internalMount();
    }
    unmount();
    
    int32_t result = SPIFFS_format(&_spiffsFileSystem);
    if (result != SPIFFS_OK) {
        os_printf("ERROR: SPIFFS format failed, error=%d\n", result);
        return false;
    }
    return true;
}

File* FS::open(const char* name, spiffs_flags flags)
{
    return new File(name, flags);
}

int32_t FS::internalMount()
{
    return SPIFFS_mount(&_spiffsFileSystem, &_config, _spiffsWorkBuf,
                        _spiffsFileDescriptors, sizeof(_spiffsFileDescriptors), nullptr, 0, NULL);
}

int32_t FS::spiffsRead(uint32_t addr, uint32_t size, uint8_t *dst)
{
    return (flashmem_read(dst, addr, size) == size) ? SPIFFS_OK : SPIFFS_ERR_NOT_READABLE;
}

int32_t FS::spiffsWrite(uint32_t addr, uint32_t size, uint8_t *src)
{
    return (flashmem_write(src, addr, size) == size) ? SPIFFS_OK : SPIFFS_ERR_NOT_WRITABLE;
}

int32_t FS::spiffsErase(uint32_t addr, uint32_t size)
{
    u32_t firstSector = flashmem_get_sector_of_address(addr);
    u32_t lastSector = firstSector;
    while(firstSector <= lastSector) {
        if(!flashmem_erase_sector(firstSector++)) {
            return SPIFFS_ERR_INTERNAL;
        }
    }
    return SPIFFS_OK;
}

DirectoryEntry::DirectoryEntry(spiffs* fs)
{
	SPIFFS_opendir(fs, "/", &_dir);
    next();
}

DirectoryEntry::~DirectoryEntry()
{
    SPIFFS_closedir(&_dir);
}

bool DirectoryEntry::next()
{
    spiffs_dirent entry;
    _valid = SPIFFS_readdir(&_dir, &entry);
    if (_valid) {
        os_strcpy(_name, reinterpret_cast<const char*>(&(entry.name[0])));
        _size = entry.size;
    }
    return _valid;
}

File::File(const char* name, spiffs_flags flags)
{
    _file = SPIFFS_open(&(FS::sharedFS()->_spiffsFileSystem), name, flags, 0);
}

File::~File()
{
    SPIFFS_close(&(FS::sharedFS()->_spiffsFileSystem), _file);
}
  
int32_t File::read(char* buf, uint32_t size)
{
    return SPIFFS_read(&(FS::sharedFS()->_spiffsFileSystem), _file, buf, size);
}

int32_t File::write(const char* buf, uint32_t size)
{
    return SPIFFS_write(&(FS::sharedFS()->_spiffsFileSystem), _file, const_cast<char*>(buf), size);
}
