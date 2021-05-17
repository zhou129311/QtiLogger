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

bool NetDevice::threadLoop()
{
    ALOGD( "%s threadLoop run!\n", getName().string() );

    char propbuf[100];
    //String8 net;
    //String8 tname;
    property_get("persist.netlog.option", propbuf, "all");
    if (!strncmp(propbuf, "wlan", 4)) {
        //String8 wlan("/system/bin/mytcpdump -p -i wlan0 -vv -s 0 -w ");
        //net = wlan.string();
        //tname = "wlan";
        ALOGD( "debug.sys.logger.tcpdump_wlan  >> 1 \n");
        property_set("debug.sys.logger.tcpdump_wlan","1");
    } else {
        //String8 all("/system/bin/mytcpdump -p -i any -vv -s 0 -w ");
        //net = all.string();
        //tname = "all";
        ALOGD( "debug.sys.logger.tcpdump_all  >> 1 \n");
        property_set("debug.sys.logger.tcpdump_all","1");
    }
    //String8 timeStamp("");
    //generateTimestamp( timeStamp );
    //String8 cmd("");

    //cmd = net + logPath + "/" + logDir + "/tcp_" + tname + "_"
    //          + timeStamp + ".pcap";
    //ALOGD("tcpdump cmd is  %s \n", cmd.string() );

    //system( cmd.string() );

    for ( ;; )
    {
        sleep( 60 );
    }

    ALOGD( "%s threadLoop exit!\n", getName().string() );

    return 0;

}
