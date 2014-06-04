#pragma once
#include <string>
#include <sstream>
#include <memory>

// some stuff that belongs in the c++ stllib, but isn't
namespace std_patch {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    // for some reason android doesn't ship with to_string
    // implement it here
    template<typename T>
    std::string to_string( const T& n ) {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}
