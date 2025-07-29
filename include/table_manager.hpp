#ifndef TABLE_MANAGER_HPP
#define TABLE_MANAGER_HPP

#include "schema.hpp"
#include <string>

namespace NovaDB {

class TableManager {
private:
    TableSchema schema;
    std::string bin_filename;

public:
    TableManager();
    TableManager(const TableSchema& schema);

    void insertInteractive();
    void displayAll();
    void deleteRow();
    void deleteTable();
    void showTables();
};
}

#endif

