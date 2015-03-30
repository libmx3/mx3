#include "query_diff.hpp"

namespace mx3 { namespace sqlite {

// Computes a ordering class for ListChange (deletes, inserts, or deletes)
static inline int ord_class(const ListChange& c) {
    if (c.to_index == -1) {
        // deletes
        return -1;
    } else if (c.from_index == -1) {
        // inserts
        return 0;
    }
    // updates
    return 1;
}

bool incremental_consistent_order(const ListChange& a, const ListChange& b) {
    const int a_ord = ord_class(a);
    const int b_ord = ord_class(b);
    // If they are different classes, order them coarsely (deletes, inserts, updates)
    if (a_ord != b_ord) {
        return a_ord < b_ord;
    }

    if (a.to_index == -1) {
        return a.from_index > b.from_index;
    } else {
        return a.to_index < b.to_index;
    }
}

bool operator==(const ListChange& a, const ListChange& b) {
    return a.from_index == b.from_index && a.to_index == b.to_index;
}

vector<ListChange> calculate_diff(
    const vector<Row>& old_list,
    const vector<Row>& new_list,
    const function<bool(const Row&, const Row&)>& is_same_entity,
    const function<bool(const Row&, const Row&)>& less_than)
{
    // supress updates only if the rows are exactly equal
    const function<bool(const Row&, const Row&)> should_supress_update = std::equal_to<Row>{};
    return calculate_diff<Row>(old_list, new_list, is_same_entity, less_than, should_supress_update);
}

vector<ListChange> calculate_diff(
    const vector<Row>& old_list,
    const vector<Row>& new_list,
    const function<bool(const Row&, const Row&)>& is_same_entity,
    const function<bool(const Row&, const Row&)>& less_than,
    const function<bool(const Row&, const Row&)>& should_supress_update)
{
    return calculate_diff<Row>(old_list, new_list, is_same_entity, less_than, should_supress_update);
}

} } // end namespace mx3::sqlite
