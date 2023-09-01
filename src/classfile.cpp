#include "classfile.h"

#include <cstdio>

void JVMClass::Ldc(uint8_t *ptr, size_t &start_pos)
{
    uint8_t index = ptr[start_pos++];

    StackObject obj;
    obj.isConst = true;
    obj.constPoolIndex = index;
    stack.push_back(obj);

    printf("ldc #%d\n", index);
}

void JVMClass::GetStatic(uint8_t *ptr, size_t &start_pos)
{
    uint16_t index = (ptr[start_pos++] << 8);
    index |= ptr[start_pos++];

    StackObject obj;
    obj.isConst = true;
    obj.constPoolIndex = index;
    stack.push_back(obj);

    printf("getstatic %d\n", index);
}

JVMClass::JVMClass(uint8_t *data)
    : buf(data)
{
    header.magic = Read32();
    header.minor_version = Read16();
    header.major_version = Read16();
    header.constantpool_count = Read16();

    printf("%d entries in constant pool\n", header.constantpool_count);

    for (uint16_t i = 0; i < (header.constantpool_count-1); i++)
    {
        ConstantPoolEntry constPoolEntry;
        constPoolEntry.tag = *buf;
        buf++;

        printf("%d: ", i+1);

        switch (constPoolEntry.tag)
        {
        case 1:
        {
            constPoolEntry.utf8_string.len = Read16();
            for (int j = 0; j < constPoolEntry.utf8_string.len; j++)
                constPoolEntry.utf8_string.str.push_back(*buf++);
            printf("Found utf8 string \"%s\"\n", constPoolEntry.utf8_string.str.c_str());
            break;
        }
        case 3:
            constPoolEntry.constInteger.bytes = Read32();
            printf("Found integer constant with value = 0x%08x\n", constPoolEntry.constInteger.bytes);
            break;
        case 7:
            constPoolEntry.classref.name_index = Read16();
            printf("Found classdef with name index = %d\n", constPoolEntry.classref.name_index);
            break;
        case 8:
            constPoolEntry.stringref.utf8_index = Read16();
            printf("Found const string with index = %d\n", constPoolEntry.stringref.utf8_index);
            break;
        case 9:
            constPoolEntry.fieldRef.class_index = Read16();
            constPoolEntry.fieldRef.name_and_type_index = Read16();
            printf("Found fieldRef with class index %d and name/type index %d\n", constPoolEntry.fieldRef.class_index, constPoolEntry.fieldRef.name_and_type_index);
            break;
        case 10:
            constPoolEntry.methodref.class_index = Read16();
            constPoolEntry.methodref.name_and_type_index = Read16();
            printf("Found methodref with class index %d and name/type index %d\n", constPoolEntry.methodref.class_index, constPoolEntry.methodref.name_and_type_index);
            break;
        case 12:
            constPoolEntry.nameandtype.name_index = Read16();
            constPoolEntry.nameandtype.descriptor_index = Read16();
            printf("Found nameandtype with name index=%d, descriptor index=%d\n", constPoolEntry.nameandtype.name_index, constPoolEntry.nameandtype.descriptor_index);
            break;
        default:
            printf("Unknown constant pool tag %d\n", constPoolEntry.tag);
            exit(1);
        }

        constantPool.push_back(constPoolEntry);
    }

    access_flags = Read16();
    this_class = Read16();
    super_class = Read16();

    printf("Access flags: 0x%04x\n", access_flags);
    printf("this class: 0x%04x\n", this_class);
    printf("super class: 0x%04x\n", super_class);

    uint16_t interfaces_count = Read16();
    for (uint16_t i = 0; i < interfaces_count; i++)
    {
        printf("TODO: Interfaces\n");
        exit(1);
    }

    uint16_t fields_count = Read16();
    for (uint16_t i = 0; i < fields_count; i++)
    {
        printf("TODO: Fields\n");
        exit(1);
    }

    uint16_t method_count = Read16();
    for (int i = 0; i < method_count; i++)
    {
        MethodInfo method;
        method.access_flags = Read16();
        method.name_index = Read16();
        method.descriptor_index = Read16();
        method.attributes_count = Read16();
        printf("Found method with access flags=0x%04x, name index=%d, descriptor index=%d, %d attributes\n", method.access_flags, method.name_index, method.descriptor_index, method.attributes_count);
        for (int j = 0; j < method.attributes_count; j++)
        {
            MethodAttrInfo attr_info;
            attr_info.attr_name_index = Read16();
            attr_info.attr_length = Read32();
            for (int k = 0; k < attr_info.attr_length; k++)
                attr_info.attr.push_back(*buf++);
            method.attributes.push_back(attr_info);
            printf("Found attribute with name index=%d, length=%d\n", attr_info.attr_name_index, attr_info.attr_length);
        }

        methods.push_back(method);
    }
}

bool JVMClass::HasFunction(std::string name)
{
    for (auto& m : methods)
    {
        if (constantPool[m.name_index-1].utf8_string.str == name)
            return true;
    }

    return false;
}

bool JVMClass::RunFunction(std::string name)
{
    if (!HasFunction(name))
        return false;
    
    MethodInfo* method;
    for (auto& m : methods)
    {
        if (constantPool[m.name_index-1].utf8_string.str == name)
        {
            method = &m;
            break;
        }
    }

    if (!method)
    {
        printf("No method named %s found\n", name.c_str());
        return false;
    }
    
    std::vector<uint8_t> code;

    // Find the method attribute with the name "CODE"
    for (uint16_t i = 0; i < method->attributes_count; i++)
    {
        MethodAttrInfo* info = &method->attributes[i];
        if (constantPool[info->attr_name_index-1].utf8_string.str == "Code")
            code = info->attr;
    }

    if (code.empty())
    {
        printf("Empty code attribute\n");
        return false;
    }

    printf("Code is %ld bytes\n", code.size());

    CodeHeader* hdr = (CodeHeader*)code.data();

    uint8_t* codebuf = (uint8_t*)(code.data() + sizeof(CodeHeader));
    
    for (size_t i = 0; i < hdr->code_length;)
    {
        uint8_t opcode = codebuf[i++];

        switch (opcode)
        {
        case 0xb2:
            GetStatic(codebuf, i);
            break;
        case 0x12:
            Ldc(codebuf, i);
            break;
        default:
            printf("ERROR: Unknown opcode 0x%02x\n", opcode);
            exit(1);
        }
    }
}
