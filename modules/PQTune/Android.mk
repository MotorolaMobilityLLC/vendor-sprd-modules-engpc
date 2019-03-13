LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := PQTune.$(TARGET_BOARD_PLATFORM)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_RELATIVE_PATH := npidevice
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += external/libxml2/include \
                    $(TOP)/vendor/sprd/proprietories-source/engmode

LOCAL_SRC_FILES := pq_diag.c \
				   pq_util.c \
				   pq_xml.c

LOCAL_SRC_FILES += pq_minui/graphics.c \
				   pq_minui/graphics_drm.c

LOCAL_SRC_FILES += pq_r2p0/tune_r2p0.c \
				   pq_r2p0/tune_abc.c \
				   pq_r2p0/tune_bld.c \
				   pq_r2p0/tune_cms.c \
				   pq_r2p0/tune_gamma.c \
				   pq_r2p0/tune_rwreg.c


LOCAL_SRC_FILES += pq_lite_r2p0/tune_lite_r2p0.c \
				   pq_lite_r2p0/tune_abc.c \
				   pq_lite_r2p0/tune_bld.c \
				   pq_lite_r2p0/tune_cms.c \
				   pq_lite_r2p0/tune_gamma.c \
				   pq_lite_r2p0/tune_rwreg.c

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),RGBX_8888)
LOCAL_CFLAGS += -DRECOVERY_RGBX
endif

ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),BGRA_8888)
LOCAL_CFLAGS += -DRECOVERY_BGRA
endif

$(warning "TARGET_ARCH= $(TARGET_ARCH)")
ifeq ($(strip $(TARGET_ARCH)),arm64)
	LOCAL_CFLAGS += -DTARGET_ARCH_ARM64
else
ifeq ($(strip $(TARGET_ARCH)),x86_64)
	LOCAL_CFLAGS += -DTARGET_ARCH_x86_64
else
	LOCAL_CFLAGS += -DTARGET_ARCH_ARM
endif
endif
LOCAL_SHARED_LIBRARIES += libcutils   \
						  libutils \
						  libdl \
						  libxml2 \
						  liblog \
						  libdrm

include $(BUILD_SHARED_LIBRARY)

