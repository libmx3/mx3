#pragma once
#include <mx3/mx3.hpp>

namespace mx3 { namespace objc {

class ObjcHttp : public mx3::Http {
  public:
    virtual ~ObjcHttp() {}
    virtual void get(const string& url, function<void(HttpResponse)>) override;
};

} } // end namespace mx3::objc
