#include <iostream>
#include <memory>
#include "db/db.h"
#include "db/options.h"
using namespace smallkv;

int main() {
    // Create options
    auto test_options = MakeOptionsForDebugging();
    auto db_holder = std::make_unique<DB>(test_options);
    // Use the DB instance
    smallkv::WriteOptions write_options;
    smallkv::DBStatus s = db_holder->Put(write_options, "key", "value");
    std::string* result = new std::string();
    DBStatus get_status = db_holder->Get(smallkv::ReadOptions(), "key", result);
    if (get_status == smallkv::Status::Success) {
        std::cout << "Get key: " << *result << std::endl;
    } else {
        std::cerr << "Error getting key: "  << std::endl;
    }
    if (s == smallkv::Status::Success) {
        std::cout << "Successfully wrote key-value pair" << std::endl;
    } else {
        std::cerr << "Error writing key-value pair: "  << std::endl;
    }
    
    return 0;
}