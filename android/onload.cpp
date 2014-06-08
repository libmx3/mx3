#include "prefix.h"
#include <jni.h>

extern "C" MX3EXPORT jint JNI_OnLoad(JavaVM * vm, void *) {
    JNIEnv * env = nullptr;
    auto result = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (result != JNI_OK) {
        return JNI_ERR;
    }
    // register natives here
    return JNI_VERSION_1_6;
}
