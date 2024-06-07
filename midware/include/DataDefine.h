#ifndef DATADEFINE_H_
#define DATADEFINE_H_

#include <string>

#pragma pack(1)

enum class EmWarningState
{
    NORMAL,
    WARNING,
};

enum class EmSWState
{
    close,
    open,
    open_half,
};

enum class EmLampState
{
    EXTINGUISH = 0,  //灭
    BRIGHT = 1,      //亮
    TWINKLE_0_5HZ = 2, //0.5HZ闪烁
    TWINKLE_1HZ = 3, //1HZ闪烁
    TWINKLE_2HZ = 4, //2HZ闪烁
    TWINKLE_4HZ = 5, //4HZ闪烁
    NONE=0x0F
};

typedef struct LampState
{
    std::string strID;
    EmLampState state;
    LampState &operator=(const LampState &it)
    {
        this->strID = it.strID;
        this->state = it.state;
        return *this;
    }
} StuTelltableLampState;

typedef struct CallRecord
{
    int type;
    std::string name;
    int amount;
    CallRecord &operator=(const CallRecord &it)
    {
        this->type = it.type;
        this->name = it.name;
        this->amount = it.amount;
        return *this;
    }
} StuCallRecord;

enum class EmCarDoorID
{
    LeftFront,
    RightFront,
    LeftRear,
    RightRear,
    BackDoor,
    FrontDoor,
};

typedef struct
{
    EmCarDoorID ID;
    EmSWState state;
} StuCarDoorState;

enum class EmGearsValue
{
    NONE,
    G1,
    G2,
    G3,
    G4,
    G5,
    G6,
    G7,
    G8,
    G9,
    G10,
    P,
    R,
    N,
    D,
    S,
    M,
    R_4HZ,
    INVALID, //无效值，显示“--”
};

//按键数据
enum class EmKey
{
    RETURN = 0, //取消
    OK = 1,     //确定
    LEFT = 2,   //向左
    RIGHT = 3,  //向右
    UP = 4,     //向上
    DOWN = 5,   //向下
    END
};

enum class EmTheme
{
    THEME_FIRST = 0,
    THEME_SECOND = 1,
    THEME_THIRD = 2
};

enum class EmKeyState
{
    NONE = 0,       //无效
    S_PRESS = 1,    //短按
    S_ELEASE = 2,   //释放短按
    L_PRESS = 3,    //长按
    L_RELEASE = 4,  //释放长按
    L_PRESS_NR = 5, //长按不释放
};

enum class EmScreenType
{
    IV,   //0:中控屏
    IC,   //1:仪表屏
    IV_S, //2:后枕屏
    AC,   //3:空调面板
};

struct StuCarTire
{
    struct tire
    {
        int tireStatus;
        int validFlag;
        std::string presValue;
    };
    tire LFTire;
    tire RFTire;
    tire LRTire;
    tire RRTire;
};

struct StuChargeSubscribe
{
    int Year;
    int Month;
    int Day;
    int Hour;
    int Minute;
};

struct MotorTire
{
    struct tire
    {
        EmWarningState presWarn;
        EmWarningState tempWarn;
        std::string presValue;
        std::string tempValue;
    };
    tire FrontTire;
    tire RearTire;
};

enum class EmDriveMode
{
    COMFORTABLE,
    ENERGY_SAVING,
    MOVEMENT,
};

enum class EmBrightness
{
    LEVEL_ZERO = 0,
    LEVEL_ONE = 1,
    LEVEL_TWO = 2,
    LEVEL_THREE = 3,
    LEVEL_FOUR = 4,
    LEVEL_FIVE = 5,
    LEVEL_SIX = 6 //背光等级:自动
};

enum class EmLanguage
{
    CHINESE,
    ENGLISH,
    RUSSIAN,
};

enum class EmUnitSystem
{
    KILOMETRE, //公里
    MILE,      //英里
};

enum class EmUnitPress
{
    PRESS_KPA, //kpa
    PRESS_BAR,  //bar
    PRESS_PSI,  //psi
};

enum class EmViewMode
{
    DAY,   //白天
    NIGHT, //黑夜
};

enum class EmFunctionItem
{
    UNIT_SYSTEM,
    LANGUAGE_SYSTEM,
    THEME_MODE,
    BLUETOOTH_STATUS,
    BRIGHTNESS,
    VIEW_MODE,
    MILEAGE_MODE,
    UNIT_PRESS,
    MENU_ITEM_CFG_LOW,
    MENU_ITEM_CFG_HIGH,
    SPEED_WARNING_VALUE,
    TRIP_A,
    TRIP_B,
    OIL_LIFE,
};

enum EmBaseDataError
{
    EM_BASE_DATA_NONE,                //无效数据 (MCU发送的无效数据)
    EM_BASE_DATA_EFFECTIVE,           //有效
    EM_BASE_DATA_OUT_UP_RANGE_NONE,   //上越界无效
    EM_BASE_DATA_OUT_DOWN_RANGE_NONE, //下越界无效
    EM_BASE_DATA_EXAMINE_ERROR,       //数据错误
};

struct StuBaseVehicleData
{
    float m_fAccuracy;         //精度
    float m_fRangeHigh;        //最大值
    float m_fRangeLow;         //最小值
    int m_iValue;              //数据值(实际值= m_iValue * m_fAccuracy)
    char m_caUnit[32];         //单位
    EmBaseDataError m_emValid; //数据有效性
};

// 指示灯状态
enum EmTelltaleState
{
    EM_TELLTALE_OFF = 0, // 关闭
    EM_TELLTALE_ON,      // 打开
};

// 弹框显示状态
enum EmPopWinState
{
    EM_POP_WIN_OFF = 0, // 关闭
    EM_POP_WIN_ON = 1,  // 打开
};

enum class EmDisplaySource
{
    RADIO = 0x00,                        // 收音机
    LOCAL_MUSIC = 0x01,                  // 本地音乐
    ONLINE_MUSIC = 0x02,                 // 在线音乐
    BLUETOOTH_MUSIC = 0x03,              // 蓝牙音乐
    BLUETOOTH_PHONE = 0x04,              // 蓝牙电话
    NAVIGATION = 0x05,                   // 导航
    MOBILE_PHONE_INTERCONNECTION = 0x06, // 手机互联
    AUX_IPOD = 0x07,                     // Aux/ipod
    NONE,
};

// 声音播放状态
enum EmAlarmVoiceState
{
    EM_ALARM_VOICE_OFF = 0, // 关闭
    EM_ALARM_VOICE_ON = 1,  // 打开
};

struct StuAlarmItem
{
    int m_iID;
    int m_iState;
};

//主题风格模式
enum EmStyleThemeMode
{
    EM_STYLE_THEME_NONE = 0, //无效数据
    EM_STYLE_THEME_1 = 1,
    EM_STYLE_THEME_2 = 2,
    EM_STYLE_THEME_3 = 3,
    EM_STYLE_THEME_4 = 4
};

// 背光状态定义
enum EmBacklightLevel
{
    EM_BACKLIGHT_LEVEL_NONE,
    EM_BACKLIGHT_LEVEL_0,
    EM_BACKLIGHT_LEVEL_1,
    EM_BACKLIGHT_LEVEL_2,
    EM_BACKLIGHT_LEVEL_3,
    EM_BACKLIGHT_LEVEL_4,
    EM_BACKLIGHT_LEVEL_5,
    EM_BACKLIGHT_LEVEL_6,
    EM_BACKLIGHT_LEVEL_7,
    EM_BACKLIGHT_LEVEL_8,
    EM_BACKLIGHT_LEVEL_9,
    EM_BACKLIGHT_LEVEL_10
};

struct StuMenuSetup
{
    EmStyleThemeMode m_emStyleThemeMode;
    EmBacklightLevel m_emBacklightLevel;
};

struct StuDataResetInfo
{
    bool m_bResetTripA;     // true--清零TripA
    bool m_bAverageOilWear; // true--清零平均油耗
    bool m_bAverageSpeed;   // true--清零平均车速
};

enum class EmTouchEvent
{
    PEN_DOWN,
    PEN_UP,
    PEN_MOVE,
    NONE,
};

struct StuGears
{
    EmGearsValue m_emGearsValue;
};

enum EmTireAlarmStatus
{
    EM_TIRE_ALARM_NONE,
    EM_TIRE_ALARM_OFF,
    EM_TIRE_ALARM_ON,
};

struct StuTireItem
{
    EmTireAlarmStatus m_emLostSignal;
    EmTireAlarmStatus m_emAirLeak;
    EmTireAlarmStatus m_emSensorLowPower;
    EmTireAlarmStatus m_emHighPress;
    EmTireAlarmStatus m_emLowPress;
    EmTireAlarmStatus m_emHighTemp;
    EmTireAlarmStatus m_emStatus;
    StuBaseVehicleData m_stPressValue;
    StuBaseVehicleData m_stTempValue;
};

struct StuTire
{
    StuTireItem m_stTireLF;
    StuTireItem m_stTireRF;
    StuTireItem m_stTireLR;
    StuTireItem m_stTireRR;
};

// 电源状态定义
enum class EmPowerStatus
{
    EM_POWER_NONE,
    EM_POWER_OFF,
    EM_POWER_ON,
    EM_POWER_ABNORMAL,
    EM_POWER_ACC
};

enum class EmScreenStatus
{
    EM_SCREEN_OFF,
    EM_SCREEN_ON
};

enum EmBacklightStatus
{
    EM_BACKLIGHT_NONE,
    EM_BACKLIGHT_OFF,
    EM_BACKLIGHT_ON
};

//当前时间消息
struct StuTime
{
    short m_nYear;
    unsigned char m_ucMonth;
    unsigned char m_ucDay;
    unsigned char m_ucHour;
    unsigned char m_ucMinute;
    unsigned char m_ucSecond;
    unsigned char m_ucValidity; //有效性，为0时无效
};

struct StuDrivenTime
{
    unsigned char m_ucHour;
    unsigned char m_ucMinute;
};

enum EmDoorStatus
{
    EM_DOOR_NONE,
    EM_DOOR_CLOSE,
    EM_DOOR_OPEN,
};

struct StuDoorStatus
{
    EmDoorStatus m_emLeftFrontDoor;  //左前门
    EmDoorStatus m_emRightFrontDoor; //右前门
    EmDoorStatus m_emLeftRearDoor;   //左后门
    EmDoorStatus m_emRightRearDoor;  //右后门
    EmDoorStatus m_emTrunkDoor;      //后备箱
    EmDoorStatus m_emEngineHood;     //引擎盖
};

enum EmSeatbeltStatus
{
    EM_SEATBELT_NONE, //安全带状态无效
    EM_SEATBELT_OFF,  //安全带未系
    EM_SEATBELT_ON,   //安全带已系上
};

enum EmSeatStatus
{
    EM_SEAT_NONE,           //座位状态无效
    EM_SEAT_EMPTY,          //座位空
    EM_SEAT_HAVE_SOMETHING, //座位承重
};

struct StuSafety
{
    EmSeatbeltStatus m_emDriverSeatbeltStatus;
    EmSeatbeltStatus m_emCoDriverSeatbeltStatus;
    EmSeatStatus m_emDriverSeatStatus;
    EmSeatStatus m_emCoDriverSeatStatus;
};

enum EmIviStatus
{
    NOT_READY, //启动未完成
    READY,     //启动完成
};

// 雷达设备位置
enum RADAR_DEVICE {
    RADAR_DEVICE_REAR_LEFT,
    RADAR_DEVICE_REAR_LEFT_CENTER,
    RADAR_DEVICE_REAR_RIGHT_CENTER,
    RADAR_DEVICE_REAR_RIGHT,
    RADAR_DEVICE_FRONT_LEFT,
    RADAR_DEVICE_FRONT_LEFT_CENTER,
    RADAR_DEVICE_FRONT_RIGHT_CENTER,
    RADAR_DEVICE_FRONT_RIGHT,
};

// 雷达探测距离状态
enum RADAR_DET_DIST {
    RADAR_DET_DIST_NONE = 0, // 没有障碍物或无效状态
    RADAR_DET_DIST_VALID, // 有效状态
    RADAR_DET_DIST_ABN = 0xFF // 异常状态
};

// 预约充电状态
enum CHARGE_ORDER {
    CHARGE_ORDER_READY = 0x0, // 即插即充
    CHARGE_ORDER_ORDER = 0x1, // 预约充电
    CHARGE_ORDER_END = 0x2, // 结束充电
    CHARGE_ORDER_RESERVED = 0x3 // 预留
};

enum CHARGE_ORDER_MODE {
    CHARGE_ORDER_MODE_NONE = 0x0, // 未设置
    CHARGE_ORDER_MODE_TIME_START_END = 0x1, // 设置开始,结束时间
    CHARGE_ORDER_MODE_TIME_START= 0x2, // 仅设置开始时间
    CHARGE_ORDER_MODE_TIME_END= 0x3, // 仅设置结束时间
};

// 充电设备连接状态
enum CHARGE_DEVICE {
    CHARGE_DEVICE_NO_CONNECTED = 0x0,
    CHARGE_DEVICE_CONNECTED = 0x1,
    CHARGE_DEVICE_ILLEGAL = 0x2,
    CHARGE_DEVICE_SIM,
    CHARGE_DEVICE_DIS_CHARGE_CONNECTED,
    CHARGE_DEVICE_DIS_CHARGE_ENABLE,
    CHARGE_DEVICE_RESERVED = 0xFF
};

// 充电状态
enum CHARGING {
    CHARGING_NONE, // 没有充电
    CHARGING_DC, // 正在交流充电
    CHARGING_AC, // 正在直流充电
    CHARGING_END, // 充电结束
    CHARGING_FAULT, // 充电故障
    CHARGING_RESERVED = 0xFF // 预留
};

enum MACK {
    MACK_INVALID, // 无效
    MACK_OK, // 成功
    MACK_FAIL, // 异常
    MACK_RESERVED = 0xff // 预留
};

enum RESET_MODE {
    RESET_MODE_TRIP_A, // 短里程A
    RESET_MODE_AVR_POWER, // 平均能耗
    RESET_MODE_TRAVEL_MILE, // 行驶里程
    RESET_MODE_TRAVEL_TIME, // 行驶时间
};

enum IVIList {
    IVI_Music_List,
    IVI_Radio_LIst,
};

#pragma pack()
#endif
