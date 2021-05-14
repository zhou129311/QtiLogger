//
// Created by zhouxian on 21-4-16.
//

#ifndef __LOGCATCORE_H__
#define __LOGCATCORE_H__

#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/List.h>
#include <log/logprint.h>

using namespace android;

class LogcatLogger
{

private:
    
    typedef class _LogcatDevice
    {

    private:

        char device[16];
        struct logger *plogger;
        struct logger_list *ploggerList;
        bool printed;
        bool binary;

    public:

        _LogcatDevice( const char *pdev, const bool b )
        {
            memset( device, 0, sizeof( device ) );
            if (strlen( pdev ) < sizeof( device ))
            {
                strcpy( device, pdev );
            }
            printed = false;
            binary = b;
            plogger = NULL;
            ploggerList = NULL;
        }

        String8 getName( void )
        {
            return String8( device );
        }

        void setLogger( struct logger *ploggerSet )
        {
            plogger = ploggerSet;
        }

        void setLoggerList( struct logger_list *ploggerListSet )
        {
            ploggerList = ploggerListSet;
        }

        bool getbinary( void )
        {
            return binary;
        }

        bool getPrinted( void )
        {
            return printed;
        }

        void setPrinted( bool print )
        {
            printed = print;
        }

    }LogcatDevice;

    static const int maxDevices;
    static String8 deviceName[]; 
    List<LogcatDevice> deviceList;
    AndroidLogFormat *plogFormat;
    String8 mode;
    int maxFiles;
    String8 logOutPath;
    String8 logFileName;
    unsigned int outByteCount;
    unsigned int maxLogSize;
    EventTagMap* eventTagMap;
    int outFD;

    bool isAvailDeviceName( const String8& device );
    bool isExistDevice( const String8& device );
    int createLogFile( void );
    int addLogcatDevice( const String8& device );
    int deleteLogcatDevice( const String8 & device );
    int processBuffer( struct log_msg *buf, bool binary );
    void printStart( const String8& device );

public:

    LogcatLogger( const String8& device1 = String8(""), const String8& device2 = String8(""), 
                  const String8& device3 = String8(""), const String8& device4 = String8(""),
                  const String8& device5 = String8("") );
    ~LogcatLogger()
    {
        printf( "LogcatLogger Destructors!\n" );
        if (plogFormat)
        {
            android_log_format_free( plogFormat );
            plogFormat = NULL;
        }

        if (eventTagMap)
        {
            android_closeEventTagMap( eventTagMap );
            eventTagMap = NULL;
        }
    }

    int showLogcatDevices( void );
    int setMode( const String8& _mode );
    int setMaxFiles( int max );
    int setLogFormatAndLogSize( const String8& format , unsigned int maxSize );
    int setLogPathAndFileName( const String8& path , const String8& name );
    int startCaptureLog( void );

};

#endif

