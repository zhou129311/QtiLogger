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
#include "configdata.h"


using namespace android;

/*------------------------DATA class----------------------*/

const unsigned int DATA::defaultLogMaxSize = 12*1024*1024;//12M


int DATA::init( void )
{

    mode = "normal";//default normal mode
    logPath = "/storage/emulated/0/zxlog/temp";
    modemLogMaskPath = "/vendor/etc/modemmask.cfg";
    dynamicLogMaskPath = "/vendor/etc/dynamic.cfg";
    
    addCommonData( String8("dmesg") );
    addCommonData( String8("qsee") );
    addCommonData( String8("tz") );
    addCommonData( String8("main") );
    addCommonData( String8("net") );
    addCommonData( String8("bugreport") );
    addCommonData( String8("modem") ,100 );

    return 0;
}


int DATA::addCommonData( const String8& _logName, unsigned int size )
{

    CommonDataList.push_back( CommonData( _logName, size ) );
    return 0;

}

int DATA::split(const String8& str, const String8& pattern)
{
    size_t pos = 0;  
    size_t size = 0;  
    String8 strtmp(str.string());

    strtmp += pattern;
    size = strtmp.size();

    for(size_t i=0; i<size; i++)  
    {  
        pos = strtmp.find(pattern, i);  
        if(pos < size)  
        {  
            PropConfigDataList.push_back(String8(strtmp.string() + i, pos - i));  
            i = pos + pattern.size() - 1;  
        }  
    }  
    return 0; 
}

int DATA::isVailLogMaskPath(const String8& str)
{
    String8 path;

    path.setPathName( str.string() );
    if (!path.getPathExtension().compare(String8(".cfg")) && path.string()[0] == '/')
    {
        return 0;
    }
    
    return -1;
}

int DATA::userSpecifyLogMask( void )
{
    List<String8>::iterator i;

    for (i = PropConfigDataList.begin(); i != PropConfigDataList.end(); i++)
    {
        if (i->contains("cfg"))
        {
            if (!isVailLogMaskPath( String8(i->string()) ))
            {
                configModemLogMaskPath(String8(i->string()));
                printf("user Specify cfg = %s\n",modemLogMaskPath.string());
                ALOGD("user Specify cfg = %s\n",modemLogMaskPath.string());
            }
        }
        printf("prop list:%s\n",i->string());
    }

    return 0;
}

int DATA::configMode( const String8& _mode )
{
    mode = _mode.string();
    return 0;
}

int DATA::configMaxFiles( int max )
{
    maxFiles = max;
    return 0;
}

int DATA::configLogPath( const String8& path )
{
    logPath = path.string();
    return 0;
}

int DATA::configModemLogMaskPath( const String8& path )
{
    modemLogMaskPath = path.string();
    return 0;
}

int DATA::configDynamicLogMaskPath( const String8& path )
{
    dynamicLogMaskPath = path.string();
    return 0;
}

int DATA::configLogEnable( const String8& name, bool enable )
{
    List<CommonData>::iterator i;

    for (i = CommonDataList.begin(); i != CommonDataList.end(); i++)
    {
        if (i->logName == name)
        {
            i->logEnable = enable;
        }
    }

    return 0;
}

int DATA::configLogMaxSize( const String8& name, unsigned int size )
{
    List<CommonData>::iterator i;

    for (i = CommonDataList.begin(); i != CommonDataList.end(); i++)
    {
        if (i->logName == name)
        {
            i->logMaxSize = size;
        }
    }
    return 0;
}

int DATA::getMode( String8& _mode )
{
    _mode = mode.string();
    return 0;
}

int DATA::getMaxFiles( void )
{
    return maxFiles;
}

int DATA::getLogPath( String8& path )
{
    path = logPath.string();
    return 0;
}

int DATA::getModemLogMaskPath( String8& path )
{
    path = modemLogMaskPath.string();
    return 0;
}

int DATA::getDynamicLogMaskPath( String8& path )
{
    path = dynamicLogMaskPath.string();
    return 0;
}

bool DATA::isLogEnable( const String8& name )
{
    List<CommonData>::iterator i;

    for (i = CommonDataList.begin(); i != CommonDataList.end(); i++)
    {
        if (i->logName == name)
        {
            return i->logEnable;
        }
    }
    return false;
}

unsigned int DATA::getLogMaxSize( const String8& name )
{
    List<CommonData>::iterator i;

    for (i = CommonDataList.begin(); i != CommonDataList.end(); i++)
    {
        if (i->logName == name)
        {
            return i->logMaxSize;
        }
    }
    return 0;
}

/*-----------------------ArgvData class-------------------*/

/*define static variable*/

ArgvData::supportArgv ArgvData::Argvs[e_ArgvMax] = 
{
    {String8("mode"),e_ArgvMode},
    {String8("path"),e_ArgvPath},
};

ArgvData::e_Argv ArgvData::getArgvType( const String8& argv )
{
    int i = 0;

    for (i = 0; i < e_ArgvMax; i++)
    {
        if (Argvs[i].Argv == argv)
        {
            return Argvs[i].eArgv;
        }
    }
    return e_ArgvMax;
}

int ArgvData::saveArgv( const String8& key, const String8& value )
{
    e_Argv eargv;

    eargv = getArgvType( key );

    switch (eargv)
    {

    case e_ArgvMode:
        mode = value.string();
        break;

    case e_ArgvPath:
        path = value.string();
        break;

    default:
        break;

    }

    return 0;

}

bool ArgvData::isSpecialMode( String8& out )
{
    //----->
    //mode = "boot";
    //-----<
    out = mode.string();
    if (mode.isEmpty())
    {
        printf("normal mode\n");
        return 0;
    }
    printf("%s mode\n",mode.string());
    return 1;
}

int ArgvData::getPath( String8& out )
{
    out = path.string();
    return 0;
}

int ArgvData::read( int argc, char** argv )
{
    int k = argc;
    int i = 0;
    char *pstr = NULL;
    char *pstrbegin = NULL;
    char *pstrend = NULL;
    char key[16];
    char value[128];
    int len = 0;

    if (k < 2)
    {
        printf( "no argv\n" );
        ALOGE( "no argv\n" );
        return -1;
    }

    for (i = 1; i < k; i++)
    {
        pstrbegin = strstr( argv[i], "<" );
        pstrend = strstr( argv[i], ">" );

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
                    saveArgv( String8(key), String8(value) );
                }


            }/*end of if(( pstr = strstr( pstrbegin, "=") ) != 0)*/

        }/*end of if(pstrbegin && pstrend)*/
    }/*end of for*/

    return 0;

}


/*----------------------------propertyData class-----------*/

const String8 PropertyData::propertyName = String8("persist.sys.logger.config");

bool PropertyData::isDebugMode( void )
{
   char propbuf[100];

   property_get( propertyName, propbuf, "empty" );

   if (String8("empty") == propbuf)
   {
       return false;
   }

   return true;

}

int PropertyData::read( void )
{
   char propbuf[100];
   String8 prop;

   property_get( propertyName, propbuf, "empty" );
   ALOGD("read config %s\n", propbuf);
   prop = propbuf;

   split( String8(prop.string()), String8(",") );

   if (prop.contains("kmsg"))
   {
       configLogEnable( String8("dmesg"),true );
   }
   if (prop.contains("tz"))
   {
       configLogEnable( String8("qsee"),true );
       configLogEnable( String8("tz"),true );
   }
   if (prop.contains("logcat"))
   {
       configLogEnable( String8("main"),true );
   }
   if (prop.contains("modem"))
   {
       configLogEnable( String8("modem"),true );
   }
   if (prop.contains("net"))
   {
       configLogEnable( String8("net"),true );
   }
    if (prop.contains("bugreport"))
    {
        configLogEnable( String8("bugreport"),true );
    }
   if (prop.contains("cfg"))
   {
       userSpecifyLogMask();
   }
   return 0;
}


/*-------------------------configData class----------------*/


int ConfigData::read( int argc, char** argv )
{
    String8 mode;
    String8 path;

    /*parse argv first!!*/
    argvData.init();
    argvData.read( argc, argv );

    if (argvData.isSpecialMode( mode ))
    {
        /*use argv*/
        DataBase = &argvData;
        printf("use argv\n");
        ALOGD("use argv\n");
        if (mode == "boot")
        {
            /*boot mode,capture boot log*/
            //argvData.configLogPath( String8("/data/local/tmp/boot") );
            argvData.configLogPath( String8("/data/zxlog/boot") );
            argvData.configLogEnable( String8("dmesg"), true );
            //argvData.configLogEnable( String8("modem"), true );
            //argvData.configLogEnable( String8("net"), true );
            argvData.configLogEnable( String8("main"), true );
        }
        else if (mode == "boot_hung_monitor")
        {
            argvData.configLogPath( String8("/proc") );
            argvData.configLogEnable( String8("main"), true );
            argvData.configLogMaxSize( String8("main"), 0 );

        }
        else if (mode == "ota")
        {
            /*ota mode,capture ota log*/
            argvData.configLogPath( String8("/data/local/tmp/ota") );
            argvData.configLogEnable( String8("dmesg"), true );

        }
        else if (mode == "recovery")
        {
            /*recovery mode,capture recovery log*/
            argvData.configLogPath( String8("/data/local/tmp/recovery") );
            argvData.configLogEnable( String8("dmesg"), true );
        }
        else if (mode == "poweroffcharger")
        {
            /*oh,in poweroffcharger mode,capture power off charger log*/
            argvData.configLogPath( String8("/data/local/tmp/poweroffcharger") );
            argvData.configLogEnable( String8("dmesg"), true );
        }

        /*special log path by argv*/
        argvData.getPath( path );
        if (!path.isEmpty())
        {
            printf("%s special log path\n",path.string());
            argvData.configLogPath( path );
        }

        /*save mode*/
        if (!mode.isEmpty())
        {
            argvData.configMode( mode );
            
            /*not support max size in special mode*/
            argvData.configLogMaxSize( String8("dmesg"), 0 );
            argvData.configLogMaxSize( String8("main"), 0 );
            /*max files that can be save in special mode*/
            argvData.configMaxFiles( 3 );
        }

    }
    else
    {
        /*no argv,use persist.sys.logger.config if exist */
        if (propertyData.isDebugMode())
        {
            propertyData.init();
            propertyData.read();
            /*use property*/
            DataBase = &propertyData;
            printf("use property\n");
            ALOGD("use property\n");
        }
        else
        {
            /*no argv,no persist.sys.logger.Config
             *so use socket
            */
            socketData.init();
            /*use socket*/
            DataBase = &socketData;
            printf("use socket\n");
            ALOGD("use socket\n");
        }
        
    }

    return 0;

}

int ConfigData::getMode( String8& mode )
{
    if (DataBase)
    {
        return DataBase->getMode( mode );
    }
    return -1;
}

int ConfigData::getMaxFiles( void )
{
    if (DataBase)
    {
        return DataBase->getMaxFiles();
    }
    return 0;
}

int ConfigData::getLogPath( String8& path )
{
    if (DataBase)
    {
        return DataBase->getLogPath( path );
    }

    return -1;
}

int ConfigData::getModemLogMaskPath( String8& path )
{
    if (DataBase)
    {
        return DataBase->getModemLogMaskPath( path );
    }

    return -1;

}

int ConfigData::getDynamicLogMaskPath( String8& path )
{
    if (DataBase)
    {
        return DataBase->getDynamicLogMaskPath( path );
    }

    return -1;
}

unsigned int ConfigData::getLogMaxSize( const String8& name )
{
    if (DataBase)
    {
        return DataBase->getLogMaxSize( name );
    }

    return 0;
}

bool ConfigData::isLogEnable( const String8& name )
{
    if (DataBase)
    {
        return DataBase->isLogEnable( name );
    }

    return false;
}
