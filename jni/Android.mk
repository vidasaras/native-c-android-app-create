LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := hello
LOCAL_SRC_FILES := hello.c android_native_app_glue.c
LOCAL_LDLIBS    := -llog -landroid -lm
include $(BUILD_SHARED_LIBRARY)
