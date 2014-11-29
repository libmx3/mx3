FORCE_GYP := $(shell make -C ../../ GypAndroid.mk)
include ../../Application.mk
APP_BUILD_SCRIPT := jni/Android.mk
