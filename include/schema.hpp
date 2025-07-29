#ifndef SCHEMA_HPP
#define SCHEMA_HPP

#include <string>
#include <vector>
#include <map>

namespace NovaDB {

enum class ColumnType {
    INT,
    STRING
};

struct Column {
    std::string name;
    ColumnType type;
    size_t size;
};

class TableSchema {
public:
    std::string table_name;
    std::vector<Column> columns;

    size_t getRowSize() const;
    TableSchema();
    TableSchema(const std::string& name);
    void addColumn(const std::string& name, ColumnType type, size_t size = 0);
    bool save();
    static TableSchema load(const std::string& table_name);
};
}

#endif
