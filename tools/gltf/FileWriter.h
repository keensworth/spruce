#pragma once

#include <string>
#include <fstream>

namespace spr::tools{
class FileWriter {
public:
    FileWriter(){}
    ~FileWriter(){}

    void writeFile(std::string path, std::string name, std::string extension, unsigned char* data, int byteCount){
        auto myfile = std::fstream(path+name+extension, std::ios::out | std::ios::binary);
        myfile.write(reinterpret_cast<char*>(data), byteCount);
        myfile.close();
    }

    void writeFile(std::string path, std::string name, std::string extension, char* data, int byteCount){
        auto myfile = std::fstream(path+name+extension, std::ios::out | std::ios::binary);
        myfile.write(data, byteCount);
        myfile.close();
    }
private:
};
}