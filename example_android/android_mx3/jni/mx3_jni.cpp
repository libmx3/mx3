#include <jni.h>
#include <ctime>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream, std::stringbuf

using namespace std;

string getTimeString();

extern "C" {
jstring Java_com_mx3_Api_nativeSayHi(JNIEnv * env, jobject thiz){
	return env->NewStringUTF(getTimeString().c_str());
}
}

string getTimeString(){
	time_t t = time(0);   // get time now
	stringstream ss;
	struct tm * now = localtime( & t );
	ss << "Welcome! The current time is " << (now->tm_year + 1900) << '-'
			<< (now->tm_mon + 1) << '-'
			<<  now->tm_mday << " at " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec
			<< endl;
	return ss.str();
}

/*extern "C" MX3EXPORT jint JNI_OnLoad(JavaVM * vm, void *) {
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
}*/
