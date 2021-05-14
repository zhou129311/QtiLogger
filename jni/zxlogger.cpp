//
// Created by zhouxian on 21-4-16.
//

#define LOG_TAG "zxlogger"

#include <cutils/properties.h>

////#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <utils/threads.h>

#if defined(HAVE_PTHREADS)
# include <pthread.h>
# include <sys/resource.h>
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "zxlogger.h"
#include "configdata.h"


using namespace android;


/*
 * Root path for saving logs
 * For example:
 * /storage/sdcard0/zxlog/
 */
int LogDeviceBase::setLogPath(const String8 &path) {
    logPath = String8(path.string());
    return 0;
}

int LogDeviceBase::setMode(const String8 &_mode) {
    mode = _mode.string();
    return 0;
}

int LogDeviceBase::setMaxFiles(int max) {
    maxFiles = max;
    return 0;
}


/*
 * Log mask file
 * For example:
 * /vendor/etc/dynamic.cfg
 * /vendor/etc/modemmask.cfg
 */
int LogDeviceBase::setLogMaskPath(const String8 &maskPath) {
    logMaskPath = String8(maskPath.string());
    return 0;
}

/*
 *if the log's is size bigger than Maxsize,we would create a new file for saving
 */
int LogDeviceBase::setLogMaxSize(unsigned int size) {
    logMaxSize = size;
    return 0;
}

/*
 * sub-directory
 * such as:
 * dmesg,main,modem,net
 */
int LogDeviceBase::setLogDirectory(const String8 &dir) {
    logDir = String8(dir.string());
    return 0;
}

/*
 * logfile name
 * such as:
 * dmesg.log.2015-08-21_14_31_22
 * event.log.2015-08-21_14_31_22,eg..
 */
int LogDeviceBase::setLogFileName(const String8 &fileName) {
    logFileName = String8(fileName.string());
    return 0;
}

String8 LogDeviceBase::getName(void) {
    return name;
}

int LogDeviceBase::createDirectory(const String8 &path, const String8 &dir) {
    String8 fullPath;
    DIR *diretory = NULL;
    int ret = -1;

    fullPath = path + "/" + dir;

    /*create log path*/
    diretory = opendir(fullPath.string());

    if (diretory == NULL) {

        ret = mkdir(fullPath.string(), 0777);

        if (ret < 0) {
            ALOGE("create %s fail\n", fullPath.string());
        } else {
            ALOGD("create %s success\n", fullPath.string());
        }
    } else {
        closedir(diretory);
        //ALOGD("%s exist\n", fullPath.string());
    }

    return ret;
}

int LogDeviceBase::split_string(char *split_str[], int split_max_num, char *str, char c) {
    char *pstr = str;
    int i = 0;

    if (pstr == NULL || split_str == NULL) {
        goto error;
    }

    for (i = 0; i < split_max_num; i++) {
        if (*pstr) {
            split_str[i] = pstr;
        } else {
            break;
        }

        while (*pstr && *pstr != c) pstr++;
        if (*pstr == c) {
            *pstr = 0;
            pstr++;
        }
    }

    return i;

    error:
    return -1;
}

int LogDeviceBase::generateTimestamp(String8 &timeStamp) {

    struct timeval tv;
    time_t cur_time;
    char buf[32];

    memset(buf, 0, sizeof(buf));

    gettimeofday(&tv, NULL);
    cur_time = tv.tv_sec;

    strftime(buf, sizeof(buf), "%Y-%m-%d_%H%M%S", localtime(&cur_time));

    timeStamp = buf;

    return 0;
}

int LogDeviceBase::readFile(const String8 &file) {
    const int bufSize = 4096;
    String8 srcFile = String8(file.string());
    String8 fileName("");
    String8 fileNametmp("");
    String8 num("");
    String8 timeStamp("");
    char *buf = NULL;
    char fb_buf[bufSize];
    size_t bsize = 0;
    ssize_t nr = 0;
    ssize_t nw = 0;
    ssize_t off = 0;
    ssize_t total = 0;
    int rfd = -1;
    int wfd = -1;
    int i = 0;

    generateTimestamp(timeStamp);

    if (mode == "normal") {
        fileName = logPath + "/" + logDir + "/" + logFileName + "." + timeStamp;
    } else {
        i = maxFiles;
        while (i > 1) {
            fileName = logPath + "/" + logDir + "/" + logFileName + num.format("%d", i - 1);
            fileNametmp = logPath + "/" + logDir + "/" + logFileName + num.format("%d", i);
            if ((rename(fileName.string(), fileNametmp.string())) == -1) {
                ALOGE("%s logger rename error!\n", fileName.string());
            } else {
                ALOGD("%s rename %s  \n", fileName.string(), fileNametmp.string());
            }
            i--;
        }
        fileName = logPath + "/" + logDir + "/" + logFileName + "1";
    }
    ALOGD("logFileName is %s  \n", fileName.string());

    rfd = open(srcFile.string(), O_RDONLY, 0);
    wfd = open(fileName.string(), O_WRONLY | O_CREAT, 0);

    if (rfd >= 0 && wfd >= 0) {
        buf = fb_buf;
        bsize = bufSize;
        total = 0;

        for (;;) {
            if ((nr = read(rfd, buf, bsize)) > 0) {
                for (off = 0; nr; nr -= nw, off += nw) {
                    if ((nw = write(wfd, buf + off, (size_t) nr)) < 0) {
                        ALOGE("%s write error!\n", fileName.string());
                    } else {
                        total += nw;
                    }
                }

                if (logMaxSize && total >= logMaxSize) {

                    generateTimestamp(timeStamp);

                    fileName = logPath + "/" + logDir + "/" + logFileName + "." + timeStamp;

                    close(wfd);
                    wfd = 0;
                    wfd = open(fileName.string(), O_WRONLY | O_CREAT, 0);

                    total = 0;

                    ALOGE("split a new %s file :%s\n", getName().string(), fileName.string());
                    if (wfd < 0) {
                        ALOGE("split %s file error\n", getName().string());
                        break;
                    }

                }
            } else {
                ALOGE("%s read error!\n", srcFile.string());
            }

        }/*end of for (;;)*/


    }/*end of if (rfd >=0 && wfd >= 0)*/

    if (rfd < 0) {
        ALOGE("open %s fail!\n", srcFile.string());
    } else {
        close(rfd);
        rfd = -1;
    }

    if (wfd < 0) {
        ALOGE("open %s fail!\n", fileName.string());
    } else {
        close(wfd);
        wfd = -1;
    }

    return 0;
}

status_t LogDeviceBase::readyToRun() {
    createDirectory(logPath);
    createDirectory(logPath, logDir);
    return NO_ERROR;
}

LogDeviceBase::_LogDeviceBase(const String8 &devName) : Thread(false) {
    /*init Variables*/
    //ALOGD("%s Device Constructors!\n", devName.string());

    name = String8(devName.string());
    logPath = String8("");
    logMaskPath = String8("");
    mode = String8("normal");//default normal mode
    maxFiles = 0;//default maxfiles,be used in special mode

    /*normally ,the log's filename is the devName + .log
     *or you can modify it by setLogFileName() function
     */
    logFileName = devName + ".log";

    /*normally,the log's diretory is the devName 
     *or yo can modify it by setLogDirectory() function
     */
    logDir = devName.string();
    logMaxSize = 0;
    threadId = 0;
}

LogDeviceBase::~_LogDeviceBase() {
    ALOGE("%s Device Destructors!\n", name.string());
}

int main(int argc, char **argv) {

    char propbuf[100];
    ConfigData logConfig;
    String8 logPath;
    String8 logMaskPath;
    String8 mode = String8("normal");
    int maxFiles = 0;
    unsigned int logMaxSize;

    logConfig.read(argc, argv);
    logConfig.getLogPath(logPath);
    ALOGD("logger getLogPath %s\n", logPath.c_str());
    logConfig.getMode(mode);
    maxFiles = logConfig.getMaxFiles();
    ALOGD("logger getMode %s\n", mode.c_str());
    if (mode == "boot") {
        property_get("persist.sys.bootlogger", propbuf, "empty");
        if (!strncmp(propbuf, "empty", 5)) {
            ALOGD("logger android first boot\n");
            //property_set("persist.sys.bootlogger","false");
        }

        if ((!strncmp(propbuf, "true", 4)) || (!strncmp(propbuf, "empty", 5))) {
            /*capture boot logger*/
            ALOGD("logger capture boot logger\n");
        } else {
            ALOGD("logger end boot logger\n");
            return 0;
        }

    }

    //DynamicDevice dynamic;
    KmsgLogDevice kmsglog;

    if (logConfig.isLogEnable(kmsglog.getName())) {
        /*dynamic device*/

        //get dynamic logmask path
        //logConfig.getDynamicLogMaskPath(logMaskPath);
        //const String8 path("/data/zxlog");
        //dynamic.setMode(mode);
        //dynamic.setMaxFiles(maxFiles);
        //dynamic.setLogPath(path);
        //dynamic.setLogMaskPath(logMaskPath);
        //dynamic.run(dynamic.getName().string());
        //ALOGD("logger dynamic run\n");
        /*kmsg log device*/

        //get kmsg log max size
        logMaxSize = logConfig.getLogMaxSize(String8(kmsglog.getName()));

        kmsglog.setMode(mode);
        kmsglog.setMaxFiles(maxFiles);
        kmsglog.setLogPath(logPath);
        kmsglog.setLogMaxSize(logMaxSize);
        kmsglog.run(kmsglog.getName().string());
        ALOGD("logger kmsglog run\n");
    }

    QseeLogDevice qseelog;

    if (logConfig.isLogEnable(qseelog.getName())) {
        /*qsee log device*/
        logMaxSize = logConfig.getLogMaxSize(String8(qseelog.getName()));
        const String8 path("/data/zxlog");
        qseelog.setMode(mode);
        qseelog.setMaxFiles(maxFiles);
        qseelog.setLogPath(path);
        qseelog.setLogMaxSize(logMaxSize);
        qseelog.run(qseelog.getName().string());
        ALOGD("logger qseelog run\n");
    }

    TzLogDevice tzlog;

    if (logConfig.isLogEnable(tzlog.getName())) {
        /*tz log device*/
        logMaxSize = logConfig.getLogMaxSize(String8(tzlog.getName()));
        const String8 path("/data/zxlog");
        tzlog.setMode(mode);
        tzlog.setMaxFiles(maxFiles);
        tzlog.setLogPath(path);
        tzlog.setLogMaxSize(logMaxSize);
        tzlog.run(tzlog.getName().string());
        ALOGD("logger tzlog run\n");
    }

    ModemLogDevice Modemlog;
    if (logConfig.isLogEnable(Modemlog.getName()) && strncmp(propbuf, "empty", 5)) {
        /*modem log device*/

        //get modem logmask path
        logConfig.getModemLogMaskPath(logMaskPath);

        //get modem log max size
        logMaxSize = logConfig.getLogMaxSize(String8(Modemlog.getName()));
        Modemlog.setMode(mode);
        Modemlog.setMaxFiles(maxFiles);
        Modemlog.setLogPath(logPath);
        Modemlog.setLogMaskPath(logMaskPath);
        Modemlog.setLogMaxSize(logMaxSize);
        Modemlog.run(Modemlog.getName().string());
        ALOGD("logger Modemlog run\n");
    }

    BugReportDevice bugreportlog;
    if (logConfig.isLogEnable(bugreportlog.getName())) {
        /*bugreport log device*/

        logMaxSize = logConfig.getLogMaxSize(String8(bugreportlog.getName()));
        bugreportlog.setMode(mode);
        bugreportlog.setMaxFiles(maxFiles);
        bugreportlog.setLogPath(logPath);
        bugreportlog.setLogMaxSize(logMaxSize);
        bugreportlog.run(bugreportlog.getName().string());
        ALOGD("logger bugreportlog run\n");
    }

    NetDevice Netlog;
    if (logConfig.isLogEnable(Netlog.getName())) {
        /*net log device*/

        //get net log max size
        logMaxSize = logConfig.getLogMaxSize(String8(Netlog.getName()));

        Netlog.setMode(mode);
        Netlog.setMaxFiles(maxFiles);
        Netlog.setLogPath(logPath);
        Netlog.setLogMaxSize(logMaxSize);
        Netlog.run(Netlog.getName().string());
        ALOGD("logger Netlog run\n");
    }

    MainLogDevice Mainlog;
    RadioLogDevice Radiolog;
    EventLogDevice Eventlog;
    SystemLogDevice Systemlog;
    CrashLogDevice Crashlog;
    if (logConfig.isLogEnable(Mainlog.getName())) {
        if (mode == "boot_hung_monitor") {
            /*boot_main*/
            Mainlog.setLogDirectory(String8("bhm"));
            Mainlog.setLogFileName(String8("main"));
            /*boot_event*/
            Eventlog.setLogDirectory(String8("bhm"));
            Eventlog.setLogFileName(String8("event"));
            /*boot_radio*/
            Radiolog.setLogDirectory(String8("bhm"));
            Radiolog.setLogFileName(String8("radio"));
            /*boot_system*/
            Systemlog.setLogDirectory(String8("bhm"));
            Systemlog.setLogFileName(String8("system"));
            /*boot_crash*/
            Crashlog.setLogDirectory(String8("bhm"));
            Crashlog.setLogFileName(String8("crash"));
        } else {
            Mainlog.setLogDirectory(String8("android"));
            Radiolog.setLogDirectory(String8("android"));
            Eventlog.setLogDirectory(String8("android"));
            Systemlog.setLogDirectory(String8("android"));
            Crashlog.setLogDirectory(String8("android"));

            Mainlog.setLogFileName(String8("main.log"));
            Radiolog.setLogFileName(String8("radio.log"));
            Eventlog.setLogFileName(String8("event.log"));
            Systemlog.setLogFileName(String8("system.log"));
            Crashlog.setLogFileName(String8("crash.log"));
        }

        /*main log device*/

        //get main log max size
        logMaxSize = logConfig.getLogMaxSize(String8(Mainlog.getName()));

        Mainlog.setMode(mode);
        Mainlog.setMaxFiles(maxFiles);
        Mainlog.setLogPath(logPath);
        Mainlog.setLogMaxSize(logMaxSize);

        /*radio log device*/
        Radiolog.setMode(mode);
        Radiolog.setMaxFiles(maxFiles);
        Radiolog.setLogPath(logPath);
        Radiolog.setLogMaxSize(logMaxSize);

        /*event log device*/
        Eventlog.setMode(mode);
        Eventlog.setMaxFiles(maxFiles);
        Eventlog.setLogPath(logPath);
        Eventlog.setLogMaxSize(logMaxSize);

        /*event log device*/
        Systemlog.setMode(mode);
        Systemlog.setMaxFiles(maxFiles);
        Systemlog.setLogPath(logPath);
        Systemlog.setLogMaxSize(logMaxSize);

        /*event log device*/
        Crashlog.setMode(mode);
        Crashlog.setMaxFiles(maxFiles);
        Crashlog.setLogPath(logPath);
        Crashlog.setLogMaxSize(logMaxSize);

        /*run*/
        Mainlog.run(Mainlog.getName().string());
        Radiolog.run(Radiolog.getName().string());
        Eventlog.run(Eventlog.getName().string());
        Systemlog.run(Systemlog.getName().string());
        Crashlog.run(Crashlog.getName().string());
        ALOGD("logger Mainlog Systemlog Radiolog Eventlog Crashlog run\n");
    }

    for (;;) {
        sleep(60);
    }

    ALOGD("logger exit!!!!\n");
    return 0;
}




