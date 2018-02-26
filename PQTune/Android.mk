LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_RELATIVE_PATH := npidevice
LOCAL_PROPRIETARY_MODULE := ture

LOCAL_C_INCLUDES += external/libxml2/include

LOCAL_SRC_FILES := PQTuneDiag.c \
		ParseTuneReg.c \
		ParseGamma.c \
		ParseCms.c \
		ParseBld.c \
		ParseAbc.c \
		Ambient.cpp \

LOCAL_MODULE := libPQTune
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_SHARED_LIBRARIES += libcutils   \
                          libutils \
                          libdl \
			  libxml2 \
			  liblog \
			  libandroid \

include $(BUILD_SHARED_LIBRARY)


