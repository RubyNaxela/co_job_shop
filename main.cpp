#include <iostream>
#include "dataset.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main() {
    js::dataset data = js::dataset::from_file("data.txt");
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
