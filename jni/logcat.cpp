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
#include "logcatcore.h"


using namespace android;


bool RadioLogDevice::threadLoop()
{
    String8 path("");

    printf( "%s threadLoop run!\n", getName().string() );
    ALOGD( "%s threadLoop run!\n", getName().string() );

    LogcatLogger radioLog( String8("radio") );

    path = logPath + "/" + logDir;

    radioLog.setMode( mode );
    radioLog.setMaxFiles( maxFiles );
    radioLog.setLogFormatAndLogSize( String8("threadtime"), logMaxSize );
    radioLog.setLogPathAndFileName( path , logFileName );
    ALOGD( "RadioLog path = %s , filename = %s\n", path.string(), logFileName.string());
    radioLog.startCaptureLog();

    for ( ;; )
    {
        sleep( 60 );
    }

    printf( "%s threadLoop exit!\n", getName().string() );

    return 0;

}

bool EventLogDevice::threadLoop()
{
    String8 path("");

    printf( "%s threadLoop run!\n", getName().string());
    ALOGD( "%s threadLoop run!\n", getName().string() );

    LogcatLogger eventLog( String8("events") );

    path = logPath + "/" + logDir;

    eventLog.setMode( mode );
    eventLog.setMaxFiles( maxFiles );
    eventLog.setLogFormatAndLogSize( String8("threadtime"), logMaxSize );
    eventLog.setLogPathAndFileName( path , logFileName );
    ALOGD( "EventLog path = %s , filename = %s\n", path.string(), logFileName.string());
    eventLog.startCaptureLog();

    for ( ;; )
    {
        sleep( 60 );
    }

    printf( "%s threadLoop exit!\n", getName().string() );

    return 0;

}

bool MainLogDevice::threadLoop()
{
    String8 path("");

    printf( "%s threadLoop run!\n", getName().string() );
    ALOGD( "%s threadLoop run!\n", getName().string() );

    LogcatLogger mainLog( String8("main"), String8("system"), String8("crash"));

    path = logPath + "/" + logDir;

    mainLog.setMode( mode );
    mainLog.setMaxFiles( maxFiles );
    mainLog.setLogFormatAndLogSize( String8("threadtime"), logMaxSize );
    mainLog.setLogPathAndFileName( path , logFileName );
    ALOGD( "MainLog path = %s , filename = %s\n", path.string(), logFileName.string());
    mainLog.startCaptureLog();

    for ( ;; )
    {
        sleep( 60 );
    }

    printf( "%s threadLoop exit!\n", getName().string() );

    return 0;

}

bool SystemLogDevice::threadLoop()
{
    String8 path("");

    printf( "%s threadLoop run!\n", getName().string() );
    ALOGD( "%s threadLoop run!\n", getName().string() );

    LogcatLogger mainLog(String8("system"));

    path = logPath + "/" + logDir;

    mainLog.setMode( mode );
    mainLog.setMaxFiles( maxFiles );
    mainLog.setLogFormatAndLogSize( String8("threadtime"), logMaxSize );
    mainLog.setLogPathAndFileName( path , logFileName );
    ALOGD( "SystemLog path = %s , filename = %s\n", path.string(), logFileName.string());
    mainLog.startCaptureLog();

    for ( ;; )
    {
        sleep( 60 );
    }

    printf( "%s threadLoop exit!\n", getName().string() );

    return 0;

}

bool CrashLogDevice::threadLoop()
{
    String8 path("");

    printf( "%s threadLoop run!\n", getName().string() );
    ALOGD( "%s threadLoop run!\n", getName().string() );

    LogcatLogger mainLog(String8("crash") );

    path = logPath + "/" + logDir;

    mainLog.setMode( mode );
    mainLog.setMaxFiles( maxFiles );
    mainLog.setLogFormatAndLogSize( String8("threadtime"), logMaxSize );
    mainLog.setLogPathAndFileName( path , logFileName );
    ALOGD( "CrashLog path = %s , filename = %s\n", path.string(), logFileName.string());
    mainLog.startCaptureLog();

    for ( ;; )
    {
        sleep( 60 );
    }

    printf( "%s threadLoop exit!\n", getName().string() );

    return 0;

}


