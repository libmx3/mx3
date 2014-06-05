#include "Mx3QueryResult.h"
#include "objc_adapter.hpp"
using mx3::ObjcAdapter;

@implementation MX3QueryResult {
    mx3::QueryResultPtr<github::User> __result;
}

- (instancetype) initWithResult:(mx3::QueryResultPtr<github::User>) result {
    self = [super init];
    if (self) {
        __result = result;
    }
    return self;
}

- (void) listenToChanges: (void (^)()) changeBlock {
    // todo(kabbes) I think we might need to Block_copy, Block_release this
    // but I'm unsure
    __result->on_change( [=] () {
        changeBlock();
    });
}

- (void) dealloc {
    __result = nullptr;
}

- (NSString *) rowAtIndex:(NSUInteger)index {
    auto row = __result->get_item_at(static_cast<size_t>(index));
    return ObjcAdapter::convert( row.login + "(" + std::to_string(row.id) + ")" );
}

- (NSUInteger) count {
    return static_cast<NSUInteger>( __result->count() );
}

@end
