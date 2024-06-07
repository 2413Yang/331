
#ifndef IDDEFINE__H__
#define IDDEFINE__H__

#define A301

/////////////////////////////////////////////////////
////              MCU to ARM
/////////////////////////////////////////////////////
//major ID
#define UP_MAJOR_KEY 0xD50000         // 按键
#define UP_MAJOR_UNIVERSAL 0x500000   // MCU通用上发协议
#define UP_MAJOR_SET_CONFIG 0X820000  // 设置相关配置信息同步
#define UP_MAJOR_POWER_MODE 0XD70000  // 电源模式
#define UP_MAJOR_RTC_TIME 0XAB0000    // RTC时钟同步
#define UP_MAJOR_MCU_VERSION 0X880000 // MCU版本信息
#define UP_MAJOR_SCREEN_MODE 0XD80000 // 屏幕信号
#define UP_MAJOR_DATA_SYNC_COMPLETE 0XD90000 // 数据同步完成

//minor ID
#define UP_MINOR_TRIP_ODO_MILEAGE 0x0600     // 里程信息
#define UP_MINOR_VEHICLE_LIGHT_STATUS 0x0601 // 灯光状态
#define UP_MINOR_TURN_LIGHT_STATUS 0x0602    // 转向灯跟双闪
#define UP_MINOR_DOOR_STATUS 0x0603          // 车门状态
#define UP_MINOR_WINDOW_STATUS 0x0604        // 车窗状态
#define UP_MINOR_VEHICLE_SPEED 0x0605        // 车速、平均车速、行驶时间
#define UP_MINOR_ENGINE_SPEED 0x0606         // 发动机转速
#define UP_MINOR_ENERGY_CONSUMPTION 0x0607   // 续航里程、瞬时能耗、平均能耗
#define UP_MINOR_ELECTRIC_INFOR 0x0608       // 动力电池信息(电量/电压/电流)
#define UP_MINOR_POWER_GEAR_STATUS 0x0609    // 变速箱档位, Ready指示灯
#define UP_MINOR_CUR_TIME 0x060A             // 当前时间
#define UP_MINOR_TEMP 0x060B                 // 车内、车外、左右温区温度
#define UP_MINOR_SEATBELT_AIRBAG 0x060C      // 安全带跟安全气囊
#define UP_MINOR_RADAR_INFOR 0x060D          // 雷达信息
#define UP_MINOR_TIRE_STATUS 0x060E          // 车胎信息
#define UP_MINOR_FAULT_CODE 0x060F           // 故障码信息

#define UP_MINOR_ENERGY_VALUE 0x0612         // 能量值
#define UP_MINOR_WATER_TEMP_AND_FUEL 0x0613  // 水温和燃油
#define UP_MINOR_FUEL_CONSUMPTION 0x0614     // 瞬时油耗和平均油耗
#define UP_MINOR_FUEL 0x0620                 // 燃油
#define UP_MINOR_POPUPS_WARNING 0x0615       // 弹窗报警
#define UP_MINOR_CHARGE_ORDER 0x616          // 预约充电信息提示
#define UP_MINOR_CHARGE_STATUS 0x617         // 充电/放电状态界面显示
#define UP_MINOR_VOICE_WARNING 0x618         // 蜂鸣器报警音
#define UP_MINOR_SELF_CHECK_ACK 0x6E0        // 自检应答
#define UP_MINOR_MEMORY_INFO 0x6E2             //记忆信息
#define UP_MINOR_RESET_ACK 0x6E3             // 清除应答
#define UP_MINOR_MEMORY_INFO_ACK 0x00E2      //设置记忆信息的应答

#define UP_MINOR_IVI_INTERACTIVE 0X0369     //仪表与中控的交互 中控->仪表
#define UP_MINOR_IVI_OTA    0X00A1          //仪表与中控的升级交互 中控->仪表
#define UP_MINOR_IVI_TIME 0X00A2            //仪表与中控的同步时间交互 中控->仪表


/////////////////////////////////////////////////////
////              ARM to MCU
/////////////////////////////////////////////////////
//major ID
#define DOWN_MAJOR_INIT 0x800000             // app正在启动
#define DOWN_MAJOR_SYNC_DATA 0x810000        // 请求同步状态
#define DOWN_MAJOR_BRIGHTNESS_CTRL 0x9A0B01  // 背光开关控制
#define DOWN_MAJOR_UNIVERSAL 0xC00000        // MCU下发通用协议主ID
#define DOWN_MAJOR_SET 0x8B0B01              // 设置项主ID
#define DOWN_MAJOR_HEARTBEAT 0x9E0000        // 心跳包
#define DOWN_MAJOR_BRIGHTNESS_LEVEL 0xF00B01 // 背光等级设置项
#define DOWN_MAJOR_RTC_SET 0xA80000          // RTC时间设置


//minor ID
#define DOWN_MINOR_ACK 0x00EF          // 应答MCU
#define DOWN_MINOR_SELF_CHECK 0x00E0   // 自检信号
#define DOWN_MINOR_MEMORY_INFO 0x00E2 // 记忆项
#define DOWN_MINOR_DATA_RESET 0x00E3   // 数据清零
#define DOWN_MINOR_IGN_OFF_POPSID 0x00E4   // 熄火弹窗ID下发
#define DOWN_MINOR_IVI_CTRL_BRIGHTNESS 0x00E5   // IVI背光控制下发
#define DOWN_MINOR_IGN_STATUS_RESPONSE 0x00E6   // ign状态应答下发
#define DOWN_MINOR_IVI_INTERACTIVE 0X0396  //仪表与中控的交互 仪表->中控 (主ID是0xC0)
#define DOWN_MINOR_IVI_OTA 0X00A0  //仪表与中控的升级交互 仪表->中控 (主ID是0xC0)
#define DOWN_MINOR_IVI_SOUND 0X00A3  //仪表与中控的报警音交互 仪表->中控 (主ID是0xC0)
#define DOWN_MINOR_OIL_LIFE_RESET	0x01EC	//机油寿命
#define DOWN_MINOR_MEMORY_INFO_ACK 0x06E2 // 恢复记忆项的应答


// UP DOWN 通用ID
#define MAJOR_ENTER_UPGRADE_MODE 0x9C0B01  // 升级相关协议ID
#define MAJOR_ENTER_UPGRADE_MODE2 0x9C0000 // 升级相关协议ID

//Setting item number
#define SET_IC_BRIGHTNESS_LEVEL 0x79 // 仪表亮度等级设置
#define SET_IC_MILE_FORMAT 0x95      // 仪表单位制式设置
#define SET_IC_LANGUAGE_MODE 0x96    // 仪表语言模式设置
#define SET_IC_DAYLIGHT_MODE 0x97    // 仪表白天黑夜模式设置
#define SET_IC_THEME_MODE 0x94       // 仪表主题模式设置
#define SET_IC_BLUETOOTH_STATUS 0x98 // 仪表蓝牙状态设置
#define SET_IC_MILEAGE_MODE 0x99     // 仪表里程模式设置
#define SET_IC_PRESSUNIT_MODE 0x9A     // 仪表压力单位设置； 0-kpa；1-bar;2-psi
#define SET_IC_MENU_ITEM_CFG_LOW 0x9B     // 仪表菜单项配置低字节
#define SET_IC_MENU_ITEM_CFG_HIGH 0x9C     // 仪表菜单项配置高字节
#define SET_IC_SPEED_WARNValue 0x9D     // 仪表超速报警设置

#define SET_IC_TIME_HOUR 0x04        // 仪表时钟 - 时
#define SET_IC_TIME_MINUTE 0x05      // 仪表时钟 - 分
#define SET_IC_TIME_SECOND 0x06      // 仪表时钟 - 秒
#define SET_IC_DAYLIGHTAUTO 0x92      //MCU->ARM:在仪表设置为背景自动模式下，MCU根据光感自动调节背景并通知ARM修改白天黑夜的背景显示
#define SET_IC_BRIGHTNESS_AUTO 0x7A    //ARM<->MCU:在仪表设置为背光自动模式下,ARM通知MCU背光是否处于自动控制，0-自动调节，1-手动调节；

/**********************************************************************************
********************************** IVI ********************************************
**********************************************************************************/
#define IVI_TO_IC_SHAKE_HANDS 0x8000              //握手（IVI-> IC）
#define IC_TO_IVI_SHAKE_HANDS 0x8001              //握手回应（IC-> IVI）
#define IVI_TO_IC_SYNC_DATA 0x8100                //请求同步数据（IVI-> IC）
#define IC_TO_IVI_SYNC_TIME 0x8101                //同步时间（IC-> IVI）
#define IC_TO_IVI_SYNC_VERSION 0x8102             //同步版本信息（IC-> IVI）
#define IVI_TO_IC_PUSH_SCREEN_START 0x8200        //开启推屏（IVI -> IC）
#define IVI_TO_IC_PUSH_SCREEN_END 0x8201          //结束推屏（IVI -> IC）
#define IVI_TO_IC_SET_SOURCE 0x8202               //设置显示源（IVI -> IC）
#define IVI_TO_IC_GET_SOURCE 0x8203               //获取当前显示源（IVI -> IC）
#define IC_TO_IVI_NOTIFY_SOURCE 0x8204            //上报当前显示源（IC -> IVI）
#define IVI_TO_IC_GET_PUSH_SCREEN_STATE 0x8205    //获取推屏状态（IVI -> IC）
#define IC_TO_IVI_NOTIFY_PUSH_SCREEN_STATE 0x8206 //上报推屏状态（IC-> IVI）
#define IC_TO_IVI_REQUEST_PUSH_SCREEN 0x8207      //仪表请求推屏服务（IC-> IVI）
#define IC_TO_IVI_KEY 0x8300                      //发送按键（IC-> IVI）
#define IC_TO_IVI_CAN_DATA 0x8400                 //仪表 CAN 数据转发（IC-> IVI）
#define IVI_TO_IC_UPDATE_START 0x8500             //请求开始升级（IVI -> IC）
#define IC_TO_IVI_UPDATE_READY 0x8501             //仪表升级准备完毕（IC-> IVI）
#define IVI_TO_IC_MD5_CHECKVALUE 0x8502           //发送升级包 md5 值校验码 （IVI -> IC）
#define IVI_TO_IC_UPDATE_PACKET 0x8503            //开始发送升级数据（IVI -> IC）
#define IC_TO_IVI_UPDATE_PACKET_ACK 0x8504        //数据接收校验回应（IC-> IVI）
#define IVI_TO_IC_UPDATE_PACKET_FINISH 0x8505     //升级数据发送完成（IVI -> IC）
#define IVI_TO_IC_UPDATE_CANCEL 0x8506            //取消升级（IVI -> IC）
#define IVI_TO_IC_UPDATE_CANCEL_ACK 0x8507        //回应取消升级（IVI -> IC）
#define IC_TO_IVI_UPDATE_FILE_CHECK 0x8508        //完整数据包校验回应（IC-> IVI）
#define IC_TO_IVI_READY_TO_UPGRADE 0x8509         //仪表准备升级（IC-> IVI）
#define IVI_TO_IC_HEARTBEAT_START 0x8800          //心跳包开始（IVI-> IC）
#define IVI_TO_IC_HEARTBEAT_SIGNAL 0x8801         //心跳包信号（IVI-> IC）
#define IVI_TO_IC_HEARTBEAT_STOP 0x8802           //心跳包停止（IVI-> IC）
#define IC_TO_IVI_ANIMAT_STATE 0x8900             //开机logo播放状态
#define IVI_TO_IC_ANIMAT_CTRL 0x8901              //动画播放控制
#define IVI_TO_IC_REQ_LOG_TRANSMIT 0x8700         //请求日志传输
#define IC_TO_IVI_LOG_PREPARING 0x8701            //日志数据准备中
#define IC_TO_IVI_NOTIFY_LOG_FILE_NAME 0x8702     //日志数据准备完毕，发送文件名
#define IC_TO_IVI_NOTIFY_LOG_FILE_MD5 0x8703      //日志数据准备完毕，发送文件MD5值
#define IC_TO_IVI_NOTIFY_LOG_FILE_PAGE 0x8704     //日志数据包
#define IVI_TO_IC_LOG_FILE_PAGE_ANSWER 0x8705     //日志数据应答
#define IC_TO_IVI_LOG_TRANSMIT_COMPLET 0x8706     //日志传输结束
#define IVI_TO_IC_LOG_FILE_CHECK_ACK 0x8707       //日志校验结果应答
#define IVI_TO_IC_TOUCH_EVENT 0x8903              //中控屏触摸事件
#define IVI_TO_IC_OPEN_360AVM 0x8904              //中控通知打开360功能
#define IC_TO_IVI_360AVM_STATE 0x8905             //仪表通知中控360功能状态
#define IC_TO_IVI_REQUEST_CLOSE_360AVM 0x8906     //仪表请求中控关闭360功能
#define IVI_TO_IC_360AVM_CLOSED 0x8907            //中控已关闭360功能

#endif /*IDDEFINE__H__*/