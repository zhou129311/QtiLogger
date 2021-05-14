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
#include <sys/wait.h>
#include <assert.h>
#include <stdarg.h>
#include <signal.h>
#include <arpa/inet.h>

#include <cutils/sockets.h>
#include <log/log.h>
#include <log/log_read.h>
#include <log/log.h>
#include <log/logprint.h>
#include <log/event_tag_map.h>
#include <utils/List.h>

#include "zxlogger.h"
#include "logcatcore.h"

using namespace android;

/*init class static Variables*/
const int LogcatLogger::maxDevices = 5;

String8 LogcatLogger::deviceName[maxDevices] =
{
    String8("main"),
    String8("radio"),
    String8("events"),
    String8("system"),
    String8("crash"),
};


bool LogcatLogger::isAvailDeviceName( const String8& device )
{
    int i = 0;
    int k = maxDevices;
    
    for (i = 0; i<k; i++)
    {
        if (deviceName[i] == device)
        {
            return true;
        }
    }

    return false;

}

bool LogcatLogger::isExistDevice( const String8& device )
{
    List<LogcatDevice>::iterator i;

    for (i = deviceList.begin(); i != deviceList.end(); i++)
    {
        if (i->getName() == device )
        {
            printf( "found an exist device in list:%s\n", i->getName().string() );

            return true;
        }
    }

    return false;

}

LogcatLogger::LogcatLogger( const String8& device1, const String8& device2, const String8& device3, 
                   const String8& device4, const String8& device5 )
{
    printf( "LogcatLogger Constructors!\n" );

    /*init Variables */
    plogFormat = NULL;
    outFD = -1;
    outByteCount = 0;
    mode = String8("normal");//default normal mode
    maxFiles = 0;//default maxfiles,be used in special mode
    logOutPath = String8("");
    logFileName = String8("");
    maxLogSize = 0;
    eventTagMap = NULL;

    /*add devices*/
    addLogcatDevice( device1 );
    addLogcatDevice( device2 );
    addLogcatDevice( device3 );
    addLogcatDevice( device4 );
    addLogcatDevice( device5 );
    
}

int LogcatLogger::addLogcatDevice( const String8& device )
{
    log_id_t logId;

    /*ensure the device is avail*/
    if (isAvailDeviceName( device ) && !isExistDevice( device ))
    {
        /*ensure the device is supported in liblog*/
        if ( (logId = android_name_to_log_id( device.string() )) != -1)
        {
            deviceList.push_back( LogcatDevice( device.string(), logId == LOG_ID_EVENTS ) );
        }

        return 0;
    }

    return -1;

}

int LogcatLogger::deleteLogcatDevice( const String8& device )
{

    List<LogcatDevice>::iterator i;

    if (isExistDevice( device ))
    {
        for (i = deviceList.begin(); i != deviceList.end(); i++)
        {
            if (i->getName() == device )
            {
                printf( "delete a device from the list!:%s\n", i->getName().string() );
                //deviceList.erase(i);//????this system func have a bug
                return 0;
            }

        }
    }
    return -1;
}

int LogcatLogger::showLogcatDevices( void )
{
    List<LogcatDevice>::iterator i;

    for (i = deviceList.begin(); i != deviceList.end(); i++)
    {
        printf( "logcat device:%s\n", i->getName().string() );
        ALOGE( "logcat device:%s\n", i->getName().string() );
    }

    return 0;
}

int LogcatLogger::setMode( const String8& _mode )
{
    mode = String8(_mode.string());    
    return 0;
}

int LogcatLogger::setMaxFiles( int max )
{
    maxFiles = max;
    return 0;
}

int LogcatLogger::setLogFormatAndLogSize( const String8& format, unsigned int maxSize )
{
    int ret = 0;
    int FilterFlag = 0;
    const char *env_tags_orig = "";
    AndroidLogPrintFormat format_tmp;

    /*set max log size*/
    maxLogSize = maxSize;

    format_tmp = android_log_formatFromString( format.string() );

    if (format_tmp == FORMAT_OFF)
    {
        // FORMAT_OFF means invalid string
        printf( "FORMAT_OFF means invalid log format!\n" );
        ALOGE( "FORMAT_OFF means invalid log format!\n" );

        return -1;
    }

    if (plogFormat == NULL)
    {
        
        /*init the plogFormat in the frist time*/
        plogFormat = android_log_format_new();

        if (plogFormat == NULL)
        {
            printf( "android_log_format_new err!\n" );
            ALOGE( "android_log_format_new err!\n" );

            return -1;
        }

        FilterFlag = 1;
    }

    if (plogFormat)
    {
        /*set format*/
        android_log_setPrintFormat( plogFormat, format_tmp );

        /*init the filter in the frist time*/
        if (FilterFlag && env_tags_orig != NULL)
        {
            ret = android_log_addFilterString( plogFormat, env_tags_orig );

            if (ret < 0)
            {
                printf( "android_log_addFilterString err!\n" );
                ALOGE( "android_log_addFilterString err!\n" );

                android_log_format_free( plogFormat );
                plogFormat = NULL;

                return -1;
            }

        }
    }

    return 0;
}

int LogcatLogger::setLogPathAndFileName( const String8& path , const String8& name )
{
    /*set the logpath and logfilename*/
    logOutPath = String8(path.string());    
    logFileName = String8(name.string());

    return 0;
}

int LogcatLogger::createLogFile( void )
{

    char fileName[256];
    char fileNametmp[256];
    char timestamp[32];
    struct timeval tv;
    time_t cur_time;
    int wfd = -1;
    int i = 0;

    memset( timestamp, 0, sizeof(timestamp) );
    memset( fileName, 0, sizeof(fileName) );

    if (mode == "normal")
    {
        gettimeofday( &tv, NULL );
        cur_time = tv.tv_sec;

        strftime( timestamp, sizeof( timestamp ), "%Y-%m-%d_%H_%M_%S", localtime( &cur_time ) );
        sprintf( fileName, "%s/%s.%s", logOutPath.string(), logFileName.string(), timestamp );
    }
    else if (mode == "boot_hung_monitor")
    {
        sprintf( fileName,"%s/%s", logOutPath.string(), logFileName.string());
    }
    else
    {
        i = maxFiles;
        while(i>1)
        {
            memset( fileName, 0, sizeof(fileName) );
            memset( fileNametmp, 0, sizeof(fileNametmp) );
            sprintf( fileName, "%s/%s%d", logOutPath.string(), logFileName.string() ,i-1);
            sprintf( fileNametmp, "%s/%s%d", logOutPath.string(), logFileName.string() ,i);
            ALOGD( "%s rename %s/%s%d!\n", fileName , logOutPath.string(), logFileName.string() ,i-1);
            ALOGD( "%s rename %s/%s%d!\n", fileNametmp , logOutPath.string(), logFileName.string() ,i-1);
            if ((rename(fileName, fileNametmp)) == -1)
            {
                printf( "%s rename error!\n", fileName );
                ALOGE( "%s logcatcore rename error!\n", fileName );
            }
            i--;
        }
        memset( fileName, 0, sizeof(fileName) );
        sprintf( fileName, "%s/%s1", logOutPath.string(), logFileName.string() );
    }

    /*create log file*/
    wfd = open( fileName, O_WRONLY | O_CREAT, 0 );

    if (wfd >= 0)
    {
        if (outFD >= 0)
        {
            close( outFD );

            outFD = -1;
        }

        outFD = wfd;

        return 0;
    }

    printf( "create log file err (%d) :%s,max:%d\n",wfd, fileName, maxLogSize );
    ALOGE( "create log file err (%d) :%s,max:%d\n",wfd, fileName, maxLogSize );

    return -1;
}

int LogcatLogger::startCaptureLog( void )
{
    List<LogcatDevice>::iterator i;
    struct logger *plogger = NULL;
    struct logger_list *ploggerList = NULL;
    unsigned int tail_lines = 0;
    struct log_msg log_msg;
    bool needBinary = false;
    int ret = 0;
    int errcount = 0;
    bool redo = false;

    if (plogFormat == NULL)
    {
        printf( "plogFormat = NULL,exit from startCaptureLog\n" );
        ALOGE( "plogFormat = NULL,exit from startCaptureLog\n" );

        return -1;
    }

    /*create output log file*/
    if (createLogFile() == -1)
    {
        printf( "createLogFile err!\n" );
        ALOGE( "createLogFile err!\n" );

        return -1;
    }

repeat:

    ploggerList = android_logger_list_alloc( O_RDONLY, tail_lines, 0 );

    if( ploggerList == NULL )
    {
        printf( "android_logger_list_alloc err!\n" );
        ALOGE( "android_logger_list_alloc err!\n" );

        return -1;
    }

    /*open logcat device*/
    for (i = deviceList.begin(); i != deviceList.end(); i++)
    {
        i->setLoggerList( ploggerList );

        plogger = android_logger_open( ploggerList,
                                       android_name_to_log_id( i->getName().string() ) );
        if (plogger == NULL)
        {
            printf( "android_logger_open err:%s!\n", i->getName().string() );
            ALOGE( "android_logger_open err:%s!\n", i->getName().string() );

            android_logger_list_free( ploggerList );

            return -1;
        }

        if (i->getbinary())
        {
            needBinary = true;
        }

        i->setLogger( plogger );
    }

    /*clear byte count*/
    outByteCount = 0;

    /*open event tag map for events*/
    if (needBinary && eventTagMap == NULL)
    {
        //printf( "open event tag map\n" );
        ALOGE( "open event tag map\n" );
        eventTagMap = android_openEventTagMap( EVENT_TAG_MAP_FILE );
    }

    errcount = 0;
    redo = false;

    for ( ;; )
    {

        if (errcount > 10)
        {
            redo = true;
            break;
        }
        /*read the log messages*/
        ret = android_logger_list_read( ploggerList, &log_msg );

        if (ret == 0)
        {
            printf( "read: Unexpected EOF!\n" );
            ALOGE( "read: Unexpected EOF!\n" );

            errcount++;
            continue;
        }

        if (ret < 0)
        {
            if (ret == -EAGAIN)
            {
                printf( "read: Unexpected EAGAIN!\n" );
                ALOGE( "read: Unexpected EAGAIN!\n" );
            }

            if (ret == -EIO)
            {
                printf( "read: Unexpected EOF!\n" );
                ALOGE( "read: Unexpected EOF!\n" );
            }

            if (ret == -EINVAL) 
            {
                printf( "read: unexpected length\n" );
                ALOGE( "read: unexpected length\n" );
            }

            printf( "android_logger_list_read err\n" );
            ALOGE( "android_logger_list_read err\n" );

            errcount++;
            continue;
        }/*end of if (ret < 0)*/

        errcount = 0;

        for (i = deviceList.begin(); i != deviceList.end(); i++)
        {
            if (android_name_to_log_id( i->getName().string() ) == log_msg.id()) 
            {
                /*process and save the logs*/
                if (!i->getPrinted())
                {
                    i->setPrinted( true );
                    printStart( i->getName() );
                }

                processBuffer( &log_msg, i->getbinary() );

                break;
            }
        }
    }/*end of for(;;)*/

    android_logger_list_free( ploggerList );

    if (redo)
    {
        sleep(1);
        printf("logcat read err (%d), redo!\n",errcount);
        ALOGE("logcat read err (%d), redo!\n",errcount);
        goto repeat;
    }

    if (outFD >= 0)
    {
        close( outFD );
        outFD = -1;
    }

    return 0;

}

void LogcatLogger::printStart( const String8& device )
{
    char buf[1024];

    if (outFD < 0)
    {
        printf( "printStart err,outFD = -1\n" );
        ALOGE( "printStart err,outFD = -1\n" );

        return;
    }

    snprintf( buf, sizeof(buf), "--------- beginning of %s\n", device.string() );

    if (write( outFD, buf, strlen( buf ) ) < 0)
    {
        printf( "printStart output error\n" );
        ALOGE( "printStart output error\n" );
    }
}

int LogcatLogger::processBuffer( struct log_msg *buf, bool binary )
{
    int bytesWritten = 0;
    int ret = 0;
    AndroidLogEntry entry;
    char binaryMsgBuf[1024];

    if (!buf)
    {
        printf( "processBuffer err:buf = NULL\n" );

        return -1;
    }

    if (outFD < 0)
    {
        printf( "processBuffer err,outFD = -1\n" );

        return -1;
    }

    if (binary)
    {
        ret = android_log_processBinaryLogBuffer( &buf->entry_v1, &entry, eventTagMap,
                                                 binaryMsgBuf, sizeof( binaryMsgBuf ) );
    }
    else
    {
        ret = android_log_processLogBuffer( &buf->entry_v1, &entry );
    }

    if (ret < 0)
    {
        printf( "android_log_processLogBuffer err!\n" );
        ALOGE( "android_log_processLogBuffer err!\n" );

        return -1;
    }

    if (android_log_shouldPrintLine( plogFormat, entry.tag, entry.priority) )
    {

        bytesWritten = android_log_printLogLine( plogFormat, outFD, &entry );

        if (bytesWritten < 0)
        {
            perror(" processBuffer output error\n" );
            ALOGE(" processBuffer output error\n" );

            return -1;
        }
    }

    outByteCount += bytesWritten;

    /*create a new log file for save*/
    if (maxLogSize && outByteCount >= maxLogSize)
    {
        if (!createLogFile())
        {
            outByteCount = 0;
        }
    }

    return 0;
}

