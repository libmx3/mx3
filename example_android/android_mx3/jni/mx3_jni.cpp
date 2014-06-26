#include <jni.h>
#include <ctime>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream, std::stringbuf

using namespace std;

//Trivial C++ file that fetches and returns a welcome message with the current date and time.

string getTimeString();

extern "C" {
	jstring Java_com_mx3_Api_nativeSayHi(JNIEnv * env, jobject thiz) {
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
