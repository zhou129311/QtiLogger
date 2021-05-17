//
// Created by zhouxian on 21-4-16.
//

#define LOG_TAG "zxlogger"

#include <cutils/properties.h>

//#include <binder/IPCThreadState.h>
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


using namespace android;

bool ModemLogDevice::threadLoop()
{
//    const String8 modem("/system/bin/diag_mdlog ");
//    String8 cmd("");
//    String8 maxSize("");

    printf( "%s threadLoop run!\n", getName().string() );
    ALOGD( "%s threadLoop run!\n", getName().string() );

//    cmd = modem + " -f " + logMaskPath + " -s " + maxSize.format("%d",logMaxSize)
//          + " -o " + logPath + "/" + logDir + " -c";

    String8 fullPath;
    DIR *diretory = NULL;
    int ret = -1;

    fullPath = logPath + "/" + logDir;

    /*create log path*/
    diretory = opendir(fullPath.string());

    if (diretory == NULL) {

        ret = mkdir(fullPath.string(), 0777);

        if (ret < 0) {
            ALOGE("create %s fail, don't start modem log\n", fullPath.string());
            return 0;
        } else {
            ALOGD("create %s success\n", fullPath.string());
        }
    } else {
        closedir(diretory);
        //ALOGD("%s exist\n", fullPath.string());
    }

    //system( cmd.string() );
    property_set("debug.sys.logger.modem","1");

    for ( ;; )
    {
        sleep( 60 );
    }

    printf( "%s threadLoop exit!\n", getName().string() );
    ALOGD( "%s threadLoop exit!\n", getName().string() );

    return 0;

}


