LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifneq (,$(findstring ums9620, $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DANALOG_UMP9622
else ifneq (,$(findstring ums9520, $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DANALOG_UMP9622
else
LOCAL_CFLAGS += -DANALOG_SC2730
endif

LOCAL_32_BIT_ONLY := true

LOCAL_SRC_FILES := dcx_cali.c

LOCAL_MODULE := libdcxcali
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := npidevice
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES:= \
    $(TOP)/vendor/sprd/proprietories-source/engpc/sprd_fts_inc

LOCAL_SHARED_LIBRARIES:= liblog libc libcutils

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

ifneq (,$(findstring ums9620, $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DANALOG_UMP9622
else ifneq (,$(findstring ums9520, $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DANALOG_UMP9622
else
LOCAL_CFLAGS += -DANALOG_SC2730
endif

LOCAL_SRC_FILES:= dcxsrv.c

LOCAL_SHARED_LIBRARIES := libutils

LOCAL_MODULE := dcxsrv
LOCAL_INIT_RC := dcxsrv.rc
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES:= \
    $(TOP)/vendor/sprd/proprietories-source/engpc/sprd_fts_inc

LOCAL_SHARED_LIBRARIES:= liblog 

include $(BUILD_EXECUTABLE)
