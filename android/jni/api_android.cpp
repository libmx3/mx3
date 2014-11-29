#include "stl.hpp"
#include "../../src/interface/api.hpp"
#include "../../src/interface/http.hpp"
#include "../../src/api.hpp"

shared_ptr<mx3_gen::Api>
mx3_gen::Api::create_api(const string & root_path, const shared_ptr<mx3_gen::Http> & http_impl) {
    return make_shared<mx3::Api>(
        root_path,
        nullptr,
        http_impl
    );
}

