#include "classfile.h"
#include <fstream>
#include <filesystem>

int main()
{
    for (auto& entry : std::filesystem::recursive_directory_iterator("java_/java"))
    {
        if (entry.path().extension() != ".class")
            continue;
        
        printf("Loading %s\n", entry.path().c_str());

        std::ifstream file(entry.path().string(), std::ios::binary | std::ios::ate);
        size_t size = file.tellg();
        uint8_t* data = new uint8_t[size];
        file.seekg(0, std::ios::beg);
        file.read((char*)data, size);
        file.close();
        JVMClass jvm(data);
    }

    std::ifstream file("Add.class", std::ios::binary | std::ios::ate);
    size_t size = file.tellg();
    uint8_t* data = new uint8_t[size];
    file.seekg(0, std::ios::beg);
    file.read((char*)data, size);
    file.close();
    JVMClass add(data);

    // Find the main() function
    if (!add.HasFunction("main"))
    {
        printf("ERROR: No main function found in classpath\n");
        exit(1);
    }

    add.RunFunction("main");

    return 0;
}