#include "query_diff.hpp"
#include <algorithm>

namespace mx3 { namespace sqlite {

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
