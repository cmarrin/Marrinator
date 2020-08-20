// This file is generated. Do not edit

#include "GeneratedValues.h"
#include "Defines.h"
#include <cstdlib>

static const char _and$[] = "and";
static const char _at[] = "at";
static const char _atput[] = "atput";
static const char _band[] = "band";
static const char _bnot[] = "bnot";
static const char _bor[] = "bor";
static const char _bxor[] = "bxor";
static const char _cat[] = "cat";
static const char _currentTime[] = "currentTime";
static const char _dec[] = "dec";
static const char _dup[] = "dup";
static const char _eq[] = "eq";
static const char _exec[] = "exec";
static const char _filter[] = "filter";
static const char _fold[] = "fold";
static const char _for$[] = "for";
static const char _ge[] = "ge";
static const char _gt[] = "gt";
static const char _if$[] = "if";
static const char _ifte[] = "ifte";
static const char _import[] = "import";
static const char _inc[] = "inc";
static const char _insert[] = "insert";
static const char _join[] = "join";
static const char _le[] = "le";
static const char _length[] = "length";
static const char _lt[] = "lt";
static const char _map[] = "map";
static const char _ne[] = "ne";
static const char _neg[] = "neg";
static const char _new$[] = "new";
static const char _not$[] = "not";
static const char _or$[] = "or";
static const char _pack[] = "pack";
static const char _pick[] = "pick";
static const char _pop[] = "pop";
static const char _print[] = "print";
static const char _remove[] = "remove";
static const char _swap[] = "swap";
static const char _tuck[] = "tuck";
static const char _unpack[] = "unpack";
static const char _while$[] = "while";

const char* _sharedAtoms[] = {
    _and$,
    _at,
    _atput,
    _band,
    _bnot,
    _bor,
    _bxor,
    _cat,
    _currentTime,
    _dec,
    _dup,
    _eq,
    _exec,
    _filter,
    _fold,
    _for$,
    _ge,
    _gt,
    _if$,
    _ifte,
    _import,
    _inc,
    _insert,
    _join,
    _le,
    _length,
    _lt,
    _map,
    _ne,
    _neg,
    _new$,
    _not$,
    _or$,
    _pack,
    _pick,
    _pop,
    _print,
    _remove,
    _swap,
    _tuck,
    _unpack,
    _while$,
};

const char* _specialChars = ""
    "\xc6!="
    "\xb8%="
    "\xc1&&"
    "\xb9&="
    "\xb6*="
    "\xbf++"
    "\xb4+="
    "\xc0--"
    "\xb5-="
    "\xb7/="
    "\xc5=="
    "\xba^="
    "\xc2||"
    "\xbb|="
    "\xc3<="
    "\xc4>="
    "\xbe<<"
    "\xbc>>"
;

const char** m8r::sharedAtoms(uint16_t& nelts)
{
    nelts = sizeof(_sharedAtoms) / sizeof(const char*);
    return _sharedAtoms;
}

const char* m8r::specialChars()
{
    return _specialChars;
}

