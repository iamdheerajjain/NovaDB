#include "storage.hpp"
#include "schema.hpp"
#include "table_manager.hpp"
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <algorithm>

namespace fs = std::filesystem;
using namespace NovaDB;

void createTableCLI() {
    std::string table_name;
    std::cout << "Enter table name: ";
    std::cin >> table_name;

    TableSchema schema(table_name);

    int col_count;
    std::cout << "Number of columns: ";
    std::cin >> col_count;

    for (int i = 0; i < col_count; ++i) {
        std::string name, type;
        size_t size = 0;
        std::cout << "Column " << (i + 1) << " name: ";
        std::cin >> name;

        std::cout << "Type (INT or STRING): ";
        std::cin >> type;

        ColumnType colType;
        if (type == "INT") {
            colType = ColumnType::INT;
        } else if (type == "STRING") {
            colType = ColumnType::STRING;
            std::cout << "Max string size: ";
            std::cin >> size;
        } else {
            std::cout << "Unsupported type. Try again.\n";
            --i;
            continue;
        }

        schema.addColumn(name, colType, size);
    }

    if (schema.save()) {
        std::cout << "\u2705 Table schema saved successfully.\n";
    } else {
        std::cout << "\u274C Failed to save schema.\n";
    }
}

void tableInsertCLI() {
    std::string table_name;
    std::cout << "Enter table name: ";
    std::cin >> table_name;

    try {
        TableSchema schema = TableSchema::load(table_name);
        TableManager tm(schema);
        tm.insertInteractive();
    } catch (...) {
        std::cout << "\u274C Table not found or invalid schema.\n";
    }
}

void tableViewCLI() {
    std::string table_name;
    std::cout << "Enter table name: ";
    std::cin >> table_name;

    try {
        TableSchema schema = TableSchema::load(table_name);
        TableManager tm(schema);
        tm.displayAll();
    } catch (...) {
        std::cout << "\u274C Table not found or invalid schema.\n";
    }
}

void deleteRowCLI() {
    std::string tableName;
    std::cout << "Enter table name: ";
    std::cin >> tableName;

    try {
        TableSchema schema = TableSchema::load(tableName);
        TableManager manager(schema);
        manager.deleteRow();
    } catch (...) {
        std::cerr << "Failed to load table.\n";
    }
}

void deleteTableCLI() {
    std::string tableName;
    std::cout << "Enter table name to delete: ";
    std::cin >> tableName;

    std::string metaPath = "data/" + tableName + ".meta";
    std::string dataPath = "data/" + tableName + ".data";

    if (!std::filesystem::exists(metaPath)) {
        std::cout << "❌ No such table found.\n";
        return;
    }

    try {
        std::filesystem::remove(metaPath);
        if (std::filesystem::exists(dataPath))
            std::filesystem::remove(dataPath);

        std::cout << "✅ Table '" << tableName << "' deleted successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << "⚠️ Error deleting table files: " << e.what() << '\n';
    }
}


int main() {
    std::filesystem::create_directories("data/");
    std::filesystem::create_directories("schemas/");

    NovaDB::TableManager manager;
    int choice;
    do {
        std::cout << "\n\U0001F537 NovaDB Menu \U0001F537\n";
        std::cout << "1. Create Table\n";
        std::cout << "2. Insert Row\n";
        std::cout << "3. Insert Record\n";
        std::cout << "4. Show All Tables\n";
        std::cout << "5. View Table Data\n";
        std::cout << "6. View all Records\n";
        std::cout << "7. Delete Table\n";
        std::cout << "8. Delete Row from Table\n";
        std::cout << "0. Exit\n";
        std::cout << "Choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                createTableCLI();
                break;

            case 2:
                tableInsertCLI();
                break;

            case 3: {
                Storage db("data/users.bin");
                Record r;
                std::cout << "Enter ID: ";
                std::cin >> r.id;
                std::cout << "Enter Name (max 31 chars): ";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.getline(r.name, sizeof(r.name));
                db.insert(r);
                std::cout << "\u2705 Record inserted.\n";
                break;
            }

            case 4:
                manager.showTables();
                break;

            case 5:
                tableViewCLI();
                break;

            case 6: {
              std::cout << "🔍 Viewing all records (Fixed Schema)..." << std::endl;

              std::ifstream file("data/users.bin", std::ios::binary);
              NovaDB::Record record;

              if (!file) {
                  std::cerr << "❌ Failed to open users.bin file.\n";
                  break; {
                    
                  }
              }

              bool found = false;
              while (file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
                  std::cout << "ID: " << record.id << ", Name: " << record.name << std::endl;
                  found = true;
              }

              if (!found) {
                  std::cout << "📭 No records found.\n";
              }

              file.close();
              break;
            }

            case 7:
                deleteTableCLI();
                break;

            case 8:
                deleteRowCLI();
                break;

        }
    } while (choice != 0);

    std::cout << "\U0001F44B Exiting NovaDB. Goodbye!\n";
    return 0;
}
