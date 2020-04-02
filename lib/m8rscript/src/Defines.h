/*-------------------------------------------------------------------------
    This source file is a part of m8rscript
    For the latest info, see http:www.marrin.org/
    Copyright (c) 2018-2019, Chris Marrin
    All rights reserved.
    Use of this source code is governed by the MIT license that can be
    found in the LICENSE file.
-------------------------------------------------------------------------*/

#pragma once

// This allows umm to be turned off just for Mac. It should never be turned off for ESP
#ifdef __APPLE__
#define USE_UMM
#else
#define USE_UMM
#endif

#define USE_LITTLEFS

// Do this so we can present defines for malloc/free for c files
#ifndef __cplusplus

#else

#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <cassert>
#include <limits>
#include <cstring>

// Debugging
static inline void DBG_PRINT(const char* type, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("===== %s: ", type);
    vprintf(fmt, args);
    printf("\n");
}

//#define DEBUG_TIMERS
#ifdef DEBUG_TIMERS 
    #define DBG_TIMERS(fmt, ...) DBG_PRINT("TMR", fmt, ##__VA_ARGS__)
#else
    #define DBG_TIMERS(fmt, ...)
#endif

// FIXME: Doing this allows things like 2s and 3.5ms.
// But they are in units of chrono::duration, so we need to make a convervsion
// between that and our Duration class. Then we can get rid of operator""_sec
// and friends.
//#include <chrono>
//
//using namespace std::chrono_literals;

namespace m8r {
    class ROMString
    {
    public:
        ROMString() { }
        explicit ROMString(const char* s) : _value(s) { }
        explicit ROMString(const uint8_t* s) : _value(reinterpret_cast<const char*>(s)) { }
        
        bool valid() const { return _value; }
        
        ROMString operator+(int32_t i) const { return ROMString(_value + i);  }
        ROMString operator-(int32_t i) const { return ROMString(_value - i);  }
        ROMString operator+=(int32_t i) { _value += i; return *this; }
        ROMString operator-=(int32_t i) { _value += i; return *this; }
        
        const char* value() const { return _value; }

    private:
        const char* _value = nullptr;
    };
}

#ifdef __APPLE__
    #define RODATA_ATTR
    #define RODATA2_ATTR
    #define ROMSTR_ATTR
    #define FLASH_ATTR
    #define ICACHE_FLASH_ATTR
    static inline uint8_t readRomByte(m8r::ROMString addr) { return *(addr.value()); }
    static inline void* ROMmemcpy(void* dst, m8r::ROMString src, size_t len) { return memcpy(dst, src.value(), len); }
    static inline size_t ROMstrlen(m8r::ROMString s) { return strlen(s.value()); }
    static inline m8r::ROMString ROMstrstr(m8r::ROMString s1, const char* s2) { return m8r::ROMString(strstr(s1.value(), s2)); }
    static inline int ROMstrcmp(m8r::ROMString s1, const char* s2) { return strcmp(s1.value(), s2); }

    static inline char* ROMCopyString(char* dst, m8r::ROMString src) { strcpy(dst, src.value()); return dst + ROMstrlen(src); }
    #define ROMSTR(s) m8r::ROMString(s)

    template <typename T>
    static inline const char* typeName() { return typeid(T).name(); }
#else

#ifndef ARDUINO
    #include <esp_attr.h>
#endif
    
    #ifndef __STRINGIFY
    #define __STRINGIFY(a) #a
    #endif
    #define RODATA_ATTR _SECTION_ATTR_IMPL(".rodata2", __COUNTER__) 
    #define RODATA2_ATTR _SECTION_ATTR_IMPL(".rodata2", __COUNTER__)
    #define ROMSTR_ATTR
    #define FLASH_ATTR
    #define ICACHE_FLASH_ATTR
//    #define FLASH_ATTR   __attribute__((section(".irom0.text")))
//    #define RAM_ATTR     __attribute__((section(".iram.text")))
//    #define RODATA_ATTR  __attribute__((section(".irom.text"))) __attribute__((aligned(4)))
//    #define RODATA2_ATTR  __attribute__((section(".irom2.text"))) __attribute__((aligned(4)))
//    #define ROMSTR_ATTR  __attribute__((section(".irom.text.romstr"))) __attribute__((aligned(4)))

    size_t ROMstrlen(m8r::ROMString s);
    void* ROMmemcpy(void* dst, m8r::ROMString src, size_t len);
    char* ROMCopyString(char* dst, m8r::ROMString src);
    m8r::ROMString ROMstrstr(m8r::ROMString s1, const char* s2);
    int ROMstrcmp(m8r::ROMString s1, const char* s2);

    #define ROMSTR(s) m8r::ROMString(__extension__({static const char __c[] ROMSTR_ATTR = (s); &__c[0];}))

    static inline uint8_t FLASH_ATTR readRomByte(m8r::ROMString addr)
    {
        uint32_t bytes = *(uint32_t*)((uint32_t)(addr.value()) & ~3);
        return ((uint8_t*)&bytes)[(uint32_t)(addr.value()) & 3];
    }

    template <typename T>
    static inline const char* typeName() { return ""; }

#endif

namespace m8r {

int32_t heapFreeSize();

static inline bool isdigit(uint8_t c)		{ return c >= '0' && c <= '9'; }
static inline bool isLCHex(uint8_t c)       { return c >= 'a' && c <= 'f'; }
static inline bool isUCHex(uint8_t c)       { return c >= 'A' && c <= 'F'; }
static inline bool isHex(uint8_t c)         { return isUCHex(c) || isLCHex(c); }
static inline bool isxdigit(uint8_t c)		{ return isHex(c) || isdigit(c); }
static inline bool isOctal(uint8_t c)       { return c >= '0' && c <= '7'; }
static inline bool isUpper(uint8_t c)		{ return (c >= 'A' && c <= 'Z'); }
static inline bool isLower(uint8_t c)		{ return (c >= 'a' && c <= 'z'); }
static inline bool isLetter(uint8_t c)		{ return isUpper(c) || isLower(c); }
static inline bool isIdFirst(uint8_t c)		{ return isLetter(c) || c == '$' || c == '_'; }
static inline bool isIdOther(uint8_t c)		{ return isdigit(c) || isIdFirst(c); }
static inline bool isspace(uint8_t c)       { return c == ' ' || c == '\n' || c == '\r' || c == '\f' || c == '\t' || c == '\v'; }
static inline uint8_t tolower(uint8_t c)    { return isUpper(c) ? (c - 'A' + 'a') : c; }
static inline uint8_t toupper(uint8_t c)    { return isLower(c) ? (c - 'a' + 'A') : c; }

// KeyActions have a 1-4 character code which m8rscript can compare against. For instance
// 
//      function handleAction(action)
//      {
//          if (action == "down") ...
//      }
//
// Interrupt is control-c
//
// to make this work efficiently Action enumerants are uint32_t with the characters packed
// in. These are converted to StringLiteral Values and sent to the script.     

static constexpr uint32_t makeAction(const char* s)
{
    return
        (static_cast<uint32_t>(s[0]) << 24) |
        (static_cast<uint32_t>(s[1]) << 16) |
        (static_cast<uint32_t>(s[2]) << 8) |
        static_cast<uint32_t>(s[3]);
}

enum class KeyAction : uint32_t {
    None = 0,
    UpArrow = makeAction("up  "),
    DownArrow = makeAction("down"),
    RightArrow = makeAction("rt  "),
    LeftArrow = makeAction("lt  "),
    Delete = makeAction("del "),
    Backspace = makeAction("bs  "),
    Interrupt = makeAction("intr"),
    NewLine = makeAction("newl"),
};

//  Class: Id/RawId template
//
//  Generic Id class

template <typename RawType>
class Id
{
public:
    class Raw
    {
        friend class Id;

    public:
        Raw() : _raw(NoId) { }
        explicit Raw(RawType raw) : _raw(raw) { }
        RawType raw() const { return _raw; }

    private:
        RawType _raw;
    };
    
    using value_type = RawType;
    
    Id() { _value = Raw(NoId); }
    explicit Id(Raw raw) { _value._raw = raw._raw; }
    explicit Id(RawType raw) { _value._raw = raw; }
    Id(const Id& other) { _value._raw = other._value._raw; }
    Id(Id&& other) { _value._raw = other._value._raw; }

    value_type raw() const { return _value._raw; }

    const Id& operator=(const Id& other) { _value._raw = other._value._raw; return *this; }
    Id& operator=(Id& other) { _value._raw = other._value._raw; return *this; }
    const Id& operator=(const Raw& other) { _value._raw = other._raw; return *this; }
    Id& operator=(Raw& other) { _value._raw = other._raw; return *this; }
    explicit operator bool() const { return _value._raw != NoId; }

    int operator-(const Id& other) const { return static_cast<int>(_value._raw) - static_cast<int>(other._value._raw); }
    bool operator==(const Id& other) const { return _value._raw == other._value._raw; }

private:
    static constexpr RawType NoId = std::numeric_limits<RawType>::max();

    Raw _value;
};

class StringLiteral : public Id<uint32_t> { using Id::Id; };
class ConstantId : public Id<uint8_t> { using Id::Id; };

using RawMad = intptr_t;
static constexpr RawMad NoRawMad = 0;

enum class MemoryType : uint8_t {
    Unknown,
    String,
    Character,
    Object,
    ExecutionUnit,
    Native,
    Vector,
    UpValue,
    Network,
    Fixed,
    NumTypes
};

struct MemoryInfo{
    struct Entry
    {
        uint32_t size = 0;
        uint32_t count = 0;
    };
    
    uint32_t totalAllocatedBytes = 0;
    uint16_t numAllocations = 0;
    std::array<Entry, static_cast<uint32_t>(MemoryType::NumTypes)> allocationsByType;
};

struct Label {
    int32_t label : 20;
    uint32_t uniqueID : 12;
    int32_t matchedAddr : 20;
};

/*
    Opcodes:

    Instructions are a series of bytes. Opcode is always first
    byte and is followed by 0-3 bytes of operands. Most operands
    are 1 byte (register index, upvalue index, constant index or
    nparams). In some cases operand is a 2 byte uint16_t or
    int16_t. The are 2 consecutive bytes, MSB first.
    
    Opcodes are 6 bits. The upper 2 bits of the Opcode byte are
    "immediate" bits. They are used by some opcodes (ending in 'I')
    as an additional operand. For instance RETI uses them as the
    return count, from 0 to 3. Because of this the inline accessor
    function (e.g., opFromCode) must be used to access opcodes.
    These both ensure that the upper 2 bits are masked out and
    that if you attempt to access an opcode that should not have
    immediate bits set and it does it will assert.

    During parsing, instructions are placed in an Instruction
    object. This contains opcode, operands and an indication of
    which operands are valid. This instruction is then emitted
    in the code vector, which is a series of uint8_t.
    
    The structure of each instruction is given below, using this
    key:

    R       - Register (0..127)
    RK      - Register (0..127) or Constant (128..255)
    UN      - Line number (0..64K)
    SN      - Address (-32K..32K)
    L       - Local variable (0..127) - only used during initial code generation
    NPARAMS - Param count (0..255)
    
    Local vs Register parameters
    ----------------------------
    During initial code generation, register indexes are relative to 0 and grow
    according to the number of register entries needed at any given time.
    The high water mark is saved in the Function when code generation for that
    Function is closed. Local variables for the function are also indexed 
    starting at 0, but in a local variable space. During initial code
    generation these are loaded into register variables by generating the LOADL
    opcode. The destination is in the register space and the source is in
    the locals space.
    
    After the Function is closed, a pass is done on the generated code. All
    register indexs are bumped by the number of locals and the LOADL opcode
    is changed to a MOVE with the destination index bumped but the source
    index left intact. So the LOADL opcode should never make it through to
    the ExecutionUnit.

    UNKNOWN
    RET         NPARAMS
    RETI        IMM
    END
    
    MOVE        R[d], RK[s]
    LOADREFK    R[d], RK[s]
    STOREFK     RK[d], RK[s]
    LOADUP      R[d], U[s]
    APPENDELT   R[d], RK[s]
 
    APPENDPROP  R[d], RK[p], RK[s]
 
    LOADLITA    R[d]
    LOADLITO    R[d]
    LOADTRUE    R[d]
    LOADFALSE   R[d]
    LOADNULL    R[d]
    LOADTHIS    R[d]
    
    PUSH        RK[s]
    POP         R[d]
    POPX
 
    LOADPROP    R[d], RK[o], K[p]
    LOADELT     R[d], RK[o], RK[e]
    STOPROP     R[o], K[p], RK[s]
    STOELT      R[o], RK[e], RK[s]
 
    <binop> ==> LOR, LAND,                          ; (20)
                OR, AND, XOR,
                EQ, NE, LT, LE, GT, GE,
                SHL, SHR, SAR,
                ADD, SUB, MUL, DIV, MOD
 
    <binop>RR   R[d], RK[s1], RK[s2]
 
    UMINUS      R[d], RK[s]
    UNOT        R[d], RK[s]
    UNEG        R[d], RK[s]

    PREINC      R[d], R[s]
    PREDEC      R[d], R[s]
    POSTINC     R[d], R[s]
    POSTDEC     R[d], R[s]

    CALL        RK[call], RK[this], NPARAMS
    NEW         RK[call], NPARAMS
    CALLPROP    RK[o], RK[p], NPARAMS
    CLOSURE     R[d], RK[s]
    
    JMP         SN
    JT          RK[s], SN
    JF          RK[s], SN
    LINENO      UN
 
    Total: 51 instructions
*/

static constexpr uint32_t MaxRegister = 127;
static constexpr uint32_t MaxParams = 256;
static constexpr int32_t MaxJump = 32767;

// Opcodes are 6 bits, 0x00 to 0x3f

// ESP RTOS defines SAR. Fix that here to avoid errors
#undef SAR

enum class Op : uint8_t {
    
    MOVE = 0x00, LOADREFK, STOREFK, LOADLITA,
    LOADLITO, LOADPROP, LOADELT, STOPROP,
    STOELT, APPENDELT, APPENDPROP, LOADTRUE,
    LOADFALSE, LOADNULL, PUSH, POP,

    LOR = 0x10, LAND, OR, AND,
    XOR, EQ,  NE, LT,
    LE, GT, GE, SHL,
    SHR, SAR, ADD, SUB,
    
    MUL = 0x20, DIV, MOD, UMINUS,
    UNOT, UNEG, PREINC, PREDEC,
    POSTINC, POSTDEC, CALL, NEW,
    CALLPROP, JMP, JT, JF,

    LINENO = 0x30, LOADTHIS, LOADUP,
    CLOSURE, YIELD, POPX, RETI, 
    
    // 0x37 - 0x3c open

    END = 0x3d, RET = 0x3e, UNKNOWN = 0x3f,
    
    LAST
};

static_assert(static_cast<uint32_t>(Op::LAST) <= 0x40, "Opcode must fit in 6 bits");

class OpInfo
{
public:
    static uint8_t size(Op op) { return array(op).size; }
    static bool aReg(Op op) { return flagFromLayout(op, Flags::a); }
    static bool bReg(Op op) { return flagFromLayout(op, Flags::b); }
    static bool cReg(Op op) { return flagFromLayout(op, Flags::c); }
    static bool dReg(Op op) { return flagFromLayout(op, Flags::d); }
    static bool imm(Op op) { return flagFromLayout(op, Flags::imm); }
    static bool params(Op op) { return flagFromLayout(op, Flags::P); }
    static bool number(Op op) { return flagFromLayout(op, Flags::N); }

private:
    // Bits here are a(0x01), b(0x02), c(0x04), sn(0x08), un(0x10)
    enum class Flags : uint8_t { None = 0, a = 0x01, b = 0x02, c = 0x04, d = 0x08, imm = 0x10, P = 0x20, N = 0x40 };
    
    // Regs a and d must be registers (<= MaxRegister)
    // Regs b and c can be reg or constant
    
    enum class Layout : uint8_t {
        None    = 0,
        A       = static_cast<uint8_t>(Flags::a),
        B       = static_cast<uint8_t>(Flags::b),
        AB      = static_cast<uint8_t>(Flags::a) | static_cast<uint8_t>(Flags::b),
        AD      = static_cast<uint8_t>(Flags::a) | static_cast<uint8_t>(Flags::d),
        BC      = static_cast<uint8_t>(Flags::b) | static_cast<uint8_t>(Flags::c),
        ABC     = static_cast<uint8_t>(Flags::a) | static_cast<uint8_t>(Flags::b) | static_cast<uint8_t>(Flags::c),
        IMM     = static_cast<uint8_t>(Flags::imm),
        P       = static_cast<uint8_t>(Flags::P),
        BP      = static_cast<uint8_t>(Flags::b) | static_cast<uint8_t>(Flags::P),
        BCP     = static_cast<uint8_t>(Flags::b) | static_cast<uint8_t>(Flags::c) | static_cast<uint8_t>(Flags::P),
        N       = static_cast<uint8_t>(Flags::N),
        AN      = static_cast<uint8_t>(Flags::a) | static_cast<uint8_t>(Flags::N),
        ABN     = static_cast<uint8_t>(Flags::a) | static_cast<uint8_t>(Flags::b) | static_cast<uint8_t>(Flags::N),
    };
    
    static bool flagFromLayout(Op op, Flags flag)
    {
        return static_cast<uint8_t>(array(op).layout) & static_cast<uint8_t>(flag);
    }
    
    struct Entry
    {
        Layout layout;
        uint8_t size;
    };

    static const Entry& array(Op op)
    {
        static const Entry RODATA_ATTR _array[ ] = {
/*0x00 */   { Layout::AB,   2 },   // MOVE         R[d], RK[s]
            { Layout::AB,   2 },   // LOADREFK     R[d], RK[s]
            { Layout::BC,   2 },   // STOREFK      RK[d], RK[s]
            { Layout::A,    1 },   // LOADLITA     R[d]
            { Layout::A,    1 },   // LOADLITO     R[d]
            { Layout::ABC,  3 },   // LOADPROP     R[d], RK[o], K[p]
            { Layout::ABC,  3 },   // LOADELT      R[d], RK[o], RK[e]
            { Layout::ABC,  3 },   // STOPROP      R[o], K[p], RK[s]
            { Layout::ABC,  3 },   // STOELT       R[o], RK[e], RK[s]
            { Layout::AB,   2 },   // APPENDELT    R[d], RK[s]
            { Layout::ABC,  3 },   // APPENDPROP   R[d], RK[p], RK[s]
            { Layout::A,    1 },   // LOADTRUE     R[d]
            { Layout::A,    1 },   // LOADFALSE    R[d]
            { Layout::A,    1 },   // LOADNULL     R[d]
            { Layout::B,    1 },   // PUSH         RK[s]
            { Layout::A,    1 },   // POP          R[d]
             
/*0x10 */   { Layout::ABC,  3 },   // LOR          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // LAND         R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // OR           R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // AND          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // XOR          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // EQ           R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // NE           R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // LT           R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // LE           R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // GT           R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // GE           R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // SHL          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // SHR          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // SAR          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // ADD          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // SUB          R[d], RK[s1], RK[s2]
              
/*0x20 */   { Layout::ABC,  3 },   // MUL          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // DIV          R[d], RK[s1], RK[s2]
            { Layout::ABC,  3 },   // MOD          R[d], RK[s1], RK[s2]
            
            { Layout::AB,   2 },   // UMINUS       R[d], RK[s]
            { Layout::AB,   2 },   // UNOT         R[d], RK[s]
            { Layout::AB,   2 },   // UNEG         R[d], RK[s]
            { Layout::AD,   2 },   // PREINC       R[d], R[s]
            { Layout::AD,   2 },   // PREDEC       R[d], R[s]
            { Layout::AD,   2 },   // POSTINC      R[d], R[s]
            { Layout::AD,   2 },   // POSTDEC      R[d], R[s]
 
            { Layout::BCP,  3 },   // CALL         RK[call], RK[this], NPARAMS
            { Layout::BP,   2 },   // NEW          RK[call], NPARAMS
            { Layout::BCP,  3 },   // CALLPROP     RK[o], RK[p], NPARAMS
            { Layout::N,    2 },   // JMP          SN
            { Layout::AN,   3 },   // JT           RK[s], SN
            { Layout::AN,   3 },   // JF           RK[s], SN
             
/*0x30 */   { Layout::N,    2 },   // LINENO       UN
            { Layout::A,    1 },   // LOADTHIS     R[d]
            { Layout::AD,   2 },   // LOADUP       R[d], U[s]
            { Layout::AB,   2 },   // CLOSURE      R[d], RK[s]
            { Layout::None, 0 },   // YIELD
            { Layout::None, 0 },   // POPX
            { Layout::IMM,  0 },   // RETI
            
/*0x37 */   { Layout::None, 0 },   // unused
            { Layout::None, 0 },   // unused
            { Layout::None, 0 },   // unused
            { Layout::None, 0 },   // unused
            { Layout::None, 0 },   // unused
/*0x3c */   { Layout::None, 0 },   // unused

/*0x3d */   { Layout::None, 0 },   // END
/*0x3e */   { Layout::P,    1 },   // RET          NPARAMS
/*0x3f */   { Layout::None, 0 },   // UNKNOWN
        };
        
        assert(static_cast<uint8_t>(op) < sizeof(_array) / sizeof(Entry));
        return _array[static_cast<uint8_t>(op)];
    }
};

enum class BuiltinConstants {
    Undefined = 0,
    Null = 1,
    Int0 = 2,
    Int1 = 3,
    AtomShort = 4,  // Next byte is Atom Id (0-255)
    AtomLong = 5,   // Next 2 bytes are Atom Id (Hi:Lo, 0-65535)
    NumBuiltins = 6
};

static inline uint8_t builtinConstantOffset() { return static_cast<uint8_t>(BuiltinConstants::NumBuiltins); }
static inline bool shortSharedAtomConstant(uint8_t reg) { return reg > MaxRegister && (reg - MaxRegister - 1) == static_cast<uint8_t>(BuiltinConstants::AtomShort); }
static inline bool longSharedAtomConstant(uint8_t reg) { return reg > MaxRegister && (reg - MaxRegister - 1) == static_cast<uint8_t>(BuiltinConstants::AtomLong); }

static inline uint8_t constantSize(uint8_t reg)
{
    if (reg <= MaxRegister) {
        return 0;
    }
    if (shortSharedAtomConstant(reg)) {
        return 1;
    }
    if (longSharedAtomConstant(reg)) {
        return 2;
    }
    return 0;
}

static inline Op opFromByte(uint8_t c) { return static_cast<Op>(c & 0x3f); }
static inline uint8_t immFromByte(uint8_t c) { return c >> 6; }
static inline uint8_t byteFromOp(Op op) { return static_cast<uint8_t>(op); }
static inline uint8_t immFromOp(Op op) { return byteFromOp(op) >> 6; }
static inline uint8_t byteFromOp(Op op, uint8_t imm) { assert(byteFromOp(op) <= 0x3f); return byteFromOp(op) | (imm << 6); }
static inline uint8_t byteFromCode(const uint8_t*& code) { return *code++; }

static inline Op opFromCode(const uint8_t*& code)
{
    // Using this form is only for opcodes that don't
    // have any immediate bits set. Assure that here
#ifndef NDEBUG
    uint8_t c = byteFromCode(code);
    assert(immFromByte(c) == 0);
    return opFromByte(c);
#else
    return opFromByte(byteFromCode(code));
#endif
}
    
static inline Op opFromCode(const uint8_t*& code, uint8_t& imm)
{
    uint8_t c = byteFromCode(code);
    imm = immFromByte(c);
    return opFromByte(c);
}

static inline uint16_t uNFromCode(const uint8_t*& code)
{
    uint16_t n = static_cast<uint16_t>(byteFromCode(code)) << 8;
    return n | static_cast<uint16_t>(byteFromCode(code));
}
static inline int16_t sNFromCode(const uint8_t*& code) { return static_cast<int16_t>(uNFromCode(code)); }

#undef DEC
enum class Token : uint8_t {
    Break       = 0x01,
    Case        = 0x02,
    Class       = 0x03,
    Constructor = 0x04,
    Continue    = 0x05,
    Default     = 0x06,
    Delete      = 0x07,
    Do          = 0x08,
    Else        = 0x09,
    False       = 0x0a,
    For         = 0x0b,
    Function    = 0x0c,
    If          = 0x0d,
    New         = 0x0e,
    Null        = 0x0f,
    Return      = 0x10,
    Switch      = 0x11,
    This        = 0x12,
    True        = 0x13,
    Undefined   = 0x14,
    Var         = 0x15,
    While       = 0x016,
    
    Bang        = '!',
    Percent     = '%',
    Ampersand   = '&',
    LParen      = '(',
    RParen      = ')',
    Star        = '*',
    Plus        = '+',
    Comma       = ',',
    Minus       = '-',
    Period      = '.',
    Slash       = '/',
    
    Colon       = ':',
    Semicolon   = ';',
    LT          = '<',
    STO         = '=',
    GT          = '>',
    Question    = '?',
    
    LBracket    = '[',
    RBracket    = ']',
    XOR         = '^',
    LBrace      = '{',
    OR          = '|',
    RBrace      = '}',
    Twiddle     = '~',
    
    SHRSTO      = 0x80,
    SARSTO      = 0x81,
    SHLSTO      = 0x82,
    ADDSTO      = 0x83,
    SUBSTO      = 0x84,
    MULSTO      = 0x85,
    DIVSTO      = 0x86,
    MODSTO      = 0x87,
    ANDSTO      = 0x88,
    XORSTO      = 0x89,
    ORSTO       = 0x8a,
    SHR         = 0x8b,
    SAR         = 0x8c,
    SHL         = 0x8d,
    INC         = 0x8e,
    DEC         = 0x8f,
    LAND        = 0x90,
    LOR         = 0x91,
    LE          = 0x92,
    GE          = 0x93,
    EQ          = 0x94,
    NE          = 0x95,
    
    Unknown     = 0xc0,
    Comment     = 0xc1,
    Whitespace  = 0xc2,

    Float       = 0xd0,
    Identifier  = 0xd1,
    String      = 0xd2,
    Integer     = 0xd3,
    
    Expr        = 0xe0,
    
    PropertyAssignment      = 0xe1,
    Statement               = 0xe2,
    DuplicateDefault        = 0xe3,
    MissingVarDecl          = 0xe4,
    OneVarDeclAllowed       = 0xe5,
    ConstantValueRequired   = 0xe6,

    None        = 0xfd,
    Error       = 0xfe,
    EndOfFile   = 0xff,
};

static constexpr uint8_t MajorVersion = 0;
static constexpr uint8_t MinorVersion = 2;

// File format is a sequence of blocks. Some blocks are simply a byte
// token indicating the block type and others are a generic format
// with the token byte followed by 2 bytes of length. Every version
// must understand Type, Version, ObjectStart and ObjectEnd. All others
// are optional can be skipped by skipping the token byte and the number
// of bytes contained in the next 2 bytes (high byte is MSB).
//
// Currently, the only properties stored are nested functions inside Function
// objects (which includes Program objects)
enum class ObjectDataType : uint8_t {
    End = 0x00,
    Type = 0x01,            // { 4 bytes: 'm', '8', 'r' }
    Version = 0x01,         // { uint8_t major, uint8_t minor }
    
    // Object
    ObjectStart = 0x10,     // Indicates start of object data
    ObjectName = 0x11,      // { uint16_t size, uint8_t name[size] }
    PropertyCount = 0x12,   // { uint16_t size = 2, uint16_t count }
    PropertyId = 0x13,      // { uint16_t size = 2, uint16_t id }
                            // Must be immediately followed by object
    ObjectEnd = 0x1f,       // Indicates end of object data

    // Program
    AtomTable = 0x20,       // { uint16_t size, uint8_t data[size] }
    StringTable = 0x21,     // { uint16_t size, uint8_t data[size] }
    ObjectCount = 0x22,     // { uint16_t size = 2, uint16_t count }
                            // Must be immediately followed by object
    
    // Function
    Locals = 0x31,          // { uint16_t nparams, uint16_t atoms[nparams] }
    ParamEnd = 0x32,        // { uint16_t size = 2, uint16_t paramEnd }
    Code = 0x33,            // { uint16_t size, uint8_t code[size] }
};

}

#endif