#include "schema.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace NovaDB;

TableSchema::TableSchema() : table_name(""), columns() {}
TableSchema::TableSchema(const std::string& name) : table_name(name) {}

void TableSchema::addColumn(const std::string& name, ColumnType type, size_t size) {
    columns.push_back({name, type, size});
}

bool TableSchema::save() {
    std::ofstream meta("data/" + table_name + ".meta", std::ios::binary);
    if (!meta) return false;

    uint32_t num_columns = columns.size();
    meta.write(reinterpret_cast<const char*>(&num_columns), sizeof(num_columns));
    for (const auto& col : columns) {
        uint32_t name_len = col.name.size();
        meta.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        meta.write(col.name.c_str(), name_len);

        int type = static_cast<int>(col.type);
        meta.write(reinterpret_cast<const char*>(&type), sizeof(type));
        meta.write(reinterpret_cast<const char*>(&col.size), sizeof(col.size));
    }
    meta.close();

    fs::create_directories("schemas");
    std::ofstream schema_file("schemas/" + table_name + ".schema");
    if (!schema_file) return false;

    for (const auto& col : columns) {
        schema_file << col.name << "," 
                    << (col.type == ColumnType::INT ? "INT" : "STRING") << "," 
                    << col.size << "\n";
    }
    schema_file.close();

    return true;
}

TableSchema TableSchema::load(const std::string& table_name) {
    TableSchema schema(table_name);
    std::ifstream meta("data/" + table_name + ".meta", std::ios::binary);
    if (!meta) throw std::runtime_error("Schema not found!");

    uint32_t num_columns;
    meta.read(reinterpret_cast<char*>(&num_columns), sizeof(num_columns));

    for (uint32_t i = 0; i < num_columns; ++i) {
        uint32_t name_len;
        meta.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
        std::string colname(name_len, ' ');
        meta.read(&colname[0], name_len);

        int type_int;
        ColumnType type;
        size_t size;

        meta.read(reinterpret_cast<char*>(&type_int), sizeof(type_int));
        type = static_cast<ColumnType>(type_int);

        meta.read(reinterpret_cast<char*>(&size), sizeof(size));

        schema.addColumn(colname, type, size);
    }

    return schema;
}

size_t TableSchema::getRowSize() const {
    size_t size = 0;
    for (const auto& col : columns) {
        if (col.type == ColumnType::INT)
            size += sizeof(int);
        else
            size += col.size;
    }
    return size;
}
