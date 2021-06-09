#include <iomanip>
#include <utility>
#include "solution.hpp"

Solution::Solution(shared_ptr<Instance> instance, unordered_set<uint_t> p_locations) {
    this->instance = std::move(instance);
    this->p_locations = std::move(p_locations);
    fullCapEval();
}

vector<pair<uint_t, dist_t>> Solution::getUrgencies() {
    vector<pair<uint_t, dist_t>> urgencies_vec;

    // get closest and second closest p location with some remaining capacity
    for (auto p:cust_satisfactions) {
        auto cust = p.first;
        auto sat = p.second;                                // cust satisfaction
        auto dem = instance->getCustWeight(cust) - sat;     // cust remaining demand
        if (sat < dem) { // cust not fully satisfied yet
            auto l1 = getClosestOpenpLoc(cust, numeric_limits<uint_t>::max());
            auto l2 = getClosestOpenpLoc(cust, l1);
            auto dist1 = instance->getRealDist(l1, cust);
            auto dist2 = instance->getRealDist(l2, cust);
            dist_t urgency = fabs(dist1 - dist2);
            urgencies_vec.emplace_back(make_pair(cust, urgency));
        }
    }

    // Sort customers by decreasing urgencies
    sort(urgencies_vec.begin(), urgencies_vec.end(), cmpPair2nd);
    reverse(urgencies_vec.begin(), urgencies_vec.end()); // high to low now

    return urgencies_vec;
}

void Solution::fullCapEval() {
    // Initialize all fields
    objective = 0;
    for (auto p_loc:this->p_locations) loc_usages[p_loc] = 0;
    for (auto cust:this->instance->getCustomers()) {
        cust_satisfactions[cust] = 0;
        assignments[cust] = assignment{};
    }

    // Check if capacity demands can be met
    uint_t total_capacity = 0;
    uint_t total_demand = instance->getTotalDemand();
    for (auto p_loc:p_locations) total_capacity += instance->getLocCapacity(p_loc);
    if (total_capacity < total_demand) {
        fprintf(stderr, "Total capacity (%i) < total demand (%i)\n", total_capacity, total_demand);
        exit(1);
    }


    // Determine unassigned customer's urgencies
    auto urgencies_vec = getUrgencies();
    bool location_full = false;

    while (!urgencies_vec.empty()) {
        // Assign customers, until some capacity is full
        for (auto p:urgencies_vec) {
            auto cust = p.first;
            auto dem_rem = instance->getCustWeight(cust) - cust_satisfactions[cust]; // remaining demand
            while (dem_rem > 0) {
                auto loc = getClosestOpenpLoc(cust, numeric_limits<uint_t>::max());
                if (loc == numeric_limits<uint_t>::max()) {
                    cerr << "Assignment not possible\n";
                    exit(1);
                }
                auto cap_rem = instance->getLocCapacity(loc) - loc_usages[loc];
                if (dem_rem > cap_rem) { // assign all remaining location capacity
                    loc_usages[loc] += cap_rem;
                    cust_satisfactions[cust] += cap_rem;
                    auto obj_increment = cap_rem * instance->getRealDist(loc, cust);
                    objective += obj_increment;
                    assignments[cust].emplace_back(my_tuple{loc, cap_rem, obj_increment});
                    dem_rem -= cap_rem;
                    location_full = true;
                    break;
                } else { // assign dem_rem
                    loc_usages[loc] += dem_rem;
                    cust_satisfactions[cust] += dem_rem;
                    auto obj_increment = dem_rem * instance->getRealDist(loc, cust);
                    objective += obj_increment;
                    assignments[cust].emplace_back(my_tuple{loc, dem_rem, obj_increment});
                    dem_rem = 0;
                }
            }
            if (location_full) break;
        }

        // Recompute urgencies and repeat (for unassigned customers and open locations only)
        location_full = false;
        urgencies_vec = getUrgencies();
    }


}

uint_t Solution::getClosestpLoc(uint_t cust) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    dist_t dist;
    uint_t loc_closest;
    for (auto loc:p_locations) {
        dist = instance->getWeightedDist(loc, cust);
        if (dist <= dist_min) {
            dist_min = dist;
            loc_closest = loc;
        }
    }

    return loc_closest;
}

uint_t Solution::getClosestOpenpLoc(uint_t cust, uint_t forbidden_loc) {
    dist_t dist_min = numeric_limits<dist_t>::max();
    dist_t dist;
    uint_t loc_closest = numeric_limits<uint_t>::max();
    for (auto loc:p_locations) {
        dist = instance->getRealDist(loc, cust);
        if (dist <= dist_min && loc_usages[loc] < instance->getLocCapacity(loc) && loc != forbidden_loc) {
            dist_min = dist;
            loc_closest = loc;
        }
    }
    return loc_closest;
}

void Solution::print() {
    cout << "p locations: ";
    for (auto p:p_locations) {
        cout << p << " ";
    }
//    for (auto cust:instance->getCustomers()) {
//        cout << cust << " " << assignment[cust].node << " " << assignment[cust].dist << endl;
//    }
    cout << endl;
    cout << setprecision(15) << "objective: " << objective << endl;
}

const unordered_set<uint_t> &Solution::get_pLocations() const {
    return this->p_locations;
}

void Solution::replaceLocation(uint_t loc_old, uint_t loc_new) {
    // Update p_locations
    p_locations.erase(loc_old);
    p_locations.insert(loc_new);
    // Update assignment and objective
    fullCapEval();
}

dist_t Solution::get_objective() const {
    return objective;
}

void Solution::printAssignment() {
    cout << setprecision(15) << "OBJECTIVE\n" << objective << endl << endl;

    cout << "P LOCATIONS\n";
    for (auto p_loc:p_locations) printf("%i\n", p_loc);
    cout << endl;

    cout << "LOCATION USAGES\nlocation (usage/capacity)\n";
    for (auto p_loc:p_locations) printf("%i (%i/%i)\n", p_loc, loc_usages[p_loc], instance->getLocCapacity(p_loc));
    cout << endl;

    cout << "CUSTOMER ASSIGNMENTS\ncustomer (demand/satisfaction) -> location (usage/capacity)\n";
    for (auto cust:instance->getCustomers()) {
        cout << cust << " (" << cust_satisfactions[cust] << "/" << instance->getCustWeight(cust) << ") -> ";
        for (auto a:assignments[cust]) printf("%i (%i/%i) ", a.node, a.usage, instance->getLocCapacity(a.node));
        cout << endl;
    }
}

//void Solution::exportSolution(const string& output_filename) {
//    ofstream output_file(output_filename);
//    if (output_file.is_open()) {
//        output_file << "p_locations\n";
//        for (auto loc:p_locations) output_file << loc << endl;
//        output_file << endl;
//        output_file << "assignment\n";
//        for (auto loc:p_locations) {
//            output_file << loc << ": ";
//            for (auto cust:instance->getCustomers()) {
//                if (assignment[cust].node == loc) {
//                    output_file << cust << " ";
//                }
//            }
//            output_file << endl;
//        }
//        output_file << endl;
//        output_file << "distances\n";
//        for (auto loc:p_locations) {
//            output_file << loc << ": ";
//            for (auto cust:instance->getCustomers()) {
//                if (assignment[cust].node == loc) {
//                    output_file << cust << " (" << assignment[cust].dist << ") ";
//                }
//            }
//            output_file << endl;
//        }
//    } else {
//        cerr << "Error while opening output file\n";
//    }
//}






