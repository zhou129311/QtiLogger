# QtiLogger
基于高通8.1的Android Log录制工具，可以录制Android Log，Bluetooth Log，kmsg，QXDM，net log，bugreport等等。
需要在源码中进行编译，需要自行配置SELinux权限。
参考配置：
file_contexts中添加：
############################
# add zxlogger
/vendor/bin/zxlogger        u:object_r:zxlogger_exec:s0
/vendor/bin/mytcpdump        u:object_r:mytcpdump_exec:s0
/vendor/etc/logcp.sh         u:object_r:logcp_exec:s0
/data/wizlog/*               u:object_r:zxlog_file:s0

file.te中添加：
type zxlog_file, file_type, data_file_type, core_data_file_type, mlstrustedobject;

shell.te中添加：
allow shell logcp_exec:file rx_file_perms;

添加zxlogger.te文件：
type zxlogger, domain;
type zxlogger_exec, exec_type, file_type, vendor_file_type;

init_daemon_domain(zxlogger)
net_domain(zxlogger)

#allow zxlogger debugfs:file { write open };
allow zxlogger kernel:system syslog_mod;
allow zxlogger logd:unix_stream_socket connectto;
allow zxlogger logdr_socket:sock_file write;
allow zxlogger proc:file { read write open };
allow zxlogger rootfs:dir { read open };
allow zxlogger self:capability2 syslog;
allow zxlogger self:capability { dac_override net_raw setgid setuid };
allow zxlogger media_rw_data_file:dir { write search read create open add_name getattr };
allow zxlogger media_rw_data_file:file { write create open getattr };
allow zxlogger sdcardfs:dir { rw_dir_perms write create open setattr read add_name search remove_name getattr rmdir};
allow zxlogger sdcardfs:file { write create open setattr read rename getattr unlink lock };
allow zxlogger vendor_shell_exec:file execute_no_trans;
allow zxlogger mytcpdump_exec:file execute_no_trans;
#allow zxlogger self:packet_socket { read write create ioctl setopt };
#allow zxlogger dumpstate:unix_stream_socket connectto;
allow zxlogger dumpstate_socket:sock_file write;
allow zxlogger system_file:file execute_no_trans;
allow zxlogger zxlog_file:file { write create open setattr read rename getattr unlink lock };
allow zxlogger zxlog_file:dir { rw_dir_perms write create open setattr read add_name search remove_name getattr rmdir};
#allow zxlogger system_data_file:dir { write read create open add_name };
#allow zxlogger system_data_file:file { write create open };

set_prop(zxlogger, debug_prop)
#read_logd(zxlogger)
r_dir_file(zxlogger, domain)


添加mytcpdump.te文件：
type mytcpdump, domain;
type mytcpdump_exec, exec_type, file_type, vendor_file_type;

init_daemon_domain(mytcpdump)
net_domain(mytcpdump)

#allow mytcpdump debugfs:file { write open };
allow mytcpdump kernel:system syslog_mod;
#allow mytcpdump logd:unix_stream_socket connectto;
allow mytcpdump logdr_socket:sock_file write;
allow mytcpdump proc:file { read write open };
allow mytcpdump rootfs:dir { read open };
allow mytcpdump self:capability2 syslog;
allow mytcpdump self:capability dac_override;
allow mytcpdump media_rw_data_file:dir { write search read create open add_name getattr };
allow mytcpdump media_rw_data_file:file { write create open getattr };
allow mytcpdump sdcardfs:dir { rw_dir_perms write create open setattr read add_name search remove_name getattr rmdir};
allow mytcpdump sdcardfs:file { write create open setattr read rename getattr unlink lock };
allow mytcpdump vendor_shell_exec:file execute_no_trans;
#allow mytcpdump dumpstate:unix_stream_socket connectto;
allow mytcpdump dumpstate_socket:sock_file write;
allow mytcpdump system_file:file execute_no_trans;
allow mytcpdump zxlog_file:file { write create open setattr read rename getattr unlink lock };
allow mytcpdump zxlog_file:dir { rw_dir_perms write create open setattr read add_name search remove_name getattr rmdir};
#allow mytcpdump system_data_file:dir { write read create open add_name };
#allow mytcpdump system_data_file:file { write create open };



添加logcp.te文件：
type logcp, domain;
type logcp_exec, exec_type, file_type, vendor_file_type;

init_daemon_domain(logcp)

allow logcp system_file:file { read open getattr };
allow logcp vendor_shell_exec:file entrypoint;
allow logcp self:capability{ chown fowner fsetid setuid setgid dac_override };
allow logcp shell_exec:file { read open getattr };
allow logcp media_rw_data_file:dir { read open getattr search write add_name create };
allow logcp media_rw_data_file:file { write create open setattr read rename getattr unlink lock };
allow logcp sdcardfs:dir { read open search write add_name create };
allow logcp sdcardfs:file { write create open setattr read rename getattr unlink lock };
allow logcp vendor_toolbox_exec:file { execute_no_trans };
allow logcp vendor_toolbox_exec:file { execute_no_trans };
allow logcp zxlog_file:file { write create open setattr read rename getattr unlink lock };
allow logcp zxlog_file:dir { rw_dir_perms write create open setattr read add_name search remove_name getattr rmdir};
allow logcp bluetooth_data_file:file { write create open setattr read rename getattr unlink lock };
allow logcp bluetooth_data_file:dir { rw_dir_perms write create open setattr read add_name search remove_name getattr rmdir};
allow logcp bluetooth_logs_data_file:file { write create open setattr read rename getattr unlink lock };
allow logcp bluetooth_logs_data_file:dir { rw_dir_perms write create open setattr read add_name search remove_name getattr rmdir};

其他SELinux权限需要自行添加
