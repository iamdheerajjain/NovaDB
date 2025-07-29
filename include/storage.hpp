#ifndef STORAGE_HPP
#define STORAGE_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>

namespace NovaDB {

struct Record {
    int id;
    char name[32];
};

class Storage {
private:
    std::string filename;
    std::fstream file;

public:
    Storage(const std::string& filename);
    ~Storage();

    void insert(const Record& record);
    std::vector<Record> readAll();
};
}

#endif
