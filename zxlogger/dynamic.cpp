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

/* dynamicLogmask.cfg format
<level=6>
<func=monitor_soc_work>
<file=qpnp-bms.c>
<func=gt1x_touch_event_handler>
<func=apds9960_als_polling_work_handler>
<func=apds9960_gesture_processing>
<func=akm_dev_poll>
<func=goodix_ts_work_func>
*/

/*init static variables*/
const int DynamicDevice::maxKeyNum = 3;

DynamicDevice::keyProp DynamicDevice::keys[maxKeyNum] =
{
    { String8("level"), String8("/proc/sys/kernel/printk"),  String8(""),     String8("")   },
    { String8("func") , String8("/d/dynamic_debug/control"), String8("func"), String8("+p") },
    { String8("file") , String8("/d/dynamic_debug/control"), String8("file"), String8("+p") },
};

int DynamicDevice::getKeyPath( const String8& key, String8& out )
{
    int i = 0;
    int k = maxKeyNum;

    for (i = 0; i < k; i++)
    {
        if (key == keys[i].key)
        {
            out = keys[i].keyPath.string();
            return 0;
        }
    }

    out = "";
    return -1;
}

int DynamicDevice::getKeyValue( const String8& key, const String8& value, String8& out )
{
    String8 valueRet("");
    int i = 0;
    int k = maxKeyNum;

    for (i = 0; i < k; i++)
    {
        if (key == keys[i].key)
        {
            valueRet = keys[i].keyValuePrefix + " " + value + " "
                + keys[i].keyValueSubfix;
            out = valueRet.string();
            return 0;
        }
    }

    out = valueRet.string();
    return -1;
}

int DynamicDevice::writeFile( const String8& path, const String8& value )
{
    int fd = -1;
    int ret = -1;

    if (path.isEmpty() || value.isEmpty())
    {
        ALOGE( "writeFile err! path / value isEmpty!!\n" );
        return -1;
    }

    printf( "path:%s,value:%s\n", path.string(), value.string() );

    fd = open( path.string(), O_WRONLY, 0622 );

    if (fd < 0)
    {
        printf( "open %s fail!!\n", path.string() );
        ALOGE( "open %s fail!!\n", path.string() );
        return -errno;
    }


    do
    {
        ret = write( fd, value.string(), value.size() );

    } while (ret < 0 && errno == EINTR);

    close( fd );

    if (ret < 0)
    {
        printf( "write %s fail!!\n", value.string() );
        ALOGE( "write %s fail!!\n", value.string() );
        return -errno;
    }
    else
    {
        return 0;
    }
}


int DynamicDevice::parseMaskFile( const String8& maskFile )
{
    FILE *fp = NULL;
    char buf[256];
    char key[16];
    char path[128];
    char value[128];
    char *pstrbegin = NULL;
    char *pstrend = NULL;
    char *pstr = NULL;
    int len = 0;
    String8 pathWrite("");
    String8 valueWrite("");


    fp = fopen( maskFile.string(), "r" );
	if (fp == NULL)
    {
        printf( "%s,open fail!\n", maskFile.string() );
        ALOGE( "%s,open fail!\n", maskFile.string() );

        return -1;
	}

    while (!feof( fp ))
    {
        memset( buf, 0, sizeof(buf) );
        fgets( buf, sizeof(buf), fp );

	    printf( "%s\n", buf );

        pstrbegin = strstr( buf, "<" );
        pstrend = strstr( buf, ">" );

        if (pstrbegin && pstrend)
        {
            pstrbegin += 1;//skip "<"
            //found "="
            if (( pstr = strstr( pstrbegin, "=" ) ) != 0)
            {
                //get key
                memset( key, 0, sizeof( key ) );

                len = pstr - pstrbegin;
                if (len > 0 && (unsigned int)len < sizeof( key ))
                {
                    strncpy( key, pstrbegin, len );
                    printf( "key:%s  ", key );
                }

                //skip "="
                pstrbegin = pstr + 1;

                //get value
                memset( value, 0, sizeof( value ) );

                len = pstrend - pstrbegin;
                if (len > 0 && (unsigned int)len < sizeof( value ))
                {
                    strncpy( value, pstrbegin, len );
                    printf( "value:%s\n", value );
                }

                if (key[0] && value[0])
                {
                    if (!getKeyValue( String8(key), String8(value), valueWrite )
                         && !getKeyPath( String8(key), pathWrite ))
                    {
                        writeFile( pathWrite, valueWrite );
                    }
                }
            
                
            }/*end of if(( pstr = strstr( pstrbegin, "=") ) != 0)*/

        }/*end of if(pstrbegin && pstrend)*/
        
    }/*end of while()*/

    fclose(fp);

    return 0;
}

bool DynamicDevice::threadLoop()
{
    parseMaskFile( logMaskPath );

    for( ;; )
    {
        sleep(60);
    }

    printf( "%s threadLoop exit!\n", getName().string() );

    return 0;
}


