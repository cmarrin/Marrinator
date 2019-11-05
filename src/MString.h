/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#pragma once

#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>
#include "Defines.h"
#include "Float.h"

namespace m8r {

//
//  Class: String
//
//  String class that works on both Mac and ESP
//

class String {
public:
    String() : _size(1), _capacity(0), _data(nullptr) { }
    String(const char* s, int32_t len = -1) : _size(1), _capacity(0), _data(nullptr)
    {
        if (!s) {
            return;
        }
        if (len == -1) {
            len = static_cast<int32_t>(strlen(s));
        }
        ensureCapacity(len + 1);
        if (len) {
            memcpy(_data, s, len);
        }
        _size = len + 1;
        _data[_size - 1] = '\0';
    }
    
    String(const String& other) : _data(nullptr)
    {
        *this = other;
    };
    
    String(String&& other)
    {
        _data = other._data;
        other._data = nullptr;
        _size = other._size;
        other._size = 0;
        _capacity = other._capacity;
        other._capacity = 0;
    }
    
    String(char c)
    {
        ensureCapacity(2);
        _data[0] = c;
        _data[1] = '\0';
        _size = 2;
    }
    
    ~String() { delete [ ] _data; };

    String& operator=(const String& other)
    {
        if (_data) {
            free(_data);
        }
        _size = other._size;
        _capacity = other._capacity;
        if (!other._data) {
            _data = nullptr;
            return *this;
        }
        
        _data = new char[_capacity];
        assert(_data);
        if (_data) {
            memcpy(_data, other._data, _size);
        } else {
            _capacity = 0;
            _size = 1;
        }
        return *this;
    }
    
    String& operator=(String&& other)
    {
        if (this == &other) {
            return *this;
        }

        delete[] _data;

        _data = other._data;
        _size = other._size;
        _capacity = other._capacity;

        other._data = nullptr;
        other._size = 0;
        other._capacity = 0;

        return *this;
    }
    
    String& operator=(char c)
    {
        ensureCapacity(2);
        _data[0] = c;
        _data[1] = '\0';
        _size = 2;
        return *this;
    }

    operator bool () { return !empty(); }
    
    const char& operator[](size_t i) const { assert(i >= 0 && i < _size - 1); return _data[i]; };
    char& operator[](size_t i) { assert(i >= 0 && i < _size - 1); return _data[i]; };
    size_t size() const { return _size ? (_size - 1) : 0; }
    bool empty() const { return _size <= 1; }
    void clear() { _size = 1; if (_data) _data[0] = '\0'; }
    String& operator+=(uint8_t c)
    {
        ensureCapacity(_size + 1);
        _data[_size - 1] = c;
        _data[_size++] = '\0';
        return *this;
    }
    
    String& operator+=(char c)
    {
        ensureCapacity(_size + 1);
        _data[_size - 1] = c;
        _data[_size] = '\0';
        _size += 1;
        return *this;
    }
    
    String& operator+=(const char* s)
    {
        size_t len = strlen(s);
        ensureCapacity(_size + len);
        memcpy(_data + _size - 1, s, len + 1);
        _size += len;
        return *this;
    }
    
    String& operator+=(const String& s) { return *this += s.c_str(); }
    
    friend String operator +(const String& s1 , const String& s2) { String s = s1; s += s2; return s; }
    friend String operator +(const String& s1 , const char* s2) { String s = s1; s += s2; return s; }
    friend String operator +(const String& s1 , char c) { String s = s1; s += c; return s; }
    friend String operator +(const char* s1 , const String& s2) { String s = s1; s += s2; return s; }
    
    bool operator<(const String& other) const { return compare(*this, other) < 0; }
    bool operator<=(const String& other) const { return compare(*this, other) <= 0; }
    bool operator>(const String& other) const { return compare(*this, other) > 0; }
    bool operator>=(const String& other) const { return compare(*this, other) >= 0; }
    bool operator==(const String& other) const { return compare(*this, other) == 0; }
    bool operator!=(const String& other) const { return compare(*this, other) != 0; }
    
    static int compare(const String& a, const String& b)
    {
        return strcmp(a.c_str(), b.c_str());
    }

    const char* c_str() const { return _data ? _data : ""; }
    String& erase(size_t pos, size_t len);

    String& erase(size_t pos = 0)
    {
        return erase(pos, _size - pos);
    }
    
    String slice(int32_t start, int32_t end) const;
    
    String slice(int32_t start) const
    {
        return slice(start, static_cast<int32_t>(size()));
    }
    
    String trim() const;
    
    // If skipEmpty is true, substrings of zero length are not added to the array
    std::vector<String> split(const String& separator, bool skipEmpty = false) const;
    
    static String join(const std::vector<String>& array, const String& separator);
    
    static String join(const std::vector<char>& array);
    
    bool isMarked() const { return _marked; }
    void setMarked(bool b) { _marked = b; }
    
    void reserve(size_t size) { ensureCapacity(size); }
    
    static String toString(Float value);
    static String toString(int32_t value);
    static bool toFloat(Float&, const char*, bool allowWhitespace = true);
    static bool toInt(int32_t&, const char*, bool allowWhitespace = true);
    static bool toUInt(uint32_t&, const char*, bool allowWhitespace = true);
    
private:
    void doEnsureCapacity(size_t size);
    
    void ensureCapacity(size_t size)
    {
        if (_capacity >= size) {
            return;
        }
        doEnsureCapacity(size);
    }

    size_t _size = 0;
    size_t _capacity = 0;
    char *_data = nullptr;
    bool _marked = true;
};

}