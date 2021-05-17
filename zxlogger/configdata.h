//
// Created by zhouxian on 21-4-16.
//

#ifndef __CONFIGDATA_H
#define __CONFIGDATA_H

#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/List.h>
#include <log/logprint.h>

using namespace android;

typedef class _DATA
{

private:

    typedef struct _CommonData
    {
        String8 logName;
        unsigned int logMaxSize;
        bool logEnable;

        _CommonData( const String8& _logName, unsigned int size )
        {
            logName = _logName.string();
            logEnable = false;
            logMaxSize = size;
        }

    }CommonData;

    const static unsigned int defaultLogMaxSize;

    String8 name;

    String8 logPath;
    String8 modemLogMaskPath;
    String8 dynamicLogMaskPath;
    String8 mode;/*zxlogger running mode,support boot,ota,poweroffcharge,recovery and normal mode*/
    int maxFiles;/*the max log files that can be save,normally be use in special mode*/

    List<struct _CommonData> CommonDataList;
    List<String8> PropConfigDataList;

    int addCommonData( const String8& _logName, unsigned int size = defaultLogMaxSize );
    int isVailLogMaskPath(const String8& str);

protected:
    int split(const String8& str, const String8& pattern);
    int userSpecifyLogMask( void );

public:

    _DATA( const String8& _name = String8("") )
    {
        printf( "%s Constructors!\n" , _name.string() );
        ALOGD( "%s Constructors!\n" , _name.string() );
        name = _name.string();
        maxFiles = 0;
    }
    ~_DATA()
    {
        printf( "%s Destructors!\n", name.string() );
        ALOGD( "%s Destructors!\n" , name.string() );
    }

    int init( void );
    int configMode( const String8& _mode );
    int configMaxFiles( int max );
    int configLogPath( const String8& path );
    int configModemLogMaskPath( const String8& path );
    int configDynamicLogMaskPath( const String8& path );
    int configLogEnable( const String8& name, bool enable );
    int configLogMaxSize( const String8& name, unsigned int size );

    int getLogPath( String8& path );
    int getModemLogMaskPath( String8& path );
    int getDynamicLogMaskPath( String8& path );
    unsigned int getLogMaxSize( const String8& name );
    bool isLogEnable( const String8& name );
    int getMode( String8& _mode );
    int getMaxFiles( void );

}DATA;

typedef class _ArgvData : public DATA
{
    /*support argv: <mode=boot> <path=/data/local/tmp>*/
private:

    typedef enum
    {
        e_ArgvMode,
        e_ArgvPath,
        e_ArgvMax,

    }e_Argv;

    typedef struct _supportArgv
    {
        String8 Argv;
        e_Argv eArgv;
    }supportArgv;

    String8 mode;
    String8 path;

    static supportArgv Argvs[];

    e_Argv getArgvType( const String8& argv );
    int saveArgv( const String8& key, const String8& value );

public:

    _ArgvData():DATA( String8("argv") ){};
    int read( int argc, char ** argv );
    bool isSpecialMode( String8& out );
    int getPath( String8& out );

}ArgvData;

typedef class _PropertyData : public DATA
{

private:
    static const String8 propertyName;
public:
    _PropertyData():DATA( String8("property") ){};
    bool isDebugMode( void );
    int read( void );

}PropertyData;

typedef class _SocketData : public DATA
{

public:
    int read( void );
    _SocketData():DATA( String8("socket") ){};

}SocketData;

typedef class _ConfigData
{

private:
    /*support mode : boot,ota,recovery,poweroffcharge,normal*/


    DATA *DataBase;

    ArgvData argvData;
    PropertyData propertyData;
    SocketData socketData;

public:

    _ConfigData(){ DataBase = NULL; };
    ~_ConfigData(){};
    int read( int argc, char** argv );
    int getMode( String8& mode );
    int getMaxFiles( void );
    int getLogPath( String8& path );
    int getModemLogMaskPath( String8& path );
    int getDynamicLogMaskPath( String8& path );
    unsigned int getLogMaxSize( const String8& name );
    bool isLogEnable( const String8& name );

}ConfigData;

#endif
