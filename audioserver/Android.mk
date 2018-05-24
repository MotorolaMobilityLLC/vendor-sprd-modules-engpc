#Build libengbt
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := audio_server.cpp 


LOCAL_MODULE := libeng-audio
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_C_INCLUDES:=   \
             frameworks/av/include \
             frameworks/av/media/libaudioclient/include \
             frameworks/base/include 

LOCAL_VENDOR_MODULE := true

LOCAL_SHARED_LIBRARIES += libcutils   \
                          libutils    \
                          libhardware \
                          libaudioclient \
                          libmedia
include $(BUILD_SHARED_LIBRARY)


