#include "MX3Snapshot.h"

#include <mx3/mx3.hpp>
#include "objc_adapter.h"
using mx3::ObjcAdapter;

@implementation MX3Snapshot {
  std::unique_ptr<mx3::SqlSnapshot> __snapshot;
}

- (instancetype) initWithSnapshot:(std::unique_ptr<mx3::SqlSnapshot>)snapshot {
  if(!(self = [super init])) {
    return nil;
  }
  // when you assign one unique_ptr to another, you have to move it because there can only ever
  // be one copy of a unique pointer at a time.  If you need to be able to "copy" pointers, use shared_ptr
  __snapshot = std::move(snapshot);
  return self;
}

- (void) dealloc {
    __snapshot = nullptr;
}

- (NSString *) rowAtIndex:(NSUInteger)index {
    auto row = __snapshot->at(static_cast<size_t>(index));
    return ObjcAdapter::convert(row);
}

- (NSUInteger) count {
    return static_cast<NSUInteger>( __snapshot->size() );
}

@end
