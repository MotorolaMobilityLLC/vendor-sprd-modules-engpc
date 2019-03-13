LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE    := false

CONFIG_MINIENGPC := false

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)

ifeq ($(strip $(BOARD_SECURE_BOOT_ENABLE)), true)
LOCAL_CFLAGS += -DSECURE_BOOT_ENABLE
endif

LOCAL_SHARED_LIBRARIES  := libcutils libsqlite libhardware libhardware_legacy libatci libutils
LOCAL_STATIC_LIBRARIES  := libbootloader_message libfs_mgr libbase

ifeq ($(strip $(BOARD_AP_SIMLOCK_EFUSE_CONFIG)), true)
LOCAL_CFLAGS += -DAP_SIMLOCK_EFUSE
endif

LOCAL_LDLIBS        += -Idl
ifeq ($(strip $(BOARD_USE_EMMC)),true)
LOCAL_CFLAGS += -DCONFIG_EMMC
endif

ifeq ($(strip $(TARGET_USERIMAGES_USE_UBIFS)),true)
LOCAL_CFLAGS := -DCONFIG_NAND
endif

ifeq ($(USE_BOOT_AT_DIAG),true)
LOCAL_CFLAGS += -DUSE_BOOT_AT_DIAG
endif

LOCAL_C_INCLUDES    += hardware/libhardware/include \
                       hardware/libhardware_legacy/include \
                       frameworks/opt/net/wifi/libwifi_hal/include \
                       system/core/include \
                       system/media/audio/include \
                       $(TOP)/vendor/sprd/proprietories-source/engmode/libtracedump \
                       bootable/recovery \
                       bootable/recovery/include \
                       $(TOP)/vendor/sprd/proprietories-source/engpc \

ifneq ($(CONFIG_MINIENGPC), true)
LOCAL_C_INCLUDES    +=  vendor/sprd/modules/audio/voiceband/effect/include \
                        vendor/sprd/modules/audio/voiceband/gaincontrol/include \
                        vendor/sprd/modules/audio/nv_exchange/include \
                        vendor/sprd/proprietories-source/trustzone/libefuse/ \
                        vendor/sprd/modules/libatci/ \
                        vendor/sprd/modules/gps/gnsspc/ \
                        vendor/sprd/proprietories-source/libbm/ \
                        hardware/marvell/bt/libbt-vendor/
endif
LOCAL_C_INCLUDES    +=  external/sqlite/dist/ \
                        $(TARGET_OUT_INTERMEDIATES)/KERNEL/source/include/uapi/mtd/


LOCAL_SRC_FILES     := eng_pcclient.c  \
                       eng_diag.c \
                       vlog.c \
                       vdiag.c \
                       eng_productdata.c \
                       adc_calibration.c\
                       crc16.c \
                       eng_attok.c \
                       engopt.c \
                       eng_at.c \
                       eng_sqlite.c \
                       eng_btwifiaddr.c \
                       eng_cmd4linuxhdlr.c \
                       power.c \
                       backlight.c \
                       eng_util.c \
                       eng_autotest.c \
                       eng_uevent.c \
                       eng_debug.c \
                       eng_ap_modem_time_sync.c \
                       emmc.cpp \
                       ddr.c \
                       eng_socket.c \
                       eng_modules.c \
                       channl_at.c \
                       channl_diag.c \
                       eng_resetft.cpp \

ifeq ($(CONFIG_ENGPC_BUSMONITOR), true)
LOCAL_SRC_FILES += eng_busmonitor.c
LOCAL_CFLAGS += -DCONFIG_ENGPC_BUSMONITOR
endif

ifeq ($(DCXDATA_SUPPORT),true)
LOCAL_SRC_FILES += dcxsrv/dcxsrv.c
LOCAL_CFLAGS += -DDCXDATA_SUPPORT
endif


#engpc adb cmd support
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/cmd_server \

LOCAL_SRC_FILES += $(call all-cpp-files-under, cmd_server)

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_MODULE := engpc
LOCAL_INIT_RC := engpc.rc
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
include $(BUILD_EXECUTABLE)
include $(call all-makefiles-under,$(LOCAL_PATH))
