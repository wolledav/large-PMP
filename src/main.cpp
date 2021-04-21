#include <set>
#include <iostream>
#include <unistd.h>
#include "instance.hpp"
#include "solution.hpp"
#include "TB.hpp"

using namespace std;

int main() {
    string loc_filename = "./provided/toulon/toulon_destination.txt";
    string cust_filename = "./provided/toulon/toulon_origin.txt";
    string dist_filename = "./provided/toulon/toulon_orig_x_tps.txt";

//    string loc_filename = "./provided/paca/global_destination.txt";
//    string cust_filename = "./provided/paca/global_origin.txt";
//    string dist_filename = "./provided/paca/global_orig_x_tps.txt";

    Instance instance(loc_filename, cust_filename, dist_filename, 5);

    default_random_engine generator;
    Instance instance1 = instance.sampleSubproblem(10, 10, 4, &generator);

//    cout << instance.getDist(690, 1) << endl; // Should return 0.226392658 for Toulon
//    cout << instance1.getDist(690, 1) << endl; // Should return 0.226392658 for Toulon

    instance.print();
    instance1.print();

//    unordered_set<uint_t> locations = {534, 529, 298, 580, 355}; // optimal solution, objective: 1112707.98040259
//    Solution solution(&instance, locations);
//    solution.print();

//    TB heuristic(&instance, 1);
//    auto sol = heuristic.run();
//    sol.print();

    return 0;
}