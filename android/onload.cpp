#include "prefix.h"
#include <vector>
#include <jni.h>

extern "C" void nativeSayHi(JNIEnv * e) {
    // do something
}

extern "C" MX3EXPORT jint JNI_OnLoad(JavaVM * vm, void *) {
    JNIEnv * env = nullptr;
    auto result = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (result != JNI_OK) {
        return JNI_ERR;
    }
    vm->AttachCurrentThread(&env, 0);

    // todo(kabbes) load libraries in a nicer way
    jclass clazz = env->FindClass("com/mx3/Api");

    std::vector<JNINativeMethod> methods;
    methods.push_back(JNINativeMethod {"nativeSayHi", "()V", (void*)(&nativeSayHi)});
    env->RegisterNatives(clazz, methods.data(), methods.size());

    return JNI_VERSION_1_6;
}
