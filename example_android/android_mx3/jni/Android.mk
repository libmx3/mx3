
LOCAL_PATH := $(call my-dir)
#MX3_PATH := $(realpath $(LOCAL_PATH)/../../..)

include $(CLEAR_VARS)

LOCAL_MODULE    := mx3_jni
LOCAL_SRC_FILES := mx3_jni.cpp

#include $(MX3_PATH)/GypAndroid.mk

include $(BUILD_SHARED_LIBRARY)