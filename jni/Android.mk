LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        zxlogger.cpp \
        configdata.cpp \
		kmsg.cpp \
		logcat.cpp \
		modem.cpp \
		net.cpp \
		dynamic.cpp \
		logcatcore.cpp \
		bugreport.cpp \
		qsee.cpp \
		tz.cpp \


LOCAL_MODULE:= zxlogger
LOCAL_MODULE_TAGS := optional 
LOCAL_INIT_RC := zxlogger.rc
LOCAL_PROPRIETARY_MODULE := true

LOCAL_STATIC_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES := \
			liblog\
			libutils

include $(BUILD_EXECUTABLE) 

include $(CLEAR_VARS)
LOCAL_MODULE := modemmask
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := modemmask.cfg
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_SUFFIX := .cfg
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := dynamic
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := dynamic.cfg
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_SUFFIX := .cfg
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := logcp
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := logcp.sh
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/vendor/bin
LOCAL_MODULE_SUFFIX := .sh
LOCAL_POST_INSTALL_CMD := \
        chmod 6755 $(LOCAL_MODULE_PATH)/$(LOCAL_SRC_FILES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := modem
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := modem.sh
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/vendor/bin
LOCAL_MODULE_SUFFIX := .sh
LOCAL_POST_INSTALL_CMD := \
        chmod 6755 $(LOCAL_MODULE_PATH)/$(LOCAL_SRC_FILES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := mytcpdump
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := mytcpdump
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/vendor/bin
LOCAL_POST_INSTALL_CMD := \
        chmod 6755 $(LOCAL_MODULE_PATH)/$(LOCAL_SRC_FILES)
include $(BUILD_PREBUILT)

