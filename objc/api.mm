#include "../src/api.hpp"
#include "objc_event_loop.hpp"
#include "objc_http.hpp"
#include "stl.hpp"
#include <Foundation/Foundation.h>

std::shared_ptr<mx3_gen::Api>
mx3_gen::Api::create_api(const string& root_path) {
    return make_shared<mx3::Api>(
        root_path,
        make_shared<mx3::objc::ObjcEventLoop>(dispatch_get_main_queue()),
        make_shared<mx3::objc::ObjcHttp>()
    );
}
