/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <string>
#include <vector>
//#include "Scanner.h"

// read Shared.txt and use it to generate GeneratedValues.cpp/GeneratedValues.h

//// Add string with tokens and special char sequences
//struct SpecialEntry
//{
//    m8r::Token token;
//    const char* str;
//};
//
//
//
//        "(),.:;?[]{}~"
//        "< << <= <<="
//        "> >> >>> >= >>= >>>="
//        "( ) , . : ; ? [ ] { } ~"
//        "! !="
//        "% %="
//        "& && &="
//        "* *="
//        "+ ++ +="
//        "- -- -="
//        "/ /="
//        "= =="
//        "^ ^="
//        "| || |="
//
//
//


static std::string strip(const char* in)
{
    std::string out;
    
    // remove leading spaces
    while(1) {
        if (isspace(*in)) {
            ++in;
        } else {
            break;
        }
    }
    
    while(1) {
        if (isspace(*in) || *in == '\n' || *in == '\0') {
            break;
        }
        out += *in++;
    }
    return out;
}

int main()
{
    char* root = getenv("ROOT");
    chdir(root);

    FILE* afile = fopen("generators/SharedAtoms.txt", "r");
    if (!afile) {
        printf("could not open SharedAtoms.txt:%d\n", errno);
        return -1;
    }
    
    FILE* hfile = fopen("../lib/m8rscript/src/GeneratedValues.h", "w");
    if (!hfile) {
        printf("could not open GeneratedValues.h:%d\n", errno);
        return -1;
    }
    
    FILE* cppfile = fopen("../lib/m8rscript/src/GeneratedValues.cpp", "w");
    if (!cppfile) {
        printf("could not open GeneratedValues.cpp:%d\n", errno);
        return -1;
    }
    
    // Write the preambles
    fprintf(hfile, "// This file is generated. Do not edit\n\n#include <cstdint>\n\nenum class SA : uint16_t {\n");
    fprintf(cppfile, "// This file is generated. Do not edit\n\n#include \"GeneratedValues.h\"\n#include \"Defines.h\"\n#include <cstdlib>\n\n");
    
    // Get the strings into a vector
    std::vector<std::string> strings;

    while (1) {
        char* line = nullptr;
        size_t length;
        ssize_t size = getline(&line, &length, afile);
        if (size < 0) {
            if (feof(afile)) {
                free(line);
                break;
            }
            printf("getline failed:%d\n", errno);
            free(line);
            return -1;
        }
        
        // Ignore lines starting with "//" to allow for comments
        if (line[0] == '/' && line[1] == '/') {
            free(line);
            continue;
        }
        
        std::string entry = strip(line);
        
        if (!entry.size()) {
            free(line);
            continue;
        }
        
        strings.push_back(entry);
        free(line);
    }
    
    // Sort the vector
    std::sort(strings.begin(), strings.end());

    // Write the .h entries and the first .cpp entries
    for (int32_t i = 0; i < strings.size(); ++i) {
        fprintf(hfile, "    %s = %d,\n", strings[i].c_str(), i);
        fprintf(cppfile, "static const char _%s[] ROMSTR_ATTR = \"%s\";\n", strings[i].c_str(), strings[i].c_str());
    }
    
    // Write the second .cpp entries
    fprintf(cppfile, "\nconst char* RODATA_ATTR _sharedAtoms[] = {\n");
    for (int32_t i = 0; i < strings.size(); ++i) {
        fprintf(cppfile, "    _%s,\n", strings[i].c_str());
    }
    
    // Round count to the nearest 100 to make it easier to compute byte offset into table.
    size_t count = ((strings.size() + 99) / 100) * 100;

    // Write the postambles
    fprintf(hfile, "};\n\nnamespace m8r {\n");
    fprintf(hfile, "    const char** sharedAtoms(uint16_t& nelts);\n");
    fprintf(hfile, "    static constexpr uint16_t ExternalAtomOffset = %zu;\n", count);
    fprintf(hfile, "}\n");

    fprintf(cppfile, "};\n\nconst char** m8r::sharedAtoms(uint16_t& nelts)\n{\n    nelts = sizeof(_sharedAtoms) / sizeof(const char*);\n    return _sharedAtoms;\n}\n");
    
    fclose(afile);
    fclose(hfile);
    fclose(cppfile);
    return 0;
}