#pragma once
#include "gen/MX3Http.h"

@interface MX3HttpObjc : NSObject <MX3Http>

- (void)get:(NSString *)url callback:(MX3HttpCallback *)callback;

@end
