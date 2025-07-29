#include "table_manager.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <sstream>
#include <dirent.h>
#include <set>
#include "schema.hpp"

namespace fs = std::filesystem;
using namespace NovaDB;

TableManager::TableManager(const TableSchema& schema)
    : schema(schema), bin_filename("data/" + schema.table_name + ".bin") {}

TableManager::TableManager() {
}

void TableManager::insertInteractive() {
    std::ofstream out(bin_filename, std::ios::app | std::ios::binary);
    if (!out) {
        std::cerr << "[Error] Failed to open data file for writing.\n";
        return;
    }

    for (const auto& col : schema.columns) {
        std::cout << col.name << " (" << (col.type == ColumnType::INT ? "INT" : "STRING") << "): ";
        if (col.type == ColumnType::INT) {
            int value;
            std::cin >> value;
            out.write(reinterpret_cast<char*>(&value), sizeof(int));
        } else {
            std::string value;
            std::cin.ignore();
            std::getline(std::cin, value);
            value.resize(col.size, '\0');
            out.write(value.c_str(), col.size);
        }
    }

    out.close();
    std::cout << "[Success] Row inserted into table '" << schema.table_name << "'\n";
}

void TableManager::displayAll() {
    std::ifstream in(bin_filename, std::ios::binary);
    if (!in) {
        std::cerr << "[Error] Failed to open data file for reading.\n";
        return;
    }

    size_t rowSize = schema.getRowSize();
    std::vector<char> buffer(rowSize);
    int rowNumber = 0;

    std::cout << "\n--- Contents of Table: " << schema.table_name << " ---\n";

    while (in.read(buffer.data(), rowSize)) {
        size_t offset = 0;
        for (const auto& col : schema.columns) {
            if (col.type == ColumnType::INT) {
                int value;
                std::memcpy(&value, buffer.data() + offset, sizeof(int));
                std::cout << col.name << ": " << value << "  ";
                offset += sizeof(int);
            } else {
                std::string strValue(buffer.data() + offset, col.size);
                strValue.erase(strValue.find('\0'));
                std::cout << col.name << ": " << strValue << "  ";
                offset += col.size;
            }
        }
    }
    if (rowNumber == 0) {
        std::cout << "[Info] No records found.\n";
    }
}

void TableManager::deleteRow() {
    if (schema.columns.empty()) {
        std::cout << "[Error] Schema is empty. Cannot delete.\n";
        return;
    }

    if (schema.columns[0].type != ColumnType::INT) {
        std::cout << "[Error] Only INT primary key (first column) is supported.\n";
        return;
    }

    int keyToDelete;
    std::cout << "Enter primary key value to delete: ";
    std::cin >> keyToDelete;

    std::ifstream inFile(bin_filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "[Error] Error opening file for reading.\n";
        return;
    }

    std::string tempFilename = bin_filename + ".temp";
    std::ofstream outFile(tempFilename, std::ios::binary);
    if (!outFile) {
        std::cerr << "[Error] Error opening temp file.\n";
        return;
    }

    size_t rowSize = schema.getRowSize();
    std::vector<char> buffer(rowSize);
    bool found = false;

    while (inFile.read(buffer.data(), rowSize)) {
        int currentKey;
        std::memcpy(&currentKey, buffer.data(), sizeof(int));
        if (currentKey == keyToDelete) {
            found = true;
            continue;
        }
        outFile.write(buffer.data(), rowSize);
    }

    inFile.close();
    outFile.close();

    if (found) {
        fs::remove(bin_filename);
        fs::rename(tempFilename, bin_filename);
        std::cout << "[Success] Row deleted successfully.\n";
    } else {
        fs::remove(tempFilename);
        std::cout << "[Info] Row not found.\n";
    }
}

void TableManager::deleteTable() {
    std::string tableName;
    std::cout << "Enter table name to delete: ";
    std::getline(std::cin >> std::ws, tableName);

    std::string schemaFile = "schemas/" + tableName + ".schema";
    std::string dataFile = "data/" + tableName + ".bin";

    bool schemaExists = fs::exists(schemaFile);
    bool dataExists = fs::exists(dataFile);

    if (!schemaExists && !dataExists) {
        std::cout << "[Info] Table '" << tableName << "' does not exist.\n";
        return;
    }

    if (schemaExists) {
        fs::remove(schemaFile);
        std::cout << "[Deleted] Schema file: " << schemaFile << "\n";
    }

    if (dataExists) {
        fs::remove(dataFile);
        std::cout << "[Deleted] Data file: " << dataFile << "\n";
    }

    std::cout << "[Success] Table '" << tableName << "' deleted.\n";
}

void TableManager::showTables() {
    std::string dataDir = "data/";
    DIR* dir = opendir(dataDir.c_str());
    if (!dir) {
        std::cout << "[Error] Failed to open data directory.\n";
        return;
    }

    std::set<std::string> tableNames;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;

        if (filename == "." || filename == "..") continue;

        size_t pos = filename.find_last_of('.');
        if (pos != std::string::npos) {
            std::string baseName = filename.substr(0, pos);
            tableNames.insert(baseName);
        }
    }

    closedir(dir);

    if (tableNames.empty()) {
        std::cout << "[Info] No tables found.\n";
    } else {
        std::cout << "[Info] Available Tables:\n";
        for (const auto& name : tableNames) {
            std::cout << " - " << name << "\n";
        }
    }
}
