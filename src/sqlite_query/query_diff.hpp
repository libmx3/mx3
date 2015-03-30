#pragma once
#include "stl.hpp"
#include "../sqlite/value.hpp"
#include <algorithm>

namespace mx3 {
namespace sqlite {

/* A representation of a change between 2 different row-based sets of data.
 */
struct ListChange final {
    // The index in the old data set or -1 if it is an insert.
    // This index refers to the index in the old data set.
    int32_t from_index;
    // The index in the new data set or -1 if it is a delete.
    // This index refers to the index in the new data set.
    int32_t to_index;
    friend bool operator==(const ListChange& a, const ListChange& b);
};

// A comparator for ListChanges which order them 'naturally'.
// The changes will be returned in such an order that they could be applied incrementally.
//
// * deletes highest -> lowest
// * inserts lowest -> highest
// * updates lowest -> highest
// * Deletes (x, -1), inserts (-1, y), then updates (x, y).
bool incremental_consistent_order(const ListChange& a, const ListChange& b);

/* Calculate the diffs between 2 data sets.
 * The changes will be ordered in `consistent_order`.  See above.
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

template<typename T>
vector<ListChange> calculate_diff(
    const vector<T>& old_list,
    const vector<T>& new_list,
    const function<bool(const T&, const T&)>& is_same_entity,
    const function<bool(const T&, const T&)>& less_than,
    const function<bool(const T&, const T&)>& should_suppress_update)
{
    const bool old_is_sorted = std::is_sorted(old_list.begin(), old_list.end(), less_than);
    const bool new_is_sorted = std::is_sorted(new_list.begin(), new_list.end(), less_than);
    if (!old_is_sorted) {
        throw std::invalid_argument {"`old_list` param must be sorted"};
    }
    if (!new_is_sorted) {
        throw std::invalid_argument {"`new_list` param must be sorted"};
    }

    const int32_t old_size = static_cast<int32_t>(old_list.size());
    const int32_t new_size = static_cast<int32_t>(new_list.size());

    vector<ListChange> deletes;
    vector<ListChange> inserts;
    vector<ListChange> updates;

    int32_t old_pos = 0;
    int32_t new_pos = 0;
    while (old_pos < old_size && new_pos < new_size) {
        const auto& a = old_list[old_pos];
        const auto& b = new_list[new_pos];
        if (is_same_entity(a, b)) {
            if (!should_suppress_update(a, b)) {
                updates.push_back({old_pos, new_pos});
            }
            old_pos++;
            new_pos++;
        } else if (less_than(a, b)) {
            deletes.push_back({old_pos, -1});
            old_pos++;
        } else if (less_than(b, a)) {
            inserts.push_back({-1, new_pos});
            new_pos++;
        } else if (should_suppress_update(a, b)) {
            old_pos++;
            new_pos++;
        } else {
            updates.push_back({old_pos, new_pos});
            old_pos++;
            new_pos++;
        }
    }

    // Since `deletes` will eventually be the basis for the final output we can reserve the space
    // right now.
    deletes.reserve(
        // Deletes that have been, and will eventually be added
        deletes.size() +
        (old_size - old_pos) +

        // Insert that have been, and will eventually be added
        inserts.size() +
        (new_size - new_pos) +

        updates.size());

    while (old_pos < old_size) {
        deletes.push_back({old_pos, -1});
        old_pos++;
    }
    while (new_pos < new_size) {
        inserts.push_back({-1, new_pos});
        new_pos++;
    }

    // Capture deletes size before moving it into changes.
    const size_t deletes_size = deletes.size();
    // The first elements of changes are the deletes in reverse, put them in that order now.
    std::reverse(deletes.begin(), deletes.end());

    // Reuse the memory from the deletes as the basis for changes.
    vector<ListChange> changes = std::move(deletes);
    // Reserve capacity for the known size of the output.
    changes.resize(changes.size() + inserts.size() + updates.size());
    const auto after_deletes = changes.begin() + deletes_size;
    const auto after_inserts = std::copy(inserts.begin(), inserts.end(), after_deletes);
    std::copy(updates.begin(), updates.end(), after_inserts);
    return changes;
}


} } // end namespace mx3::sqlite
