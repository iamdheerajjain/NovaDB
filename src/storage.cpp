#include "storage.hpp"
#include <iostream>
#include <filesystem>

using namespace NovaDB;

Storage::Storage(const std::string& fname) : filename(fname) {
    bool file_exists = std::filesystem::exists(filename);
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file_exists) {
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    if (!file) {
        std::cerr << "Failed to open database file.\n";
        exit(1);
    }
}

Storage::~Storage() {
    if (file.is_open()) {
        file.close();
    }
}

void Storage::insert(const Record& record) {
    file.seekp(0, std::ios::end);
    file.write(reinterpret_cast<const char*>(&record), sizeof(Record));
    file.flush();
}

std::vector<Record> Storage::readAll() {
    std::vector<Record> records;
    file.seekg(0, std::ios::beg);
    Record temp;

    while (file.read(reinterpret_cast<char*>(&temp), sizeof(Record))) {
        records.push_back(temp);
    }

    return records;
}
