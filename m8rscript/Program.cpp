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

#include "Program.h"

using namespace m8r;

AtomTable Program::_atomTable;

Program::Program(SystemInterface* system) : _global(system)
{
}

Program::~Program()
{
}

bool Program::serialize(Stream* stream) const
{
    // Write the atom table
    if (!serializeWrite(stream, ObjectDataType::AtomTable)) {
        return false;
    }
    const String& atomTableString = _atomTable.stringTable();
    size_t size = atomTableString.length();
    assert(size < 65536);
    uint16_t ssize = static_cast<uint16_t>(size);
    if (!serializeWrite(stream, ssize)) {
        return false;
    }
    for (uint16_t i = 0; i < ssize; ++i) {
        if (!serializeWrite(stream, static_cast<uint8_t>(atomTableString[i]))) {
            return false;
        }
    }
        
    // Write the string table
    if (!serializeWrite(stream, ObjectDataType::StringTable)) {
        return false;
    }
    size = _stringTable.size();
    assert(size < 65536);
    ssize = static_cast<uint16_t>(size);
    if (!serializeWrite(stream, ssize)) {
        return false;
    }
    for (uint16_t i = 0; i < ssize; ++i) {
        if (!serializeWrite(stream, static_cast<uint8_t>(_stringTable[i]))) {
            return false;
        }
    }

    Function::serialize(stream);
    return true;
}
