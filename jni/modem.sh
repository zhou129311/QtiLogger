#!/system/bin/sh
/system/bin/diag_mdlog -c -k
/system/bin/stop zxlogger
/system/bin/stop zxbootlogger

exit 0
