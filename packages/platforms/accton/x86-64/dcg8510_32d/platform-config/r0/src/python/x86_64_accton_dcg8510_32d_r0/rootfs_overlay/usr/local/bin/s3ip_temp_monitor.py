#!/usr/bin/env python

try:
    import getopt
    import sys
    import syslog
    import subprocess
    import logging
    import logging.config
    import logging.handlers
    import time  # this is only being used as part of the example
    #from sonic_platform.bfn_extensions.platform_sensors import *
    
except ImportError as e:
    raise ImportError('%s - required module not found' % str(e))

RET_TRUE  = 0
RET_FALSE = -1

FUNCTION_NAME = 'temp-to-bmc'
PRODUCT_NAME  = 'dcg8510_32d'

TEST_MODE = False
DEBUG_MODE = False
SHOW_MODE = False


class commands:
    @staticmethod
    def getstatusoutput(cmd):
        try:
            data = subprocess.check_output(cmd, shell=True, universal_newlines=True, stderr=subprocess.STDOUT)
            status = 0
        except subprocess.CalledProcessError as ex:
            data = ex.output
            status = ex.returncode
        if data[-1:] == '\n':
            data = data[:-1]
        return status, data


###############################################################################
# LOG.

# priorities (these are ordered)

LOG_EMERG     = 0       #  system is unusable
LOG_ALERT     = 1       #  action must be taken immediately
LOG_CRIT      = 2       #  critical conditions
LOG_ERR       = 3       #  error conditions
LOG_WARNING   = 4       #  warning conditions
LOG_NOTICE    = 5       #  normal but significant condition
LOG_INFO      = 6       #  informational
LOG_DEBUG     = 7       #  debug-level messages

priority_name = {
    LOG_CRIT    : "critical",
    LOG_DEBUG   : "debug",
    LOG_WARNING : "warning",
    LOG_INFO    : "info",
}

def SYS_LOG(level, msg):
    if TEST_MODE:
        time_str = time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime())
        print('%s %s: %-12s %s' %(time_str, FUNCTION_NAME, priority_name[level].upper(), msg))
    else:
        syslog.syslog(level, msg)

def DBG_LOG(msg):
    if DEBUG_MODE:
        level = syslog.LOG_DEBUG
        x = PRODUCT_NAME + ' ' + priority_name[level].upper() + ' : ' + msg
        SYS_LOG(level, x)

def SYS_LOG_INFO(msg):
    level = syslog.LOG_INFO
    x = PRODUCT_NAME + ' ' + priority_name[level].upper() + ' : ' + msg
    SYS_LOG(level, x)

def SYS_LOG_WARN(msg):
    level = syslog.LOG_WARNING
    x = PRODUCT_NAME + ' ' + priority_name[level].upper() + ' : ' + msg
    SYS_LOG(level, x)

def SYS_LOG_CRITICAL(msg):
    level = syslog.LOG_CRIT
    x = PRODUCT_NAME + ' ' + priority_name[level].upper() + ' : ' + msg
    SYS_LOG(level, x)

# Deafults
VERSION = '1.0'
FUNCTION_NAME = '/usr/local/bin/dcg8510_32d_temp_to_bmc'
DEBUG = False

TEMP_TIME = 10

def my_log(txt):
    if DEBUG == True:
        print("[ACCTON DBG]: "+txt)
    return

def log_os_system(cmd):
    logging.info('Run :'+cmd)
    status = 1
    output = ""
    status, output = commands.getstatusoutput(cmd)
    if status:
        logging.info('Failed :'+cmd)
    return  status, output
    
##############################################################################################   
def get_sfp_temperature():
    max_result = 0;
    
    for port in range(32):
        
        present_cmd = 'cat /sys/switch/transceiver/eth%d/present' % (port + 1)
        ret, present_output = commands.getstatusoutput(present_cmd)
        if(RET_TRUE != ret):
            DBG_LOG('fail to get SFP(%d) present.' %(port))
            continue

        if (0 == int(present_output)):
            continue

        tmp_int = 0;
        tmp_cmd = 'cat /sys/switch/transceiver/eth%d/temp_input' % (port + 1)
        ret, tmp_output = commands.getstatusoutput(tmp_cmd)
        if(RET_TRUE != ret):
            DBG_LOG('fail to get SFP(%d) temperature.' %(port + 1))
            continue
        
        tmp_int = float(tmp_output)
        if(tmp_int > max_result):
            max_result = tmp_int

    return max_result
    
def get_tf2_pipe_temperature():
    max_result = 0.0
    
    try:
        #max_result = platform_tf2_max_pipe_temp_get()
        max_result = 0
    except Exception as e:
         repr(e)
         return 0

    if(max_result < 0):
        tmp_val = abs(int(max_result-0.5))
        result = ((~tmp_val)&0xff)+1
    else:
        result = int(max_result+0.5)
        
    return result

def get_x86_core_temperature():
    max_result = 0;

    for x86_core in range(2,6):
        tmp_int = 0;
        tmp_cmd = 'cat /sys/switch/sensor/temp%d/temp_input' % (x86_core)
        ret, tmp_output = commands.getstatusoutput(tmp_cmd)
        if(RET_TRUE != ret):
            DBG_LOG('fail to get x86_core(%d) temperature.' %x86_core)
            continue
        
        tmp_int = float(tmp_output)
        if(tmp_int > max_result):
            max_result = tmp_int

    return (max_result/1000)

def send_sfp_temp_to_bmc(temp):
    cmd = 'ipmitool raw 0x36 0x16 %s' % (hex(int(temp))) 
    
    ret, output = commands.getstatusoutput(cmd)
    #SYS_LOG_INFO('%s, ret:%d, "%s"' %(cmd, ret, output))
    if(RET_TRUE != ret):
        DBG_LOG('fail to send SFP temperature to BMC.')
        return RET_FALSE

    return RET_TRUE

def send_tf2_pipe_temp_to_bmc(temp):
    cmd = 'ipmitool raw 0x36 0x17 %s' % (hex(int(temp))) 
    
    ret, output = commands.getstatusoutput(cmd)
    #SYS_LOG_INFO('%s, ret:%d, "%s"' %(cmd, ret, output))
    if(RET_TRUE != ret):
        DBG_LOG('fail to send tfe pipe temperature to BMC.')
        return RET_FALSE

    return RET_TRUE

def send_x86_core_temp_to_bmc(temp):
    cmd = 'ipmitool raw 0x36 0xA3 %s' % (hex(int(temp))) 
    
    ret, output = commands.getstatusoutput(cmd)
    #SYS_LOG_INFO('%s, ret:%d, "%s"' %(cmd, ret, output))
    if(RET_TRUE != ret):
        DBG_LOG('fail to send x86 core temperature to BMC.')
        return RET_FALSE

    return RET_TRUE

class device_monitor(object):
   
    def __init__(self, log_file, log_level):
        """Needs a logger and a logger level."""
        # set up logging to file
        logging.basicConfig(
            filename=log_file,
            filemode='w',
            level=log_level,
            format= '[%(asctime)s] {%(pathname)s:%(lineno)d} %(levelname)s - %(message)s',
            datefmt='%H:%M:%S'
        )
        
        # set up logging to console
        if log_level == logging.DEBUG:
            console = logging.StreamHandler()
            console.setLevel(log_level)
            formatter = logging.Formatter('%(name)-12s: %(levelname)-8s %(message)s')
            console.setFormatter(formatter)
            logging.getLogger('').addHandler(console)

        sys_handler = handler = logging.handlers.SysLogHandler(address = '/dev/log')
        sys_handler.setLevel(logging.WARNING)       
        logging.getLogger('').addHandler(sys_handler)

        #logging.debug('SET. logfile:%s / loglevel:%d', log_file, log_level)

    def send_temp_to_bmc_progress(self):
        #sfp
        sfp_tmp = get_sfp_temperature()
        send_sfp_temp_to_bmc(round(sfp_tmp))
        
        time.sleep(TEMP_TIME)

        #tf2 pipe
        tf2_pipe_tmp = get_tf2_pipe_temperature()
        send_tf2_pipe_temp_to_bmc(round(tf2_pipe_tmp))

        time.sleep(TEMP_TIME)

        #x86 core temperature
        x86_core_tmp = get_x86_core_temperature()
        #send_x86_core_temp_to_bmc(round(x86_core_tmp))
        

        time.sleep(TEMP_TIME)
        return True
        
def main(argv):
    log_file  = '%s.log' % FUNCTION_NAME
    log_level = logging.INFO

    if len(sys.argv) != 1:
        try:
            opts, args = getopt.getopt(argv,'hdlr',['lfile='])
        except getopt.GetoptError:
            print('A:Usage: %s [-d] [-l <log_file>]' % sys.argv[0])
            return 0
        
        for opt, arg in opts:
            if opt == '-h':
                print('B:Usage: %s [-d] [-l <log_file>]' % sys.argv[0])
                return 0
            elif opt in ('-d', '--debug'):
                log_level = logging.DEBUG
            elif opt in ('-l', '--lfile'):
                log_file = arg

    monitor = device_monitor(log_file, log_level)

    while True:
        #1 heartbeat progress
        monitor.send_temp_to_bmc_progress()

if __name__ == "__main__":
    main(sys.argv)
