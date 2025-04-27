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
    
    if (s == smallkv::Status::Success) {
        std::cout << "Successfully wrote key-value pair" << std::endl;
    } else {
        std::cerr << "Error writing key-value pair: "  << std::endl;
    }
    
    return 0;
}