#pragma once
#include "stl.hpp"
#include "../sqlite/value.hpp"

namespace mx3 {
namespace sqlite {

using Row = vector<Value>;

/* A representation of a change between 2 different row-based sets of data.
 */
struct ListChange final {
    // The index in the old data set or -1 if it is an insert.
    // This index refers to the index in the old data set.
    int32_t from_index;
    // The index in the new data set or -1 if it is a delete.
    // This index refers to the index in the new data set.
    int32_t to_index;
};

/* Calculate the diffs between 2 data sets.  The changes will be returned in such an order that
 * they could be applied incrementally.  Deletes (x, -1), inserts (-1, x), then updates (x, y).
 *
 * old_list - the previous (stale) data.  Must be sorted by the `less_than` parameter
 * new_list - the new data.  Must also be sorted by the `less_than` parameter
 * is_same_entity - a function determining whether 2 rows are the same entity
 *     (generally same primary key)
 * less_than - the sort order of rows in the 2 data sets
 * should_suppress_update - for an update ListChange, whether this update should be ignored.  This
 *      would possibly be useful to filter updates which are metadata-only changes.
 */
vector<ListChange> calculate_diff(
    const vector<Row>& old_list,
    const vector<Row>& new_list,
    const function<bool(const Row&, const Row&)>& is_same_entity,
    const function<bool(const Row&, const Row&)>& less_than,
    const function<bool(const Row&, const Row&)>& should_suppress_update);

// See above for docs. Allows all updates through.
vector<ListChange> calculate_diff(
    const vector<Row>& old_list,
    const vector<Row>& new_list,
    const function<bool(const Row&, const Row&)>& is_same_entity,
    const function<bool(const Row&, const Row&)>& less_than);

} } // end namespace mx3::sqlite
