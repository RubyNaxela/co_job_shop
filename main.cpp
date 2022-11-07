#include <iostream>
#include "dataset.hpp"
#include "schedule.hpp"

// http://www.cs.put.poznan.pl/mdrozdowski/dyd/ok/index.html

int main() {
    js::dataset data = js::dataset::from_file("data.txt");
    js::schedule schedule;
    return 0;
}
