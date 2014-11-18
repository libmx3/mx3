#pragma once

namespace mx3 {

class JsonStore {
  public:
    virtual ~JsonStore() {}
    virtual optional<json11::Json> get(const string& key) = 0;
    virtual void set(const string& key) = 0;
};

}
