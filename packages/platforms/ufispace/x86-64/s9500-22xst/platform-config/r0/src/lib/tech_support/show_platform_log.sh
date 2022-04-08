#!/bin/bash

SH_VER="20211005"

# TRUE=0, FALSE=1
TRUE=0
FALSE=1


# DATESTR: The format of log folder and log file
DATESTR=$(date +"%Y%m%d%H%M%S")
LOG_FOLDER_NAME="log_platform_${DATESTR}"
LOG_FILE_NAME="log_platform_${DATESTR}.log"

# LOG_FOLDER_ROOT: The root folder of log files
LOG_FOLDER_ROOT="/tmp/log"
LOG_FOLDER_PATH="${LOG_FOLDER_ROOT}/${LOG_FOLDER_NAME}"
LOG_FILE_PATH="${LOG_FOLDER_PATH}/${LOG_FILE_NAME}"


# MODEL_NAME: set by function _board_info
MODEL_NAME=""
# HW_REV: set by function _board_info
HW_REV=""
# BSP_INIT_FLAG: set bu function _check_bsp_init
BSP_INIT_FLAG=""

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
IOGET="${SCRIPTPATH}/ioget"

# LOG_FILE_ENABLE=1: Log all the platform info to log files (${LOG_FILE_NAME})
# LOG_FILE_ENABLE=0: Print all the platform info in console
LOG_FILE_ENABLE=1


# Log Redirection
# LOG_REDIRECT="2> /dev/null": remove the error message from console
# LOG_REDIRECT=""            : show the error message in console
LOG_REDIRECT="2> /dev/null"

# GPIO_OFFSET: update by function _update_gpio_offset
GPIO_OFFSET=0



function _echo {
    str="$1"

    if [ "${LOG_FILE_ENABLE}" == "1" ] && [ -f "${LOG_FILE_PATH}" ]; then
        echo "${str}" >> "${LOG_FILE_PATH}"
    else
        echo "${str}"
    fi
}

function _run_echo {
    str="$1"

    if [ "${LOG_FILE_ENABLE}" == "1" ] && [ -f "${LOG_FILE_PATH}" ]; then
        eval ${str} >> "${LOG_FILE_PATH}"
    else
        eval ${str}
    fi
}

function _banner {
   banner="$1"

   if [ ! -z "${banner}" ]; then
       _echo ""
       _echo "##############################"
       _echo "#   ${banner}"
       echo  "#   ${banner}..."
       _echo "##############################"
   fi
}

function _pkg_version {
    _banner "Package Version = ${SH_VER}"
}

function _update_gpio_offset {
    _banner "Update GPIO Offset"

    max_gpiochip=`ls /sys/class/gpio/ | sort -r | grep -m1 gpiochip`
    max_gpiochip_num="${max_gpiochip#*gpiochip}"

    if [ -z "${max_gpiochip_num}" ]; then
        GPIO_OFFSET=0
    elif [ ${max_gpiochip_num} -lt 256 ]; then
        GPIO_OFFSET=256
    else
        GPIO_OFFSET=0
    fi

    _echo "[GPIOCHIP MAX    ]: ${max_gpiochip}"
    _echo "[GPIOCHIP MAX NUM]: ${max_gpiochip_num}"
    _echo "[GPIO OFFSET     ]: ${GPIO_OFFSET}"
}

function _check_env {
    #_banner "Check Environment"

    # check utility
    if [ ! -f "${IOGET}" ]; then
        echo "Error!!! ioget(${IOGET}) file not found!!! Exit!!!"
        echo "Please update the ioget file path in script or put the ioget under ${IOGET}."
        exit 1
    fi

    # check basic commands
    cmd_array=("ipmitool" "lsusb" "dmidecode")
    for (( i=0; i<${#cmd_array[@]}; i++ ))
    do
        ret=`which ${cmd_array[$i]}`

        if [ ! $? -eq 0 ]; then
            _echo "${cmd_array[$i]} command not found!!"
            exit 1
        fi
    done

    if [ "${LOG_FILE_ENABLE}" == "1" ]; then
        mkdir -p "${LOG_FOLDER_PATH}"
        echo "${LOG_FILE_NAME}" > "${LOG_FILE_PATH}"
    fi
    
    # check BSP init
    _check_bsp_init
    _update_gpio_offset
}

function _check_filepath {
    filepath=$1
    if [ -z "${filepath}" ]; then
        _echo "ERROR, the ipnut string is empyt!!!"
        return ${FALSE}
    elif [ ! -f "$filepath" ]; then
        _echo "ERROR: No such file: ${filepath}"
        return ${FALSE}
    else
        #_echo "File Path: ${filepath}"
        return ${TRUE}
    fi
}

function _check_i2c_device {
    i2c_addr=$1

    if [ -z "${i2c_addr}" ]; then
        _echo "ERROR, the ipnut string is empyt!!!"
        return ${FALSE}
    fi

    value=$(eval "i2cget -y -f 0 ${i2c_addr} ${LOG_REDIRECT}")
    ret=$?

    if [ $ret -eq 0 ]; then
        return ${TRUE}
    else
        _echo "ERROR: No such device: ${i2c_addr}"
        return ${FALSE}
    fi
}

function _check_bsp_init {
    _banner "Check BSP Init"

    i2c_bus_0=$(eval "i2cdetect -y 0 ${LOG_REDIRECT} | grep UU")
    ret=$?
    if [ $ret -eq 0 ] && [ ! -z "${i2c_bus_0}" ] ; then
        BSP_INIT_FLAG=1
    else
        BSP_INIT_FLAG=0
    fi

    _echo "[BSP_INIT_FLAG]: ${BSP_INIT_FLAG}"
}

function _show_system_info {
    _banner "Show System Info"

    x86_date=`date`
    x86_uptime=`uptime`
    bmc_date=$(eval "ipmitool sel time get ${LOG_REDIRECT}")
    last_login=`last`

    _echo "[X86 Date Time ]: ${x86_date}"
    _echo "[BMC Date Time ]: ${bmc_date}"
    _echo "[X86 Up Time   ]: ${x86_uptime}"
    _echo "[X86 Last Login]: "
    _echo "${last_login}"
    _echo ""

    cmd_array=("uname -a" "cat /proc/cmdline" "cat /proc/ioports" \
               "cat /proc/iomem" "cat /proc/meminfo" \
               "cat /proc/sys/kernel/printk" )
    
    for (( i=0; i<${#cmd_array[@]}; i++ ))
    do
        _echo "[Command]: ${cmd_array[$i]}"
        ret=$(eval "${cmd_array[$i]} ${LOG_REDIRECT}")
        _echo "${ret}"
        _echo ""
    done

}

function _show_board_info {
    _banner "Show Board Info"
        
    #check hw rev value
    if [ "${BSP_INIT_FLAG}" == "1" ]; then
        hw_rev_value=`i2cget -y -f 3 0x20 0x1`
        rov_raw_value=`i2cget -y -f 3 0x20 0x0`
    else
        i2cset -y 0 0x75 0x4
        if [ "$?" != "0" ]; then
            _echo "0x75 does not exist, Exit!!"
            exit 255;
        fi
        
        hw_rev_value=`i2cget -y 0 0x20 0x1`
        rov_raw_value=`i2cget -y 0 0x20 0x0`
    fi

    if [ "$((hw_rev_value & 0xC))" == 12 ]; then
        hw_rev="PVT"
    elif [ "$((hw_rev_value & 0x8))" == 8 ]; then
        hw_rev="ALPHA"
    elif [ "$((hw_rev_value & 0x4))" == 4 ]; then
        hw_rev="BETA"
    else
        hw_rev="PROTO"
    fi
    
    #check board id value
    if [ "$((hw_rev_value & 0xf0))" == "$((16#00))" ]; then
        model_name="S9500-22XST"
        MODEL_NAME=${model_name}
    else
        _echo "Not support platform, Exit!!"
        exit 255;
    fi
    
    #check build rev value
    if [ "$((hw_rev_value & 0x3))" == 3 ]; then
        build_rev="3"
    elif [ "$((hw_rev_value & 0x3))" == 1 ]; then
        build_rev="2"
    elif [ "$((hw_rev_value & 0x3))" == 2 ]; then
        build_rev="1"
    else
        build_rev="0"
    fi
    
    # finish work
    if [ "${BSP_INIT_FLAG}" == "0" ]; then
        i2cset -y 0 0x75 0x0
    fi

    _echo "[Board Type and Revision]: ${model_name} ${hw_rev} ${build_rev}"
    _echo "[Misc. ROV Raw Info]: ${rov_raw_value}"
}

function _bios_version {
    _banner "Show BIOS Version"

    bios_ver=$(eval "dmidecode -s bios-version ${LOG_REDIRECT}")
    bios_boot_rom=`${IOGET} 0x602`
    if [ $? -eq 0 ]; then
        bios_boot_rom=`echo ${bios_boot_rom} | awk -F" " '{print $NF}'`
    fi

    _echo "[BIOS Vesion  ]: ${bios_ver}"
    _echo "[BIOS Boot ROM]: ${bios_boot_rom}"
}

function _bmc_version {
    _banner "Show BMC Version"

    bmc_rom1_ver=$(eval "ipmitool raw 0x32 0x8f 0x8 0x1 ${LOG_REDIRECT}")
    bmc_rom2_ver=$(eval "ipmitool raw 0x32 0x8f 0x8 0x2 ${LOG_REDIRECT}")
    bmc_active_rom=$(eval "ipmitool raw 0x32 0x8f 0x7 ${LOG_REDIRECT}")

    _echo "[BMC ROM1 Ver  ]: ${bmc_rom1_ver}"
    _echo "[BMC ROM2 Ver  ]: ${bmc_rom2_ver}"
    _echo "[BMC Active ROM]: ${bmc_active_rom}"
}

function _cpld_version {
    _banner "Show CPLD Version"

    # CPU CPLD
    cpu_cpld_info=`${IOGET} 0x600`
    ret=$?
    if [ $ret -eq 0 ]; then
        cpu_cpld_info=`echo ${cpu_cpld_info} | awk -F" " '{print $NF}'`
    else
        _echo "Get CPU CPLD version info failed ($ret), Exit!!"
        exit $ret
    fi

    _echo "[CPU CPLD Reg Raw]: ${cpu_cpld_info} " 
    _echo "[CPU CPLD Version]: $(( (cpu_cpld_info & 2#01000000) >> 6)).$(( cpu_cpld_info & 2#00111111 ))" 

    # Main CPLD
    mb_cpld_info=`${IOGET} 0x702`
    ret=$?
    if [ $ret -eq 0 ]; then
        mb_cpld_info=`echo ${mb_cpld_info} | awk -F" " '{print $NF}'`
    else
        _echo "Get MB CPLD version info failed ($ret), Exit!!"
        exit $ret
    fi

    _echo "[MB CPLD Reg Raw]: ${mb_cpld_info} " 
    _echo "[MB CPLD Version]: $(( (mb_cpld_info & 2#01000000) >> 6)).$(( mb_cpld_info & 2#00111111 ))" 
    
}

function _ucd_version {
    _banner "Show UCD Version"


    brd=("MB")


    #get ucd date via BMC
    ucd_date_raw=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x7 0x9d ${LOG_REDIRECT}")
    
    ret=$?
    
    #check return code
    if [ ! $ret -eq 0 ] ; then
        _echo "Get UCD date fail!"
        return $ret
    fi
    
    #add prefix 0x for each byte
    ucd_date_hex=${ucd_date_raw// /0x}
    
    #convert hex to ascii
    ucd_date_ascii=`echo $ucd_date_hex | xxd -r`
    
    ucd_date_revision=${ucd_date_ascii}
    
    #get ucd version via BMC
    ucd_ver_raw=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x6 0x9b ${LOG_REDIRECT}")
    
    ret=$?
    
    #check return code
    if [ ! $ret -eq 0 ] ; then
        _echo "Get UCD version fail!"
        return $ret
    fi
    
    #add prefix 0x for each byte
    ucd_ver_hex=${ucd_ver_raw// /0x}
    
    #convert hex to ascii
    ucd_ver_ascii=`echo $ucd_ver_hex | xxd -r`
    
    #first len-6 char are for revision
    ucd_ver_revision=${ucd_ver_ascii}
    
    _echo "[${brd[i]} MFR_REVISION]: ${ucd_ver_revision}"
    _echo "[${brd[i]} MFR_DATE    ]: ${ucd_date_revision}"
}

function _show_version {
    _bios_version
    _bmc_version
    _cpld_version
    _ucd_version
}

function _show_i2c_tree_bus_0 {
    _banner "Show I2C Tree Bus 0"

    ret=$(eval "i2cdetect -y 0 ${LOG_REDIRECT}")

    _echo "[I2C Tree]:"
    _echo "${ret}"
}

function _show_i2c_mux_devices {
    local chip_addr=$1
    local channel_num=$2
    local chip_dev_desc=$3
    local i=0;

    if [ -z "${chip_addr}" ] || [ -z "${channel_num}" ] || [ -z "${chip_dev_desc}" ]; then
        _echo "ERROR: parameter cannot be empty!!!"
        exit 99
    fi

    _check_i2c_device "$chip_addr"
    ret=$?
    if [ "$ret" == "0" ]; then
        _echo "TCA9548 Mux ${chip_dev_desc}"
        _echo "---------------------------------------------------"
        for (( i=0; i<${channel_num}; i++ ))
        do
            _echo "TCA9548 Mux ${chip_dev_desc} - Channel ${i}"
            # open mux channel - 0x75
            i2cset -y 0 ${chip_addr} $(( 2 ** ${i} ))
            # dump i2c tree
            ret=$(eval "i2cdetect -y 0 ${LOG_REDIRECT}")
            _echo "${ret}"
            # close mux channel
            i2cset -y 0 ${chip_addr} 0x0 
            _echo ""
        done
    fi

}

function _show_i2c_tree_bus_mux_i2c {
    _banner "Show I2C Tree Bus MUX (I2C)"

    local chip_addr1=""
    local chip_addr2_1=""
    local chip_addr2_2=""
    local chip_addr2_3=""
    local chip_addr2_4=""

    if [ "${MODEL_NAME}" == "S9500-22XST" ]; then 
        ## ROOT-0x75
        _show_i2c_mux_devices "0x75" "8" "ROOT-0x75"
        
        ## ROOT-0x76
        _show_i2c_mux_devices "0x76" "8" "ROOT-0x76"

        ## ROOT-0x76-Channel(0~7)-0x76-Channel(0~7)
        chip_addr1="0x76"
        chip_addr2_1="0x72"
        chip_addr2_2="0x73"
        chip_addr2_3="0x74"
        chip_addr2_4="0x70"
        _check_i2c_device "${chip_addr1}"
        ret=$?
        if [ "$ret" == "0" ]; then
            # open mux channel - 0x76 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x8
            _show_i2c_mux_devices "${chip_addr2_1}" "8" "ROOT-${chip_addr1}-3-${chip_addr2_1}"
            # close mux channel - 0x72 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x0
            
            # open mux channel - 0x76 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x8
            _show_i2c_mux_devices "${chip_addr2_2}" "8" "ROOT-${chip_addr1}-3-${chip_addr2_2}"
            # close mux channel - 0x72 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x0
            
            # open mux channel - 0x76 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x8
            _show_i2c_mux_devices "${chip_addr2_3}" "8" "ROOT-${chip_addr1}-3-${chip_addr2_3}"
            # close mux channel - 0x72 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x0
            
            # open mux channel - 0x76 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x8
            _show_i2c_mux_devices "${chip_addr2_4}" "8" "ROOT-${chip_addr1}-3-${chip_addr2_4}"
            # close mux channel - 0x72 (chip_addr1)
            i2cset -y 0 ${chip_addr1} 0x0
        fi
    else
        echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi
}

function _show_i2c_tree_bus_mux_sysfs {
    _banner "Show I2C Tree Bus MUX (sysfs)"
    
    local i2c_bus=""

    if [ "${MODEL_NAME}" == "S9500-22XST" ]; then 
        all_i2c_bus=`ls /dev/i2c-* | awk -F"-" '{print $2}'`
        for i2c_bus in ${all_i2c_bus}
        do
            _echo "I2C bus: ${i2c_bus}"
            _echo "---------------------------------------------------"
            ret=$(eval "i2cdetect -y ${i2c_bus} ${LOG_REDIRECT}")
            _echo "${ret}"
        done        
    else
        echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi
}

function _show_i2c_tree {
    _banner "Show I2C Tree"

    _show_i2c_tree_bus_0
    
    if [ "${BSP_INIT_FLAG}" == "1" ]; then
        _show_i2c_tree_bus_mux_sysfs
    else
        _show_i2c_tree_bus_mux_i2c
    fi

    _show_i2c_tree_bus_0
}

function _show_i2c_device_info {
    _banner "Show I2C Device Info"

    ret=`i2cdump -y -f 0 0x77 b`
    _echo "[I2C Device 0x77]:"
    _echo "${ret}"
    _echo ""

    local pca954x_device_id=("")
    if [ "${MODEL_NAME}" == "S9500-22XST" ]; then 
        pca954x_device_id=("0x75" "0x76")
    else
        _echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi
   
    for ((i=0;i<5;i++))
    do
        _echo "[DEV PCA9548 (${i})]"
        for (( j=0; j<${#pca954x_device_id[@]}; j++ ))
        do
            ret=`i2cget -f -y 0 ${pca954x_device_id[$j]}`
            _echo "[I2C Device ${pca954x_device_id[$j]}]: $ret"
        done
        sleep 0.4
    done
}

function _show_sys_devices {
    _banner "Show System Devices"

    _echo "[Command]: ls /sys/class/gpio/"
    ret=($(ls /sys/class/gpio/))
    _echo "#${ret[*]}"
    
    # show all gpio value and direction
    dev_num=`ls /sys/class/gpio | grep -v 'gpiochip\|export'`
    for i in ${dev_num}
    do
        gpio_dir=`cat /sys/class/gpio/${i}/direction`
        gpio_val=`cat /sys/class/gpio/${i}/value`
        _echo "[${i}] Direction: ${gpio_dir} Value: ${gpio_val}"
    done

    local file_path="/sys/kernel/debug/gpio"
    if [ -f "${file_path}" ]; then
        _echo ""
        _echo "[Command]: cat ${file_path}"
        _echo "$(cat ${file_path})"
    fi

    _echo ""
    _echo "[Command]: ls /sys/bus/i2c/devices/"
    ret=($(ls /sys/bus/i2c/devices/))
    _echo "#${ret[*]}"

    _echo ""
    _echo "[Command]: ls /dev/"
    ret=($(ls /dev/))
    _echo "#${ret[*]}"
}

function _show_cpu_eeprom_i2c {
    _banner "Show CPU EEPROM"

    cpu_eeprom=$(eval "i2cdump -y 0 0x57 c")
    cpu_eeprom=$(eval "i2cdump -y 0 0x57 c")
    _echo "[CPU EEPROM]:"
    _echo "${cpu_eeprom}"
}

function _show_cpu_eeprom_sysfs {
    _banner "Show CPU EEPROM"

    cpu_eeprom=$(eval "cat /sys/bus/i2c/devices/0-0057/eeprom ${LOG_REDIRECT} | hexdump -C")
    _echo "[CPU EEPROM]:"
    _echo "${cpu_eeprom}"
}

function _show_cpu_eeprom {
    if [ "${BSP_INIT_FLAG}" == "1" ]; then
        _show_cpu_eeprom_sysfs
    else
        _show_cpu_eeprom_i2c
    fi
}

function _show_psu_status {
    _banner "Show PSU Status"

    bus_id=""
    if [ "${MODEL_NAME}" != "S9500-22XST" ]; then
        _echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi
    
    # Read PSU0 Power Good Status (1: power good, 0: not providing power)
    cpld_psu_status_0_reg=`${IOGET} 0x708 | awk -F'is ' '{print $2}'`
    psu0_power_ok=$(((cpld_psu_status_0_reg & 2#00001000) >> 3))

    # Read PSU0 Absent Status (0: psu present, 1: psu absent)
    psu0_absent_l=$(((cpld_psu_status_0_reg & 2#00000010) >> 1))

    # Read PSU1 Power Good Status (1: power good, 0: not providing power)
    psu1_power_ok=$(((cpld_psu_status_0_reg & 2#00000100) >> 2))

    # Read PSU1 Absent Status (0: psu present, 1: psu absent)
    psu1_absent_l=$(((cpld_psu_status_0_reg & 2#00000001)))


    _echo "[PSU0 Status Reg Raw   ]: ${cpld_psu_status_0_reg}"
    _echo "[PSU0 Power Good Status]: ${psu0_power_ok}"
    _echo "[PSU0 Absent Status (L)]: ${psu0_absent_l}"
    _echo "[PSU1 Status Reg Raw   ]: ${cpld_psu_status_0_reg}"
    _echo "[PSU1 Power Good Status]: ${psu1_power_ok}"
    _echo "[PSU1 Absent Status (L)]: ${psu1_absent_l}"    
}

function _show_rov {
    _echo "ROV info is included in _show_board_info"
}

function _show_port_status_sysfs {
    _banner "Show Port Status / EEPROM"
    echo "    Show Port Status / EEPROM, please wait..."

    bus_id=""

    if [ "${MODEL_NAME}" == "S9500-22XST" ]; then 
        sfp_bus_id="7"
        qsfp_bus_id="5"

        sfp_eeprom_bus_id_array=( "-1" "-1" "-1" "-1" "9" \
                                   "10" "11" "12" "13" "14" \
                                   "15" "16" "21" "22" "23" \
                                   "24" "25" "26" "27" "28" )

        sfp_status_ioexp_addr_array=("-1" "-1" "-1" "-1" "0x20" \
                                     "0x20" "0x20" "0x20" "0x20" "0x20" \
                                     "0x20" "0x20" "0x22" "0x22" "0x22" \
                                     "0x22" "0x22" "0x22" "0x22" "0x22" )
                                     
        sfp_status_ioexp_reg_array=( "-1"  "-1"  "-1"  "-1"  "1" \
                                      "1"  "1"  "1"  "1"  "1" \
                                      "1" "1"  "0"  "0"  "0" \
                                      "0"  "1"  "1"  "1"  "1"  )

        sfp_status_ioexp_index_array=( "-1"  "-1"  "-1"  "-1"  "80" \
                                        "40"  "20"  "10"  "08"  "04" \
                                        "02" "01"  "08"  "04"  "02" \
                                        "01"  "80"  "40"  "20"  "10" )

        for (( i=4; i<${#sfp_eeprom_bus_id_array[@]}; i++ ))
        do
            # Module NIF Port present status (0: Present, 1:Absence)
            sfp_port_status_reg=$(eval "i2cget -y -f ${sfp_bus_id} ${sfp_status_ioexp_addr_array[${i}]} ${sfp_status_ioexp_reg_array[${i}]} ${LOG_REDIRECT}")
            sfp_module_absent=$((sfp_port_status_reg & 16#${sfp_status_ioexp_index_array[${i}]}))
            if [ "${sfp_module_absent}" == "0" ]; then
                sfp_module_absent=0
            else
                sfp_module_absent=1
            fi


            # Module NIF Port Dump EEPROM
            if [ "${sfp_module_absent}" == "0" ]; then
                _check_filepath "/sys/bus/i2c/devices/${sfp_eeprom_bus_id_array[${i}]}-0050/eeprom"
                port_eeprom_A0_1st=$(eval "dd if=/sys/bus/i2c/devices/${sfp_eeprom_bus_id_array[${i}]}-0050/eeprom bs=128 count=2 skip=0 status=none ${LOG_REDIRECT} | hexdump -C")
                port_eeprom_A0_2nd=$(eval "dd if=/sys/bus/i2c/devices/${sfp_eeprom_bus_id_array[${i}]}-0050/eeprom bs=128 count=2 skip=0 status=none ${LOG_REDIRECT} | hexdump -C")
                port_eeprom_A2_1st=$(eval "i2cdump -y -f ${sfp_eeprom_bus_id_array[${i}]} 0x51 ${LOG_REDIRECT}")
                port_eeprom_A2_2nd=$(eval "i2cdump -y -f ${sfp_eeprom_bus_id_array[${i}]} 0x51 ${LOG_REDIRECT}")
                if [ -z "$port_eeprom_A0_1st" ]; then
                    port_eeprom_A0_1st="ERROR!!! The result is empty. It should read failed (/sys/bus/i2c/devices/${sfp_eeprom_bus_id_array[${i}]}-0050/eeprom)!!"
                fi

                # Full EEPROM Log
                #if [ "${LOG_FILE_ENABLE}" == "1" ]; then
                #    hexdump -C "/sys/bus/i2c/devices/${sfp_eeprom_bus_id_array[${i}]}-0050/eeprom" > ${LOG_FOLDER_PATH}/qsfp_port${i}_eeprom.log 2>&1
                #fi
            else
                port_eeprom_A0_1st="N/A"
                port_eeprom_A0_2nd="N/A"
                port_eeprom_A2_1st="N/A"
                port_eeprom_A2_2nd="N/A"
            fi

            _echo "[Port${i} Status Reg Raw]: ${sfp_port_status_reg}"
            #_echo "[Port${i} Module INT (L)]: ${port_module_interrupt_l}"
            _echo "[Port${i} Module Absent ]: ${sfp_module_absent}"
            #_echo "[Port${i} Config Reg Raw]: ${cpld_qsfp_port_config_reg}"
            #_echo "[Port${i} Low Power Mode]: ${port_lp_mode}"
            _echo "[Port${i} EEPROM A0h(1st)]:"
            _echo "${port_eeprom_A0_1st}"
            _echo "[Port${i} EEPROM A0h(2nd)]:"
            _echo "${port_eeprom_A0_2nd}"
            _echo "[Port${i} EEPROM A2h (1st)]:"
            _echo "${port_eeprom_A2_1st}"
            _echo "[Port${i} EEPROM A2h (2nd)]:"
            _echo "${port_eeprom_A2_2nd}"
            _echo ""
        done
        
        qsfp_eeprom_bus_id_array=( "36" "35" )

        qsfp_status_ioexp_addr_array=( "0x21" "0x21" )
                                     
        qsfp_status_ioexp_reg_array=( "0"  "0"  )

        qsfp_status_ioexp_index_array=( "20"  "10"  )

        for (( i=0; i<${#qsfp_eeprom_bus_id_array[@]}; i++ ))
        do
            # Module NIF Port present status (0: Present, 1:Absence)
            qsfp_port_status_reg=$(eval "i2cget -y -f ${qsfp_bus_id} ${qsfp_status_ioexp_addr_array[${i}]} ${qsfp_status_ioexp_reg_array[${i}]}  ${LOG_REDIRECT}")
            qsfp_module_absent=$((qsfp_port_status_reg & 16#${qsfp_status_ioexp_index_array[${i}]}))
            if [ "${qsfp_module_absent}" == "0" ]; then
                qsfp_module_absent=0
            else
                qsfp_module_absent=1
            fi


            # Module NIF Port Dump EEPROM
            if [ "${qsfp_module_absent}" == "0" ]; then
                _check_filepath "/sys/bus/i2c/devices/${qsfp_eeprom_bus_id_array[${i}]}-0050/eeprom"
                port_eeprom_p0_1st=$(eval "dd if=/sys/bus/i2c/devices/${qsfp_eeprom_bus_id_array[${i}]}-0050/eeprom bs=128 count=2 skip=0 status=none ${LOG_REDIRECT} | hexdump -C")
                port_eeprom_p0_2nd=$(eval "dd if=/sys/bus/i2c/devices/${qsfp_eeprom_bus_id_array[${i}]}-0050/eeprom bs=128 count=2 skip=0 status=none ${LOG_REDIRECT} | hexdump -C")
                port_eeprom_p17_1st=$(eval "dd if=/sys/bus/i2c/devices/${qsfp_eeprom_bus_id_array[${i}]}-0050/eeprom bs=128 count=1 skip=18 status=none ${LOG_REDIRECT} | hexdump -C")
                port_eeprom_p17_2nd=$(eval "dd if=/sys/bus/i2c/devices/${qsfp_eeprom_bus_id_array[${i}]}-0050/eeprom bs=128 count=1 skip=18 status=none ${LOG_REDIRECT} | hexdump -C")
                if [ -z "$port_eeprom_p0_1st" ]; then
                    port_eeprom_p0_1st="ERROR!!! The result is empty. It should read failed (/sys/bus/i2c/devices/${qsfp_eeprom_bus_id_array[${i}]}-0050/eeprom)!!"
                fi

                # Full EEPROM Log
                if [ "${LOG_FILE_ENABLE}" == "1" ]; then
                    hexdump -C "/sys/bus/i2c/devices/${qsfp_eeprom_bus_id_array[${i}]}-0050/eeprom" > ${LOG_FOLDER_PATH}/qsfp_port${i}_eeprom.log 2>&1
                fi
            else
                port_eeprom_p0_1st="N/A"
                port_eeprom_p0_2nd="N/A"
                port_eeprom_p17_1st="N/A"
                port_eeprom_p17_2nd="N/A"
            fi

            _echo "[Port${i} Status Reg Raw]: ${qsfp_port_status_reg}"
            #_echo "[Port${i} Module INT (L)]: ${port_module_interrupt_l}"
            _echo "[Port${i} Module Absent ]: ${port_module_absent}"
            #_echo "[Port${i} Config Reg Raw]: ${cpld_qsfp_port_config_reg}"
            #_echo "[Port${i} Low Power Mode]: ${port_lp_mode}"
            _echo "[Port${i} EEPROM Page0-0(1st)]:"
            _echo "${port_eeprom_p0_1st}"
            _echo "[Port${i} EEPROM Page0-0(2nd)]:"
            _echo "${port_eeprom_p0_2nd}"
            _echo "[Port${i} EEPROM Page17 (1st)]:"
            _echo "${port_eeprom_p17_1st}"
            _echo "[Port${i} EEPROM Page17 (2nd)]:"
            _echo "${port_eeprom_p17_2nd}"
            _echo ""
        done
    else
        _echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi
}

function _show_port_status {
    if [ "${BSP_INIT_FLAG}" == "1" ]; then
        _show_port_status_sysfs
    fi
}

function _show_cpu_temperature_sysfs {
    _banner "show CPU Temperature"

    for ((i=1;i<=5;i++))
    do
        if [ -f "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp${i}_input" ]; then
            _check_filepath "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp${i}_input"
            _check_filepath "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp${i}_max"
            _check_filepath "/sys/devices/platform/coretemp.0/hwmon/hwmon0/temp${i}_crit"
            temp_input=$(eval "cat /sys/devices/platform/coretemp.0/hwmon/hwmon0/temp${i}_input ${LOG_REDIRECT}")
            temp_max=$(eval "cat /sys/devices/platform/coretemp.0/hwmon/hwmon0/temp${i}_max ${LOG_REDIRECT}")
            temp_crit=$(eval "cat /sys/devices/platform/coretemp.0/hwmon/hwmon0/temp${i}_crit ${LOG_REDIRECT}")
        elif [ -f "/sys/devices/platform/coretemp.0/temp${i}_input" ]; then
            _check_filepath "/sys/devices/platform/coretemp.0/temp${i}_input"
            _check_filepath "/sys/devices/platform/coretemp.0/temp${i}_max"
            _check_filepath "/sys/devices/platform/coretemp.0/temp${i}_crit"
            temp_input=$(eval "cat /sys/devices/platform/coretemp.0/temp${i}_input ${LOG_REDIRECT}")
            temp_max=$(eval "cat /sys/devices/platform/coretemp.0/temp${i}_max ${LOG_REDIRECT}")
            temp_crit=$(eval "cat /sys/devices/platform/coretemp.0/temp${i}_crit ${LOG_REDIRECT}")
        else
            _echo "sysfs of CPU core temperature not found!!!"
        fi

        _echo "[CPU Core Temp${i} Input   ]: ${temp_input}"
        _echo "[CPU Core Temp${i} Max     ]: ${temp_max}"
        _echo "[CPU Core Temp${i} Crit    ]: ${temp_crit}"
        _echo ""
    done

    if [ -f "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/temp1_input" ]; then
        _check_filepath "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/temp1_input"
        _check_filepath "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/temp1_max"
        _check_filepath "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/temp1_max_hyst"
        temp_input=$(eval "cat /sys/bus/i2c/devices/0-004f/hwmon/hwmon1/temp1_input ${LOG_REDIRECT}")
        temp_max=$(eval "cat /sys/bus/i2c/devices/0-004f/hwmon/hwmon1/temp1_max ${LOG_REDIRECT}")
        temp_max_hyst=$(eval "cat /sys/bus/i2c/devices/0-004f/hwmon/hwmon1/temp1_max_hyst ${LOG_REDIRECT}")
    elif [ -f "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/device/temp1_input" ]; then
        _check_filepath "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/device/temp1_input"
        _check_filepath "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/device/temp1_max"
        _check_filepath "/sys/bus/i2c/devices/0-004f/hwmon/hwmon1/device/temp1_max_hyst"
        temp_input=$(eval "cat /sys/bus/i2c/devices/0-004f/hwmon/hwmon1/device/temp1_input ${LOG_REDIRECT}")
        temp_max=$(eval "cat /sys/bus/i2c/devices/0-004f/hwmon/hwmon1/device/temp1_max ${LOG_REDIRECT}")
        temp_max_hyst=$(eval "cat /sys/bus/i2c/devices/0-004f/hwmon/hwmon1/device/temp1_max_hyst ${LOG_REDIRECT}")
    else
        _echo "sysfs of CPU board temperature not found!!!"
    fi

    _echo "[CPU Board Temp Input   ]: ${temp_input}"
    _echo "[CPU Board Temp Max     ]: ${temp_max}"
    _echo "[CPU Board Temp Max Hyst]: ${temp_max_hyst}"
}

function _show_cpu_temperature {
    if [ "${BSP_INIT_FLAG}" == "1" ]; then
        _show_cpu_temperature_sysfs
    fi
}

function _show_cpld_cpudebug_register {
    _banner "Show CPLD all and some CPU debug register"

    if [ "${MODEL_NAME}" == "S9500-22XST" ]; then
        mb_start_cpld_reg=0x700
        mb_end_cpld_reg=0x71c
        
        # MB CPLD register value
        _echo "[MB CPLD]:"
        for (( i=${mb_start_cpld_reg}; i<=${mb_end_cpld_reg}; i++ ))
        do
            addr=$(eval "printf '%x' ${i}")
            ret=$(eval "${IOGET} ${addr} ${LOG_REDIRECT}")
            _echo "${ret}"
        done
        
        cpu_start_cpld_reg=0x600
        cpu_end_cpld_reg=0x606
        
        # CPU CPLD register value
        _echo "[CPU CPLD]:"
        for (( i=${cpu_start_cpld_reg}; i<=${cpu_end_cpld_reg}; i++ ))
        do
            addr=$(eval "printf '%x' ${i}")
            ret=$(eval "${IOGET} ${addr} ${LOG_REDIRECT}")
            _echo "${ret}"
        done
        
        _run_echo "${IOGET} 0x501 ${LOG_REDIRECT}"
        _run_echo "${IOGET} 0xf000 ${LOG_REDIRECT}"
        _run_echo "${IOGET} 0xf011 ${LOG_REDIRECT}"
    else
        _echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi

}

function _show_cpld_error_log {
    _banner "Show CPLD Error Log"
    
    _echo "Register: 0xB7 0xB6 0xB5 0xB4"
    _echo "============================="
    for ((i=0;i<256;i++))
    do
        i2cset -y 0 0x66 0xbf 0x00
        i2cset -y 0 0x66 0xb9 0x00
        i2cset -y 0 0x66 0xb8 ${i}
        i2cset -y 0 0x66 0xba 0x01
        i2cset -y 0 0x66 0xbb 0x01

        reg_0xb7=`i2cget -y 0 0x66 0xb7`
        reg_0xb6=`i2cget -y 0 0x66 0xb6`
        reg_0xb5=`i2cget -y 0 0x66 0xb5`
        ret_0xb4=`i2cget -y 0 0x66 0xb4`

        _echo "$(printf "Addr_%03d: %s %s %s %s\n" $i ${reg_0xb7} ${reg_0xb6} ${reg_0xb5} ${ret_0xb4})"
    done
}

# Note: In order to prevent affecting MCE mechanism, 
#       the function will not clear the 0x425 and 0x429 registers at step 1.1/1.2,
#       and only use to show the current correctable error count.
function _show_memory_correctable_error_count {
    _banner "Show Memory Correctable Error Count"

    which rdmsr > /dev/null 2>&1
    ret_rdmsr=$?
    which wrmsr > /dev/null 2>&1
    ret_wrmsr=$?

    if [ ${ret_rdmsr} -eq 0 ] && [ ${ret_wrmsr} -eq 0 ]; then
        ERROR_COUNT_THREASHOLD=12438
        modprobe msr

        # Step 0.1: Before clear the register, dump the correctable error count in channel 0 bank 9
        reg_c0_str=`rdmsr -p0 0x425`
        reg_c0_value=`printf "%u\n" 0x${reg_c0_str}`
        # CORRECTED_ERR_COUNT bit[52:38]
        error_count_c0=$(((reg_c0_value >> 38) & 0x7FFF))
        _echo "[Ori_C0_Error_Count]: ${error_count_c0}"

        # Step 0.2: Before clear the register, dump the correctable error count in channel 1 bank 10
        reg_c1_str=`rdmsr -p0 0x429`
        reg_c1_value=`printf "%u\n" 0x${reg_c1_str}`
        # CORRECTED_ERR_COUNT bit[52:38]
        error_count_c1=$(((reg_c1_value >> 38) & 0x7FFF))
        _echo "[Ori_C1_Error_Count]: ${error_count_c1}"

        # Step 1.1: clear correctable error count in channel 0 bank 9
        #wrmsr -p0 0x425 0x0

        # Step 1.2: clear correctable error count in channel 1 bank 10
        #wrmsr -p0 0x429 0x0

        # Step 2: wait 2 seconds
        sleep 2

        # Step 3.1: Read correctable error count in channel 0 bank 9
        reg_c0_str=`rdmsr -p0 0x425`
        reg_c0_value=`printf "%u\n" 0x${reg_c0_str}`
        # CORRECTED_ERR_COUNT bit[52:38]
        error_count_c0=$(((reg_c0_value >> 38) & 0x7FFF))
        if [ ${error_count_c0} -gt ${ERROR_COUNT_THREASHOLD} ]; then
            _echo "[ERROR] Channel 0 Bank  9 Register Value: 0x${reg_c0_str}, Error Count: ${error_count_c0}"
        else
            _echo "[Info] Channel 0 Bank  9 Register Value: 0x${reg_c0_str}, Error Count: ${error_count_c0}"
        fi

        # Step 3.2: Read correctable error count in channel 1 bank 10
        reg_c1_str=`rdmsr -p0 0x429`
        reg_c1_value=`printf "%u\n" 0x${reg_c1_str}`
        # CORRECTED_ERR_COUNT bit[52:38]
        error_count_c1=$(((reg_c1_value >> 38) & 0x7FFF))
        if [ ${error_count_c1} -gt ${ERROR_COUNT_THREASHOLD} ]; then
            _echo "[ERROR] Channel 1 Bank 10 Register Value: 0x${reg_c1_str}, Error Count: ${error_count_c1}"
        else
            _echo "[Info] Channel 1 Bank 10 Register Value: 0x${reg_c1_str}, Error Count: ${error_count_c1}"
        fi
    else
        _echo "Not support! Please install msr-tools to enble this function."
    fi
}

function _show_usb_info {
    _banner "Show USB Info"

    _echo "[Command]: lsusb -v"
    ret=$(eval "lsusb -v ${LOG_REDIRECT}")
    _echo "${ret}"
    _echo ""
    
    _echo "[Command]: lsusb -t"
    ret=$(eval "lsusb -t ${LOG_REDIRECT}")
    _echo "${ret}"
    _echo ""

    # check usb auth
    _echo "[USB Port Authentication]: "
    usb_auth_file_array=("/sys/bus/usb/devices/usb2/authorized" \
                         "/sys/bus/usb/devices/usb2/authorized_default" \
                         "/sys/bus/usb/devices/usb2/2-4/authorized" \
                         "/sys/bus/usb/devices/usb2/2-4/2-4.1/authorized" \
                         "/sys/bus/usb/devices/usb2/2-4/2-4:1.0/authorized" )

    for (( i=0; i<${#usb_auth_file_array[@]}; i++ ))
    do
        _check_filepath "${usb_auth_file_array[$i]}"
        if [ -f "${usb_auth_file_array[$i]}" ]; then
            ret=$(eval "cat ${usb_auth_file_array[$i]} ${LOG_REDIRECT}")
            _echo "${usb_auth_file_array[$i]}: $ret"
        else
            _echo "${usb_auth_file_array[$i]}: -1"
        fi
    done
}

function _show_scsi_device_info {
    _banner "Show SCSI Device Info"

    scsi_device_info=$(eval "cat /proc/scsi/sg/device_strs ${LOG_REDIRECT}")
    _echo "[SCSI Device Info]: "
    _echo "${scsi_device_info}"
    _echo ""
}

function _show_onie_upgrade_info {
    _banner "Show ONIE Upgrade Info"

    if [ -d "/sys/firmware/efi" ]; then
        if [ ! -d "/mnt/onie-boot/" ]; then
            mkdir /mnt/onie-boot
        fi

        mount LABEL=ONIE-BOOT /mnt/onie-boot/ 
        onie_show_version=`/mnt/onie-boot/onie/tools/bin/onie-version`
        onie_show_pending=`/mnt/onie-boot/onie/tools/bin/onie-fwpkg show-pending`
        onie_show_result=`/mnt/onie-boot/onie/tools/bin/onie-fwpkg show-results`
        onie_show_log=`/mnt/onie-boot/onie/tools/bin/onie-fwpkg show-log`
        umount /mnt/onie-boot/

        _echo "[ONIE Show Version]:"
        _echo "${onie_show_version}"
        _echo ""
    else
        _echo "BIOS is in Legacy Mode!!!!!"
    fi
}

function _show_disk_info {
    _banner "Show Disk Info"
   
    cmd_array=("lsblk" "parted -l /dev/sda" "fdisk -l /dev/sda" "cat /sys/fs/*/*/errors_count")
    
    for (( i=0; i<${#cmd_array[@]}; i++ ))
    do
        _echo "[Command]: ${cmd_array[$i]}"
        ret=$(eval "${cmd_array[$i]} ${LOG_REDIRECT}")
        _echo "${ret}"
        _echo ""
    done

    # check smartctl command
    cmd="smartctl -a /dev/sda"
    ret=`which smartctl`
    if [ ! $? -eq 0 ]; then
        _echo "[command]: ($cmd) not found (SKIP)!!"
    else
        ret=$(eval "$cmd ${LOG_REDIRECT}")
        _echo "[command]: $cmd"
        _echo "${ret}"
    fi

}

function _show_lspci {
    _banner "Show lspci Info"
    
    ret=`lspci`
    _echo "${ret}"
    _echo ""

    _echo "[PCI Bridge Hotplug Status]: "
    pci_device_id=($(lspci | grep "PLX Technology" | awk '{print $1}'))
    for i in "${pci_device_id[@]}"
    do  
        ret=`lspci -vvv -s ${i} | grep HotPlug`
        _echo "${i} ${ret}"
    done
}

function _show_lspci_detail {
    _banner "Show lspci Detail Info"
    
    ret=$(eval "lspci -xxxx -vvv ${LOG_REDIRECT}")
    _echo "${ret}"
}

function _show_proc_interrupt {
    _banner "Show Proc Interrupts"

    for i in {1..5};
    do
        ret=$(eval "cat /proc/interrupts ${LOG_REDIRECT}")
        _echo "[Proc Interrupts ($i)]:"
        _echo "${ret}"
        _echo ""
        sleep 1
    done
}

function _show_ipmi_info {
    _banner "Show IPMI Info"

    ipmi_folder="/proc/ipmi/0/"

    if [ -d "${ipmi_folder}" ]; then
        ipmi_file_array=($(ls ${ipmi_folder}))
        for (( i=0; i<${#ipmi_file_array[@]}; i++ ))           
        do
            _echo "[Command]: cat ${ipmi_folder}/${ipmi_file_array[$i]} "
            ret=$(eval "cat "${ipmi_folder}/${ipmi_file_array[$i]}" ${LOG_REDIRECT}")
            _echo "${ret}"
            _echo ""
        done
    else
        _echo "Warning, folder not found (${ipmi_folder})!!!"
    fi

    _echo "[Command]: lsmod | grep ipmi "
    ret=`lsmod | grep ipmi`
    _echo "${ret}"
}

function _show_bios_info {
    _banner "Show BIOS Info"

    cmd_array=("dmidecode -t 0" \
               "dmidecode -t 1" \
               "dmidecode -t 2" \
               "dmidecode -t 3")
    
    for (( i=0; i<${#cmd_array[@]}; i++ ))
    do
        _echo "[Command]: ${cmd_array[$i]} "
        ret=$(eval "${cmd_array[$i]} ${LOG_REDIRECT}")
        _echo "${ret}"
        _echo ""
    done
}

function _show_bmc_info {
    _banner "Show BMC Info"

    cmd_array=("ipmitool mc info" "ipmitool lan print" "ipmitool sel info" \
               "ipmitool fru -v" "ipmitool power status" \
               "ipmitool channel info 0xf" "ipmitool channel info 0x1" \
               "ipmitool sol info 0x1" "ipmitool i2c bus=2 0x80 0x1 0x45" \
               "ipmitool mc watchdog get" "ipmitool mc info -I usb")
    
    for (( i=0; i<${#cmd_array[@]}; i++ ))
    do
        _echo "[Command]: ${cmd_array[$i]} "
        ret=$(eval "${cmd_array[$i]} ${LOG_REDIRECT}")
        _echo "${ret}"
        _echo ""
    done
    
}

function _show_bmc_device_status {
    _banner "Show BMC Device Status"

    # Step1: Stop IPMI Polling
    ret=$(eval "ipmitool raw 0x3c 0x07 0x1 ${LOG_REDIRECT}")
    _echo "[Stop IPMI Polling ]: ${ret}"
    _echo ""
    sleep 3

    # UCD Device
    # Step1: Stop IPMI Polling
    # Step2: Switch I2C MUX to UCD related channel
    # Step3: Check status register of UCD
    # Step4: Re-start IPMI polling
    _echo "[UCD Device Status (BMC) ]"
    if [ "${MODEL_NAME}" == "S9500-22XST" ]; then 
        ## Step1: Stop IPMI Polling
        #ret=$(eval "ipmitool raw 0x3c 0x4 0x9 0x1 0x0 ${LOG_REDIRECT}")
        #_echo "[Stop IPMI Polling ]: ${ret}"
        #sleep 3

        # Step2: Switch I2C MUX to UCD related channel
        # Nothing to do

        # Step3: Check Status Register of UCD
        status_word=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x2 0x79 ${LOG_REDIRECT}" | head -n 1)
        logged_faults=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0xf 0xea ${LOG_REDIRECT}")
        mfr_status=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x5 0xf3 ${LOG_REDIRECT}")
        status_vout=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x1 0x7a ${LOG_REDIRECT}" | head -n 1)
        status_iout=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x1 0x7b ${LOG_REDIRECT}" | head -n 1)
        status_temperature=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x1 0x7d ${LOG_REDIRECT}" | head -n 1)
        status_cml=$(eval "ipmitool raw 0x3c 0x12 0x0 0x34 0x1 0x7e ${LOG_REDIRECT}" | head -n 1)
        _echo "[UCD Status Word   ]: ${status_word}"
        _echo "[UCD Logged Faults ]: ${logged_faults}"
        _echo "[UCD MFR Status    ]: ${mfr_status}"
        _echo "[UCD Status VOUT   ]: ${status_vout}"
        _echo "[UCD Status IOUT   ]: ${status_iout}"
        _echo "[UCD Status Tempe  ]: ${status_temperature}"
        _echo "[UCD Status CML    ]: ${status_cml}"
        _echo ""

        ## Step4: Re-start IPMI polling
        #ret=$(eval "ipmitool raw 0x3c 0x4 0x9 0x1 0x1 ${LOG_REDIRECT}")
        #_echo "[Start IPMI Polling]: ${ret}"
        #_echo ""
    else
        _echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi

    # PSU Device
    # Step1: Stop IPMI Polling
    # Step2: Switch I2C MUX to PSU0 Channel and Check Status Registers
    # Step3: Switch I2C MUX to PSU1 Channel and Check Status Registers
    # Step4: Re-start IPMI polling
    _echo "[PSU Device Status (BMC) ]"
    if [ "${MODEL_NAME}" == "S9500-22XST" ]; then 
        ## Step1: Stop IPMI Polling
        #ret=$(eval "ipmitool raw 0x3c 0x4 0x9 0x1 0x0 ${LOG_REDIRECT}")
        #_echo "[Stop IPMI Polling ]: ${ret}"
        #sleep 3

        # Step2: Switch I2C MUX to PSU0 Channel and Check Status Registers
        ret=$(eval "ipmitool raw 0x3c 0x12 0x2 0x33 0x1 0x6 0xe8 ${LOG_REDIRECT}")
        ret=$(eval "i2cset -y -f 0 0x70 0x1 ${LOG_REDIRECT}")
        status_word_psu0=$(eval "i2cget -y -f 0 0x58 0x79 w ${LOG_REDIRECT}" | head -n 1)
        status_vout_psu0=$(eval "i2cget -y -f 0 0x58 0x7a ${LOG_REDIRECT}" | head -n 1)
        status_iout_psu0=$(eval "i2cget -y -f 0 0x58 0x7b ${LOG_REDIRECT}" | head -n 1)
        status_temperature_psu0=$(eval "i2cget -y -f 0 0x58 0x7d ${LOG_REDIRECT}" | head -n 1)
        status_fan_psu0=$(eval "i2cget -y -f 0 0x58 0x81 ${LOG_REDIRECT}" | head -n 1)
        _echo "[PSU0 Status Word  ]: ${status_word_psu0}"
        _echo "[PSU0 Status VOUT  ]: ${status_vout_psu0}"
        _echo "[PSU0 Status IOUT  ]: ${status_iout_psu0}"
        _echo "[PSU0 Status Tempe ]: ${status_temperature_psu0}"
        _echo "[PSU0 Status FAN   ]: ${status_fan_psu0}"

        # Step3: Switch I2C MUX to PSU1 Channel and Check Status Registers
        ret=$(eval "i2cset -y -f 0 0x70 0x2 ${LOG_REDIRECT}")
        status_word_psu1=$(eval "ipmitool i2c bus=2 0xb0 0x2 0x79 ${LOG_REDIRECT}" | head -n 1)
        status_vout_psu1=$(eval "ipmitool i2c bus=2 0xb0 0x1 0x7a ${LOG_REDIRECT}" | head -n 1)
        status_iout_psu1=$(eval "ipmitool i2c bus=2 0xb0 0x1 0x7b ${LOG_REDIRECT}" | head -n 1)
        status_temperature_psu1=$(eval "ipmitool i2c bus=2 0xb0 0x1 0x7d ${LOG_REDIRECT}" | head -n 1)
        status_fan_psu1=$(eval "ipmitool i2c bus=2 0xb0 0x1 0x81 ${LOG_REDIRECT}" | head -n 1)
        _echo "[PSU1 Status Word  ]: ${status_word_psu1}"
        _echo "[PSU1 Status VOUT  ]: ${status_vout_psu1}"
        _echo "[PSU1 Status IOUT  ]: ${status_iout_psu1}"
        _echo "[PSU1 Status Tempe ]: ${status_temperature_psu1}"
        _echo "[PSU1 Status FAN   ]: ${status_fan_psu1}"
        _echo ""

        ## Step4: Re-start IPMI polling
        #ret=$(eval "ipmitool raw 0x3c 0x4 0x9 0x1 0x1 ${LOG_REDIRECT}")
        #_echo "[Start IPMI Polling]: ${ret}"
        #_echo ""
        
        ret=$(eval "ipmitool raw 0x3c 0x12 0x2 0x33 0x1 0x6 0x40 ${LOG_REDIRECT}")
    else
        _echo "Unknown MODEL_NAME (${MODEL_NAME}), exit!!!"
        exit 1
    fi
    
    # Step4: Re-start IPMI polling
    ret=$(eval "ipmitool raw 0x3c 0x07 0x0 ${LOG_REDIRECT}")
    _echo "[Start IPMI Polling]: ${ret}"
    _echo ""

    sleep 3
}

function _show_bmc_sensors {
    _banner "Show BMC Sensors"

    ret=$(eval "ipmitool sensor ${LOG_REDIRECT}")
    _echo "[Sensors]:"
    _echo "${ret}"
}

function _show_bmc_sel_raw_data {
    _banner "Show BMC SEL Raw Data"
    echo "    Show BMC SEL Raw Data, please wait..."
    
    if [ "${LOG_FILE_ENABLE}" == "1" ]; then
        _echo "[SEL RAW Data]:"
        ret=$(eval "ipmitool sel save ${LOG_FOLDER_PATH}/sel_raw_data.log ${LOG_REDIRECT}")
        _echo "The file is located at ${LOG_FOLDER_NAME}/sel_raw_data.log"
    fi
}

function _show_bmc_sel_elist {
    _banner "Show BMC SEL"
    
    ret=$(eval "ipmitool sel elist ${LOG_REDIRECT}")
    _echo "[SEL Record]:"
    _echo "${ret}"
}

function _show_bmc_sel_elist_detail {
    _banner "Show BMC SEL Detail -- Abnormal Event"

    echo "    Show BMC SEL details, please wait..."
    sel_id_list=""

    readarray sel_array < <(ipmitool sel elist 2> /dev/null)

    for (( i=0; i<${#sel_array[@]}; i++ ))
    do
        if [[ "${sel_array[$i]}" == *"Undetermined"* ]] ||
           [[ "${sel_array[$i]}" == *"Bus"* ]] ||
           [[ "${sel_array[$i]}" == *"CATERR"* ]] ||
           [[ "${sel_array[$i]}" == *"OEM"* ]] ; then
            _echo  "${sel_array[$i]}"
            sel_id=($(echo "${sel_array[$i]}" | awk -F" " '{print $1}'))
            sel_id_list="${sel_id_list} 0x${sel_id}"
        fi
    done

    if [ ! -z "${sel_id_list}" ]; then
        sel_detail=$(eval "ipmitool sel get ${sel_id_list} ${LOG_REDIRECT}")
    else
        sel_detail=""
    fi

    _echo "[SEL Record ID]: ${sel_id_list}"
    _echo ""
    _echo "[SEL Detail   ]:"
    _echo "${sel_detail}"


    #sel_rid_array=`ipmitool sel elist | grep -P -i '(Undetermined|Bus|CATERR|OEM)' | awk '{printf("0x%s ",$1)}'`
    #sel_detail=`ipmitool sel get ${sel_rid_array}`

    #_echo "[SEL Record ID]: ${sel_rid_array}"
    #_echo ""
    #_echo "[SEL Detail   ]:"
    #_echo "${sel_detail}"

    #sel_elist_number=10
    #bmc_sel_id_array=($(ipmitool sel list | awk -F" " '{print $1}' | tail -n ${sel_elist_number}))
    #
    #for (( i=0; i<${#bmc_sel_id_array[@]}; i++ ))
    #do
    #    _echo ""
    #    _echo "[Command]: ipmitool sel get 0x${bmc_sel_id_array[${i}]} "
    #    ret=`ipmitool sel get 0x${bmc_sel_id_array[${i}]}`
    #    _echo "${ret}"
    #done
}

function _show_dpll_register {
    _banner "Show DPLL all register"
    
    if [ "${BSP_INIT_FLAG}" == "1" ]; then
        dpll_bus_num=4
    else
        i2cset -y 0 0x75 8
        if [ "$?" != "0" ]; then
            _echo "0x75 does not exist, Exit!!"
            exit 255;
        fi
        
        dpll_bus_num=0
    fi
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 0"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 0
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 1"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 1
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 2"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 2
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 3"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 3
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 4"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 4
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 5"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 5
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 6"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 6
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    _echo "i2cset -y ${dpll_bus_num} 0x53 0x7f 7"
    i2cset -y ${dpll_bus_num} 0x53 0x7f 7
    _run_echo "i2cdump -y ${dpll_bus_num} 0x53 b"
    
    if [ "${BSP_INIT_FLAG}" != "1" ]; then
        i2cset -y 0 0x75 0
    fi
}

function _show_dmesg {
    _banner "Show Dmesg"
    
    ret=$(eval "dmesg ${LOG_REDIRECT}")
    _echo "${ret}"
}

function _additional_log_collection {
    _banner "Additional Log Collection"

    if [ -z "${LOG_FOLDER_PATH}" ] || [ ! -d "${LOG_FOLDER_PATH}" ]; then
        _echo "LOG_FOLDER_PATH (${LOG_FOLDER_PATH}) not found!!!"
        _echo "do nothing..."
    else
        #_echo "copy /var/log/syslog* to ${LOG_FOLDER_PATH}"
        #cp /var/log/syslog*  "${LOG_FOLDER_PATH}"

        if [ -f "/var/log/kern.log" ]; then
            _echo "copy /var/log/kern.log* to ${LOG_FOLDER_PATH}"
            cp /var/log/kern.log*  "${LOG_FOLDER_PATH}"
        fi
        
        if [ -f "/var/log/dmesg" ]; then
            _echo "copy /var/log/dmesg* to ${LOG_FOLDER_PATH}"
            cp /var/log/dmesg*  "${LOG_FOLDER_PATH}"
        fi
    fi
}

function _compression {
    _banner "Compression"
    
    if [ ! -z "${LOG_FOLDER_PATH}" ] && [ -d "${LOG_FOLDER_PATH}" ]; then
        cd "${LOG_FOLDER_ROOT}"
        tar -zcf "${LOG_FOLDER_NAME}".tgz "${LOG_FOLDER_NAME}"

        echo "    The tarball is ready at ${LOG_FOLDER_ROOT}/${LOG_FOLDER_NAME}.tgz"
        _echo "    The tarball is ready at ${LOG_FOLDER_ROOT}/${LOG_FOLDER_NAME}.tgz"
    fi
}

function _main {
    echo "The script will take a few minutes, please wait..."
    _check_env
    _pkg_version
    _show_board_info
    _show_version
    _show_i2c_tree
    _show_i2c_device_info
    _show_sys_devices
    _show_cpu_eeprom
    _show_psu_status
    _show_rov
    _show_port_status
    _show_cpu_temperature
    _show_cpld_cpudebug_register
    _show_system_info
    _show_memory_correctable_error_count
    _show_usb_info
    _show_scsi_device_info
    _show_onie_upgrade_info
    _show_disk_info
    _show_lspci
    _show_lspci_detail
    _show_proc_interrupt
    _show_bios_info
    _show_bmc_info
    _show_bmc_device_status
    _show_bmc_sensors
    _show_bmc_sel_raw_data
    _show_bmc_sel_elist
    _show_bmc_sel_elist_detail
    _show_dpll_register
    _show_dmesg
    _additional_log_collection
    _compression

    echo "#   done..."
}

_main

