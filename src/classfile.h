#pragma once

#include <cstdint>
#include <vector>
#include <cstdlib>
#include <string>

struct ClassFile
{
    uint32_t magic;
    uint16_t minor_version;
    uint16_t major_version;
    uint16_t constantpool_count;
};

struct StackObject
{
    bool isConst;
    int constPoolIndex;
};

class JVMClass
{
private:
    uint8_t* buf;
    ClassFile header;
    struct ConstantPoolEntry
    {
        uint8_t tag;
        struct
        {
            uint16_t class_index;
            uint16_t name_and_type_index;
        } methodref;
        struct
        {
            uint16_t name_index;
        } classref;
        struct
        {
            uint16_t name_index;
            uint16_t descriptor_index;
        } nameandtype;
        struct
        {
            uint16_t len;
            std::string str;
        } utf8_string;
        struct
        {
            uint16_t utf8_index;
        } stringref;
        struct
        {
            uint32_t bytes;
        } constInteger;
        struct
        {
            uint16_t class_index;
            uint16_t name_and_type_index;
        } fieldRef;
    };

    std::vector<ConstantPoolEntry> constantPool;
    uint16_t access_flags;
    uint16_t this_class;
    uint16_t super_class;

    std::vector<StackObject> stack;

    struct MethodAttrInfo
    {
        uint16_t attr_name_index;
        uint32_t attr_length;
        std::vector<uint8_t> attr;
    };

    struct MethodInfo
    {
        uint16_t access_flags;
        uint16_t name_index;
        uint16_t descriptor_index;
        uint16_t attributes_count;
        std::vector<MethodAttrInfo> attributes;
    };

    struct CodeHeader
    {
        uint16_t max_stack;
        uint16_t max_locals;
        uint32_t code_length;
    };

    std::vector<MethodInfo> methods;

    uint32_t Read32()
    {
        uint32_t data = __bswap_32(*(uint32_t*)buf);
        buf += 4;
        return data;
    }

    uint16_t Read16()
    {
        uint16_t data = __bswap_16(*(uint16_t*)buf);
        buf += 2;
        return data;
    }

    void Ldc(uint8_t* ptr, size_t& start_pos); // 0x12
    void GetStatic(uint8_t* ptr, size_t& start_pos); // 0xb2
public:
    JVMClass(uint8_t* buf);

    bool HasFunction(std::string name);
    bool RunFunction(std::string name);
};