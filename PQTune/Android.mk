LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := PQTune.$(TARGET_BOARD_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_RELATIVE_PATH := npidevice
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES += external/libxml2/include

LOCAL_SRC_FILES := PQTuneDiag.c \
		ParseTuneReg.c \
		ParseGamma.c \
		ParseCms.c \
		ParseBld.c \
		ParseAbc.c \

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_SHARED_LIBRARIES += libcutils   \
                          libutils \
                          libdl \
			  libxml2 \
			  liblog \

include $(BUILD_SHARED_LIBRARY)



