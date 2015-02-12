#include "query_diff.hpp"
#include <algorithm>

namespace mx3 { namespace sqlite {

template<typename T>
static vector<ListChange> _calculate_diff(
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

vector<ListChange> calculate_diff(
    const vector<Row>& old_list,
    const vector<Row>& new_list,
    const function<bool(const Row&, const Row&)>& is_same_entity,
    const function<bool(const Row&, const Row&)>& less_than)
{
    // supress updates only if the rows are exactly equal
    const function<bool(const Row&, const Row&)> should_supress_update = std::equal_to<Row>{};
    return _calculate_diff(old_list, new_list, is_same_entity, less_than, should_supress_update);
}

vector<ListChange> calculate_diff(
    const vector<Row>& old_list,
    const vector<Row>& new_list,
    const function<bool(const Row&, const Row&)>& is_same_entity,
    const function<bool(const Row&, const Row&)>& less_than,
    const function<bool(const Row&, const Row&)>& should_supress_update)
{
    return _calculate_diff(old_list, new_list, is_same_entity, less_than, should_supress_update);
}

} } // end namespace mx3::sqlite
