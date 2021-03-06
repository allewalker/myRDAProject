#include "os.h"
#define SIM_SN			(CFW_SIM_0)
#define API_PS_DETACH_GPRS     1
#define __FAST_PWM__
const uint8_t gEmergencyNumNoSim[][2] = {{0x11, 0xF2}, {0x19, 0xF1},{0x11, 0xF0},{0x11, 0xF9},{0x99, 0xF9}};
extern PUBLIC UINT8* pal_GetImei(UINT8 simIndex);
extern UINT32 CFW_AttDetach (UINT8 nState, UINT16 nUTI, UINT8 AttDetachType
        , CFW_SIM_ID nSimID
       );
extern PUBLIC UINT16 pmd_GetGpadcBatteryLevel(VOID);
extern PUBLIC CONST UINT8 *pal_GetFactoryImei(UINT8 simIndex);
extern UINT32 CFW_getDnsServerbyPdp(UINT8 nCid, UINT8 nSimID );
extern BOOL hal_PwmResourceMgmt(VOID);
UINT32 CFW_GprsGetPdpAddr(UINT8 nCid, UINT8 *nLength, UINT8 *pPdpAdd, CFW_SIM_ID nSimID);




typedef double (*MathFun1)(double);
typedef double (*MathFun2)(double, double);

typedef struct
{
	void (*GPIOInit)(HAL_GPIO_GPIO_ID_T gpio, CONST HAL_GPIO_CFG_T* cfg);
	void (*GPIODeInit)(HAL_GPIO_GPIO_ID_T gpio);
	void (*SPIInit)(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum, CONST HAL_SPI_CFG_T* spiConfigPtr);
	void (*SPIOpen)(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
	void (*SPIClose)(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum);
	uint8_t (*DMAStart)(HAL_IFC_REQUEST_ID_T IfcID, uint8_t* Buf, uint32_t Len, HAL_IFC_MODE_T IfcMode);
	void (*I2COpen)(void);
	void (*I2CClose)(void);
	HAL_ERR_T (*I2CXfer)(HAL_I2C_BUS_ID_T BusId, uint8_t Addr, uint8_t *Reg, uint8_t RegNum, uint8_t *Buf, uint8_t Len, uint8_t WriteFlag, uint32_t To);
	void (*UartOpen)(HAL_UART_ID_T UartID, HAL_UART_CFG_T* uartCfg, HAL_UART_IRQ_STATUS_T mask, HAL_UART_IRQ_HANDLER_T handler);
	void (*UartClose)(HAL_UART_ID_T UartID);
	void (*UartSetBR)(HAL_UART_ID_T UartID, uint32_t BR);
	uint16_t (*GetVbatADC)(void);
	void (*PWLStart)(uint8_t Duty);
	void (*PWLStop)(void);
	void (*PWTStart)(uint16_t Freq, uint16_t Level, uint8_t Duty);
	void (*PWTStop)(void);
	uint8_t (*GetResetReason)(void);
	uint8_t (*SendEvent)(HANDLE hTask, uint32_t EventID, uint32_t Param1, uint32_t Param2, uint32_t Param3);
	void (*StartTimer)(HANDLE hTask, uint8_t nTimerId, uint8_t nMode, uint32_t nElapse);
	void (*StopTimer)(HANDLE hTask, uint8_t nTimerId);
	void (*Sleep)(uint32_t To);

	uint8_t (*GetSimStatus)(void);
	void (*GetICCID)(uint8_t *ICCID);
	void (*GetIMSI)(uint8_t *IMSI, int8_t *Str, uint32_t Len);
	void (*GetIMSIReq)(void);
	void (*GetIMEI)(uint8_t *IMEI);
	void (*FlyMode)(uint8_t Switch);
	uint8_t (*GetRegStatus)(void);
	void (*GPRSAttachReq)(uint8_t Req);
	void (*GetGPRSAttach)(uint8_t *State);
	void (*GPRSActReq)(uint8_t Req, uint8_t *APNName, uint8_t *APNUser, uint8_t *APNPassword);
	void (*GetGPRSActive)(uint8_t *State);
	void (*SetCIPIPPdpCxt)(uint8_t *APNName, uint8_t *APNUser, uint8_t *APNPassword);
	void (*GetCIPIPPdpCxt)(IP_AddrUnion *LocalIP, IP_AddrUnion *DNS);
	void (*GetCellInfo)(CFW_TSM_CURR_CELL_INFO *pCurrCellInfo, CFW_TSM_ALL_NEBCELL_INFO *pNeighborCellInfo);
	void (*StartTSM)(void);
	uint8_t (*Call)(uint8_t *Num, uint8_t NumLen, uint8_t Type);
	void (*CallAccpet)(void);
	void (*CallRelease)(void);
	void (*SMSInitStart)(uint8_t Param);
	void (*SMSInitFinish)(uint16_t nUTI, CFW_SMS_PARAMETER *sInfo);
	void (*SMSGetStorageInfo)(CFW_SMS_STORAGE_INFO *Info);
	uint32_t (*SMSTxByPDU)(uint8_t *pData, uint16_t nDataSize);

	uint32_t (*GetHost)(int8_t *Name, struct ip_addr *IP);
	SOCKET(*CreateSocket)(uint8_t nDomain, uint8_t nType, uint8_t nProtocol);
	uint32_t (*SocketConnect)(SOCKET SocketID, uint32_t LocalIP, uint16_t LocalPort, uint32_t RemoteIP, uint16_t RemotePort);
	uint32_t (*SocketDisconnect)(SOCKET SocketID);
	uint32_t (*SocketReceive)(SOCKET SocketID, uint8_t *Buf, uint32_t Len, CFW_TCPIP_SOCKET_ADDR *from, INT32 *fromlen);
	uint32_t (*SocketSend)(SOCKET SocketID, uint8_t *Buf, uint32_t Len, CFW_TCPIP_SOCKET_ADDR *to, INT32 tolen);
	int32_t (*TTS_Play)(void *Data, uint32_t Len, void *PCMCB, void *TTSCB);
	uint32_t (*UCS2ToGB2312)(const uint8_t * src, uint8_t * dst,uint32_t srclen, uint8_t IsBigEnding);
	uint32_t (*GB2312ToUCS2)(const uint8_t * src, uint8_t * dst,uint32_t srclen, uint8_t IsBigEnding);
	void (*FileSet)(uint32_t Len);
	uint8_t (*WriteFile)(uint8_t *Data, uint32_t Len);
	uint8_t (*UpgradeVaildCheck)(void);
	uint8_t (*GetUpgradeState)(void);
	void (*ClearUpgradeState)(void);
	UINT32 (*inet_addr)(const INT8 *cp);
	UINT32 (*htonl)(UINT32 n);
	UINT16 (*htons)(UINT16 n);
	char * (*strcpy)(char *to, const char *from);
	char * (*strcat)(char *s, const char *append);
	int (*strcmp)(const char *s1, const char *s2);
	char *(*strchr)(const char *p, int ch);
	char *(*strstr)(const char *s, const char *find);
	size_t (*strlen)(const char *);
	int (*memcmp)(const void *, const void *, size_t);
	void *(*memcpy)(void *dstpp, const void *srcpp, size_t len);
	void *(*memset)(void *dst0, int c0, size_t length);
	void *(*memmove)(void *, const void *, size_t);
	int (*vsnprintf)(char *buf, size_t size, const char *fmt, va_list ap);
	int (*vsprintf)(char *buf, const char *fmt, va_list ap);
	int (*sprintf)(char *buf, const char *fmt, ...);
	int (*snprintf)(char *buf, size_t size, const char *fmt, ...);
	MathFun1 sin;
	MathFun1 cos;
	MathFun1 tan;
	MathFun1 asin;
	MathFun1 acos;
	MathFun1 atan;
	MathFun2 pow;
	MathFun1 sqrt;
	MathFun1 exp;
	MathFun1 log;
	MathFun1 log10;
}OS_APIListStruct;

OS_APIListStruct gOSAPIList;

void OS_APIInit(void)
{
	gOSAPIList.GPIOInit = OS_GPIOInit;
	gOSAPIList.GPIODeInit = OS_GPIODeInit;
	gOSAPIList.SPIInit = OS_SPIInit;
	gOSAPIList.SPIOpen = OS_SPIOpen;
	gOSAPIList.SPIClose = OS_SPIClose;
	gOSAPIList.I2COpen = OS_I2COpen;
	gOSAPIList.I2CClose = OS_I2CClose;
	gOSAPIList.UartOpen = OS_UartOpen;
	gOSAPIList.UartClose = OS_UartClose;
	gOSAPIList.UartSetBR = OS_UartSetBR;
	gOSAPIList.DMAStart = OS_DMAStart;
	gOSAPIList.I2CXfer = OS_I2CXfer;
	gOSAPIList.GetVbatADC = OS_GetVbatADC;
	gOSAPIList.PWLStart = OS_PWLStart;
	gOSAPIList.PWLStop = OS_PWLStop;
	gOSAPIList.PWTStart = OS_PWTStart;
	gOSAPIList.PWTStop = OS_PWTStop;
	gOSAPIList.GetResetReason = OS_GetResetReason;
	gOSAPIList.SendEvent = OS_SendEvent;
	gOSAPIList.StartTimer = OS_StartTimer;
	gOSAPIList.StopTimer = OS_StopTimer;
	gOSAPIList.Sleep = OS_Sleep;
	gOSAPIList.GetIMEI = OS_GetIMEI;
	gOSAPIList.GetSimStatus = OS_GetSimStatus;
	gOSAPIList.GetICCID = OS_GetICCID;
	gOSAPIList.GetIMSI = OS_GetIMSI;
	gOSAPIList.GetIMSIReq = OS_GetIMSIReq;
	gOSAPIList.FlyMode = OS_FlyMode;
	gOSAPIList.GetRegStatus = OS_GetRegStatus;
	gOSAPIList.GetGPRSAttach = OS_GetGPRSAttach;
	gOSAPIList.GPRSAttachReq = OS_GPRSAttachReq;
	gOSAPIList.GPRSActReq = OS_GPRSActReq;
	gOSAPIList.SetCIPIPPdpCxt = OS_SetCIPIPPdpCxt;
	gOSAPIList.GetCIPIPPdpCxt = OS_GetCIPIPPdpCxt;
	gOSAPIList.GetCellInfo = OS_GetCellInfo;
	gOSAPIList.StartTSM = OS_StartTSM;
	gOSAPIList.Call = OS_Call;
	gOSAPIList.CallAccpet = OS_CallAccpet;
	gOSAPIList.CallRelease = OS_CallRelease;
	gOSAPIList.SMSInitStart = OS_SMSInitStart;
	gOSAPIList.SMSInitFinish = OS_SMSInitFinish;
	gOSAPIList.SMSGetStorageInfo = OS_SMSGetStorageInfo;
	gOSAPIList.SMSTxByPDU = OS_SMSTxByPDU;
	gOSAPIList.GetHost = OS_GetHost;
	gOSAPIList.CreateSocket = OS_CreateSocket;
	gOSAPIList.SocketConnect = OS_SocketConnect;
	gOSAPIList.SocketDisconnect = OS_SocketDisconnect;
	gOSAPIList.SocketReceive = OS_SocketReceive;
	gOSAPIList.SocketSend = OS_SocketSend;
#ifdef __TTS_ENABLE__
	gOSAPIList.TTS_Play = __TTS_Play;
#endif
	gOSAPIList.UCS2ToGB2312 = OS_UCS2ToGB2312;
	gOSAPIList.GB2312ToUCS2 = OS_GB2312ToUCS2;
	gOSAPIList.FileSet = __FileSet;
	gOSAPIList.WriteFile = __WriteFile;
	gOSAPIList.UpgradeVaildCheck = __UpgradeVaildCheck;
	gOSAPIList.GetUpgradeState = __GetUpgradeState;
	gOSAPIList.ClearUpgradeState = __ClearUpgradeState;
	gOSAPIList.inet_addr = inet_addr;
	gOSAPIList.htonl = htonl;
	gOSAPIList.htons = htons;
	gOSAPIList.strcpy = strcpy;
	gOSAPIList.strcat = strcat;
	gOSAPIList.strcmp = strcmp;
	gOSAPIList.strchr = strchr;
	gOSAPIList.strstr = strstr;
	gOSAPIList.strlen = strlen;
	gOSAPIList.memcmp = memcmp;
	gOSAPIList.memcpy = memcpy;
	gOSAPIList.memset = memset;
	gOSAPIList.memmove = memmove;
	gOSAPIList.vsnprintf = vsnprintf;
	gOSAPIList.vsprintf = vsprintf;
	gOSAPIList.sprintf = sprintf;
	gOSAPIList.snprintf = snprintf;
	gOSAPIList.sin = sin;
	gOSAPIList.cos = cos;
	gOSAPIList.tan = tan;
	gOSAPIList.asin = asin;
	gOSAPIList.acos = acos;
	gOSAPIList.atan = atan;
	gOSAPIList.pow = pow;
	gOSAPIList.sqrt = sqrt;
	gOSAPIList.exp = exp;
	gOSAPIList.log = log;
	gOSAPIList.log10 = log10;

}

void OS_GPIOInit(HAL_GPIO_GPIO_ID_T gpio, CONST HAL_GPIO_CFG_T* cfg)
{
	hal_GpioOpen(gpio, cfg);
}
void OS_GPIODeInit(HAL_GPIO_GPIO_ID_T gpio)
{
	hal_GpioClose(gpio);
}
//SPI
void OS_SPIInit(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum, CONST HAL_SPI_CFG_T* spiConfigPtr)
{
	hal_SpiOpen(BusId, csNum, spiConfigPtr);
}

void OS_SPIOpen(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum)
{
	hal_SpiActivateCs(BusId, csNum);
}

void OS_SPIClose(HAL_SPI_ID_T BusId, HAL_SPI_CS_T csNum)
{
	hal_SpiDeActivateCs(BusId, csNum);
}
//DMA
uint8_t OS_DMAStart(HAL_IFC_REQUEST_ID_T IfcID, uint8_t* Buf, uint32_t Len, HAL_IFC_MODE_T IfcMode)
{
	return hal_IfcTransferStart(IfcID, Buf, Len, IfcMode);
}
//I2C
void OS_I2COpen(void)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->GPIO_Mode &= ~((1 << 24)|(1 << 25));
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	 hwp_iomux->pad_GPIO_6_cfg = IOMUX_PAD_GPIO_6_SEL(2);
	 hwp_iomux->pad_GPIO_7_cfg = IOMUX_PAD_GPIO_7_SEL(2);
#endif
}

void OS_I2CClose(void)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_configRegs->GPIO_Mode |= (1 << 24)|(1 << 25);
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	 hwp_iomux->pad_GPIO_6_cfg = IOMUX_PAD_GPIO_6_SEL(0);
	 hwp_iomux->pad_GPIO_7_cfg = IOMUX_PAD_GPIO_7_SEL(0);
#endif
}

void OS_UartOpen(HAL_UART_ID_T UartID, HAL_UART_CFG_T* uartCfg, HAL_UART_IRQ_STATUS_T mask, HAL_UART_IRQ_HANDLER_T handler)
{
	hal_UartOpen(UartID, uartCfg);
	hal_UartIrqSetMask(UartID, mask);
#ifdef __IRQ_CB_WITH_PARAM__
	hal_UartIrqSetHandler(UartID, handler, 0);
#else
	hal_UartIrqSetHandler(UartID, handler);
#endif
}

void OS_UartClose(HAL_UART_ID_T UartID)
{
	hal_UartClose(UartID);
}

void OS_UartSetBR(HAL_UART_ID_T UartID, uint32_t BR)
{
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	UINT32 uartClockDivisor = 0;
    switch(BR)
    {
        //  Using the slow clock at 52MHz   //   8955  // div is set 4
        case HAL_UART_BAUD_RATE_3250000: //0x1001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(4);
            break;
        case HAL_UART_BAUD_RATE_2166700: //0x1801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(6);
            break;
        case HAL_UART_BAUD_RATE_1625000: //0x2001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(8);
            break;
        case HAL_UART_BAUD_RATE_1300000: //0x2801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(10);
            break;
        case HAL_UART_BAUD_RATE_921600:  //0x3801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(14);
            break;
        case HAL_UART_BAUD_RATE_460800: //0x2c1464
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(100)|SYS_CTRL_CFG_UART_DENOM(2821);
            break;
        case HAL_UART_BAUD_RATE_230400: //0x46805
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(5)|SYS_CTRL_CFG_UART_DENOM(282);
            break;
        case HAL_UART_BAUD_RATE_115200: //0x1c401
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(113);
            break;
        case HAL_UART_BAUD_RATE_57600: //0x23440a
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(10)|SYS_CTRL_CFG_UART_DENOM(2257);
            break;
        case HAL_UART_BAUD_RATE_38400: //0xa9402
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(2)|SYS_CTRL_CFG_UART_DENOM(677);
            break;
        case HAL_UART_BAUD_RATE_33600: //0x1e3405
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(5)|SYS_CTRL_CFG_UART_DENOM(1933);
            break;
        case HAL_UART_BAUD_RATE_28800: //0x70c01
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(451);
            break;
        case HAL_UART_BAUD_RATE_19200: //0xa9401
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(677);
            break;
        case HAL_UART_BAUD_RATE_14400: //0xe1c01
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(903);
            break;
        case HAL_UART_BAUD_RATE_9600: //0x152801
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(1354);
            break;
        case HAL_UART_BAUD_RATE_4800: //0x2a5001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(2708);
            break;
        case HAL_UART_BAUD_RATE_2400: //0x54a001
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(5416);//  52/4/0.0024
            break;
        case HAL_UART_BAUD_RATE_1200: //0xa94401
            uartClockDivisor = SYS_CTRL_CFG_UART_NUM(1)|SYS_CTRL_CFG_UART_DENOM(10833);//  52/4/0.0012
            break;

        default:
            // Baud rate is calculated as 6.5M/2, 6.5M/3, 6.5M/4, ...
            // Limit the number of supported baud rates to avoid
            // rounding issue in the division.
            return;
    }
    hwp_sysCtrl->Cfg_Clk_Uart[UartID] = uartClockDivisor;
#endif

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
    UINT32 uartClockDivisorMode = 4;
    UINT32 uartClockDivisor = 0;
    UINT32 fs, fsSys, mode;
    UINT32 clockToUse;
    register UINT32 uartConfig;
    switch(BR)
    {

	//  Using mode divisor = 16
	case HAL_UART_BAUD_RATE_2400:
	case HAL_UART_BAUD_RATE_4800:
		uartConfig |= UART_DIVISOR_MODE;
		uartClockDivisorMode = 16;
		break;
		//  Using mode divisor = 4
	default:
		uartConfig &= ~UART_DIVISOR_MODE;
		uartClockDivisorMode = 4;
		break;
    }
    fsSys = 26000000;
    clockToUse = SYS_CTRL_UART_SEL_PLL_SLOW;
    fs = BR;
    mode = uartClockDivisorMode;
    uartClockDivisor = ( (fsSys + ((mode / 2) * fs)) / (mode * fs) ) - 2;

    //  Configure the clock register.
    hwp_sysCtrl->Cfg_Clk_Uart[UartID] =
        SYS_CTRL_UART_DIVIDER(uartClockDivisor) | clockToUse;
#endif
}

static void OS_I2CClockDown(void)
{
	UINT32 criticalSectionValue;
    criticalSectionValue = hal_SysEnterCriticalSection();
    hal_SysRequestFreq(HAL_SYS_FREQ_I2C, HAL_SYS_FREQ_32K, NULL);
    hal_SysExitCriticalSection(criticalSectionValue);
}

static void OS_I2CClockUpdate(HAL_SYS_FREQ_T sysFreq)
{
    UINT32 newClkScale;
    UINT32 ctrlReg;

    ctrlReg = hwp_i2cMaster->CTRL & ~(I2C_MASTER_CLOCK_PRESCALE_MASK);
    newClkScale = sysFreq/(5 * 400000)-1;
    ctrlReg |= I2C_MASTER_CLOCK_PRESCALE(newClkScale);
    hwp_i2cMaster->CTRL = ctrlReg;

    ctrlReg = hwp_i2cMaster2->CTRL & ~(I2C_MASTER_CLOCK_PRESCALE_MASK);
    ctrlReg |= I2C_MASTER_CLOCK_PRESCALE(newClkScale);
    hwp_i2cMaster2->CTRL = ctrlReg;

    ctrlReg = hwp_i2cMaster3->CTRL & ~(I2C_MASTER_CLOCK_PRESCALE_MASK);
    ctrlReg |= I2C_MASTER_CLOCK_PRESCALE(newClkScale);
    hwp_i2cMaster3->CTRL = ctrlReg;

}

static HAL_ERR_T OS_I2CGetData(uint8_t BusId, uint8_t Addr, uint8_t *Reg, uint8_t RegNum, uint8_t *Buf, uint8_t Len, uint8_t WriteFlag, uint32_t To)
{
    UINT32 second_time,first_time;
    HWP_I2C_MASTER_T* i2cMaster;
    UINT32 criticalSectionValue;
    UINT32 currentByte = 0;
    To = (To * SYS_TICK) / 1000;
    uint8_t i;

    first_time = hal_TimGetUpTime();

    switch (BusId)
    {
    case HAL_I2C_BUS_ID_1:
    	i2cMaster = hwp_i2cMaster;
    	break;
    case HAL_I2C_BUS_ID_2:
    	i2cMaster = hwp_i2cMaster2;
    	break;
    case HAL_I2C_BUS_ID_3:
    	i2cMaster = hwp_i2cMaster3;
    	break;
    default:
    	return HAL_ERR_RESOURCE_NOT_ENABLED;
    }



    criticalSectionValue = hal_SysEnterCriticalSection();
    hal_SysRequestFreq(HAL_SYS_FREQ_I2C, HAL_SYS_FREQ_104M, OS_I2CClockUpdate);
    OS_I2CClockUpdate(hal_SysGetFreq());
    hal_SysExitCriticalSection(criticalSectionValue);

    // Set the new clock scal.
    // Clear status bit in case previous transfer (Raw) hasn't
    // cleared it.
    i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

    if (!RegNum)
    {
    	if (!WriteFlag)
    	{
    		goto I2C_RAW_READ;
    	}
    }

    // Write slave address (Write mode, to write memory address)
    i2cMaster -> TXRX_BUFFER = (Addr << 1);
    i2cMaster -> CMD = I2C_MASTER_WR | I2C_MASTER_STA;

    // Polling on the TIP flag
    /*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
    while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
    {
        second_time = hal_TimGetUpTime();
        if (second_time - first_time > To)
        {
            i2cMaster->CMD = I2C_MASTER_STO;
            OS_I2CClockDown();
            return HAL_ERR_RESOURCE_TIMEOUT;
        }
    };

    i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

    // Transfert done

    // Check RxACK
    if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
    {
        // Abort the transfert
        i2cMaster -> CMD = I2C_MASTER_STO ;
        while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
        {
            second_time = hal_TimGetUpTime();
            if (second_time - first_time > To)
            {
            	OS_I2CClockDown();
                return HAL_ERR_RESOURCE_TIMEOUT;
            }
        };

        i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;
        OS_I2CClockDown();
        return HAL_ERR_COMMUNICATION_FAILED;
    }

    for(i = 0; i < RegNum; i++)
    {
	   // Write memory address
		i2cMaster -> TXRX_BUFFER = Reg[i];
		i2cMaster -> CMD = I2C_MASTER_WR;

		// Polling on the TIP flag
		/*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
		while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
		{
			second_time = hal_TimGetUpTime();
			if (second_time - first_time > To)
			{
				i2cMaster->CMD = I2C_MASTER_STO;
				OS_I2CClockDown();
				return HAL_ERR_RESOURCE_TIMEOUT;
			}
		};

		i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

		// Check RxACK
		if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
		{
			// Abort the transfert
			i2cMaster -> CMD = I2C_MASTER_STO ;
			while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
			{
				second_time = hal_TimGetUpTime();
				if (second_time - first_time > To)
				{
					OS_I2CClockDown();
					return HAL_ERR_RESOURCE_TIMEOUT;
				}
			};

			i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;
			OS_I2CClockDown();
			return HAL_ERR_COMMUNICATION_FAILED;
		}
    }

    if (WriteFlag)
    {
	   // Write all data but the last one.
		for (currentByte = 0 ; currentByte < Len - 1 ; currentByte++)
		{
			i2cMaster -> TXRX_BUFFER = Buf[currentByte];
			i2cMaster -> CMD = I2C_MASTER_WR;

			// Polling on the TIP flag
			/*         while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
			while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
			{
				second_time = hal_TimGetUpTime();
				if (second_time - first_time > To)
				{
					i2cMaster->CMD = I2C_MASTER_STO;
					OS_I2CClockDown();
					return HAL_ERR_RESOURCE_TIMEOUT;
				}
			};

			i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

			// Check RxACK
			if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
			{
				// Stop condition sent via the previous
				// command
				OS_I2CClockDown();
				return HAL_ERR_COMMUNICATION_FAILED;
			}
		}

		// Send last byte with stop condition
		i2cMaster -> TXRX_BUFFER = Buf[Len - 1];
		i2cMaster -> CMD = I2C_MASTER_WR | I2C_MASTER_STO ;

		// Polling on the TIP flag
		/*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
		while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
		{
			second_time = hal_TimGetUpTime();
			if(second_time - first_time > To)
			{
				OS_I2CClockDown();
				return HAL_ERR_RESOURCE_TIMEOUT;
			}
		};

		i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;


		// Check RxACK
		if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
		{
			// Stop condition sent via the previous
			// command
			OS_I2CClockDown();
			return HAL_ERR_COMMUNICATION_FAILED;
		}
		goto I2C_FINISH;
    }

I2C_RAW_READ:
	// Write slave address + R/W = '1' (Read mode)
	i2cMaster -> TXRX_BUFFER = (Addr << 1 | 0x1);
	i2cMaster -> CMD = I2C_MASTER_WR | I2C_MASTER_STA;

	// Polling on the TIP flag
	/*     while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
	while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
	{
		second_time = hal_TimGetUpTime();
		if (second_time - first_time > To)
		{
			i2cMaster->CMD = I2C_MASTER_STO;
			OS_I2CClockDown();
			return HAL_ERR_RESOURCE_TIMEOUT;
		}
	};

	i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

	// Transfert done

	// Check RxACK
	if (i2cMaster -> STATUS & I2C_MASTER_RXACK)
	{
		// Abort the transfert
		i2cMaster -> CMD = I2C_MASTER_STO ;
		while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
		{
			second_time = hal_TimGetUpTime();
			if (second_time - first_time > To)
			{
				OS_I2CClockDown();
				return HAL_ERR_RESOURCE_TIMEOUT;
			}
		};

		i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;
		OS_I2CClockDown();
		return HAL_ERR_COMMUNICATION_FAILED;
	}

	// Read all values but the last one
	for (currentByte=0; currentByte<Len-1 ; currentByte++)
	{
		// Read value
		i2cMaster -> CMD = I2C_MASTER_RD;

		// Polling on the TIP flag
		/*         while(i2cMaster -> STATUS & I2C_MASTER_TIP); */
		while(!(i2cMaster -> STATUS & I2C_MASTER_IRQ_STATUS))
		{
			second_time = hal_TimGetUpTime();
			if (second_time - first_time > To)
			{
				i2cMaster->CMD = I2C_MASTER_STO;
				OS_I2CClockDown();
				return HAL_ERR_RESOURCE_TIMEOUT;
			}
		};

		i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

		// Store read value
		Buf[currentByte] = i2cMaster -> TXRX_BUFFER;
	}

	// Read last value - send no acknowledge - send stop condition/bit
	i2cMaster -> CMD = I2C_MASTER_RD | I2C_MASTER_ACK | I2C_MASTER_STO;

	// Polling on the TIP flag
	while(i2cMaster -> STATUS & I2C_MASTER_TIP)
	{
		second_time = hal_TimGetUpTime();
		if (second_time - first_time > To)
		{
			OS_I2CClockDown();
			return HAL_ERR_RESOURCE_TIMEOUT;
		}
	};

	i2cMaster->IRQ_CLR = I2C_MASTER_IRQ_CLR;

	Buf[Len-1] = i2cMaster -> TXRX_BUFFER;
I2C_FINISH:
    OS_I2CClockDown();
    return HAL_ERR_NO;
}

HAL_ERR_T OS_I2CXfer(HAL_I2C_BUS_ID_T BusId, uint8_t Addr, uint8_t *Reg, uint8_t RegNum, uint8_t *Buf, uint8_t Len, uint8_t WriteFlag, uint32_t To)
{
	HAL_ERR_T Error;
	uint8_t Retry = 0;
	uint8_t Result = 0;
	while(Retry < 2)
	{
		Error = hal_I2cOpen(BusId);
		if (Error)
		{
			Result = Error;
			Retry++;
			continue;
		}

		Error = OS_I2CGetData(BusId, Addr, Reg, RegNum, Buf, Len, WriteFlag, To);

		if (Error)
		{
			Result = Error;
			Retry++;
			continue;
		}
		else
		{
			Result = 0;
			break;
		}
	}
	hal_I2cClose(BusId);
	return Result;
}

uint16_t OS_GetVbatADC(void)
{
	return pmd_GetGpadcBatteryLevel();
}

void OS_PWMUpdateDivider(HAL_SYS_FREQ_T freq)
{
    UINT32 divider;

    if (freq < HAL_SYS_FREQ_13M)
    {
    	return ;
    }
    divider = freq / (HAL_SYS_FREQ_13M) - 1;

    if (divider > 0xFF)
    {
        divider = 0xFF;
    }
    hwp_sysCtrl->Cfg_Clk_PWM = divider;
}

void OS_PWMClkUpdate(void)
{
    if ( (hwp_pwm->PWL0_Config & PWM_PWL0_EN_H) != 0 ||
            (hwp_pwm->PWL1_Config & PWM_PWL1_EN_H) != 0 ||
            (hwp_pwm->PWT_Config & PWM_PWT_ENABLE) != 0 )
    {
        hal_SysRequestFreq(HAL_SYS_FREQ_PWM, HAL_SYS_FREQ_26M, OS_PWMUpdateDivider);

        UINT32 scStatus = hal_SysEnterCriticalSection();
        OS_PWMUpdateDivider(hal_SysGetFreq());
        hal_SysExitCriticalSection(scStatus);
        return ;
    }
    else
    {
        hal_SysRequestFreq(HAL_SYS_FREQ_PWM, HAL_SYS_FREQ_32K, NULL);
        return ;
    }
}

void OS_PWLStart(uint8_t Duty)
{
#if (__BOARD__ == __AIR201__) || (__BOARD__ == __AIR202__)
	hwp_pwm->PWL1_Config = PWM_PWL1_SET_OE|(PWM_PWL1_EN_H | PWM_PWL1_THRESHOLD(Duty));
#ifdef __FAST_PWM__
	OS_PWMClkUpdate();
#else
	hal_PwmResourceMgmt();
#endif
	hwp_iomux->pad_GPIO_2_cfg = IOMUX_PAD_GPIO_2_SEL_FUN_PWL_1_SEL;
#endif
}

void OS_PWLStop(void)
{
#if (__BOARD__ == __AIR201__) || (__BOARD__ == __AIR202__)
	hwp_pwm->PWL1_Config = 0;
#ifdef __FAST_PWM__
	OS_PWMClkUpdate();
#else
	hal_PwmResourceMgmt();
#endif
	hwp_iomux->pad_GPIO_2_cfg = IOMUX_PAD_GPIO_2_SEL_FUN_GPIO_2_SEL;
#endif
}

void OS_PWTStart(uint16_t Freq, uint16_t Level, uint8_t Duty)
{
#if (__BOARD__ == __AIR201__) || (__BOARD__ == __AIR202__)

    uint32_t noteDiv;
    uint32_t dutyCmp;
    if (!Freq || (!Level && !Duty))
    {
    	CORE("%u %u %u",Freq, Level, Duty);
    	return;
    }
#ifdef __FAST_PWM__
    noteDiv = HAL_SYS_FREQ_13M / Freq;
#else
	noteDiv = HAL_SYS_FREQ_13M / 10 /Freq;
#endif

    if (Level > noteDiv)
    {
    	CORE("%u %u",noteDiv, Level);
    	return;
    }

    if (Level)
    {
    	dutyCmp = Level;
    }
    else
    {
    	dutyCmp = noteDiv * Duty / 100;
    }
    hwp_pwm->PWT_Config =
        (PWM_PWT_PERIOD(noteDiv) | PWM_PWT_DUTY(dutyCmp) | PWM_PWT_ENABLE);
#ifdef __FAST_PWM__
	OS_PWMClkUpdate();
#else
	hal_PwmResourceMgmt();
#endif
	hwp_iomux->pad_GPIO_5_cfg = IOMUX_PAD_GPIO_5_SEL_FUN_PWT_SEL;
#endif
}
void OS_PWTStop(void)
{
#if (__BOARD__ == __AIR201__) || (__BOARD__ == __AIR202__)
    hwp_pwm->PWT_Config = 0;
#ifdef __FAST_PWM__
	OS_PWMClkUpdate();
#else
	hal_PwmResourceMgmt();
#endif
	hwp_iomux->pad_GPIO_5_cfg = IOMUX_PAD_GPIO_5_SEL_FUN_GPIO_5_SEL;
#endif
}

uint8_t OS_GetResetReason(void)
{
	return hal_SysGetResetCause();
}

void OS_StartTimer(HANDLE hTask, uint8_t nTimerId, uint8_t nMode, uint32_t nElapse)
{
	COS_KillTimer(hTask, nTimerId);
	if (nElapse > 0x7FFFFFFF)
	{
		CORE("%d %u", nTimerId, nElapse);
		nElapse = 60 * SYS_TICK;
	}
	if (FALSE == COS_SetTimer(hTask, nTimerId, nMode, nElapse))
	{
		CORE("%d timer fail", nTimerId);
	}
}
void OS_StopTimer(HANDLE hTask, uint8_t nTimerId)
{
	COS_KillTimer(hTask, nTimerId);
}

void OS_Sleep(uint32_t To)
{
	sxr_Sleep(To);
}

uint8_t OS_SendEvent(HANDLE hTask, uint32_t EventID, uint32_t Param1, uint32_t Param2, uint32_t Param3)
{
	COS_EVENT Event;
	Event.nEventId = EventID;
	Event.nParam1 = Param1;
	Event.nParam2 = Param2;
	Event.nParam3 = Param3;
	return COS_SendEvent(hTask, &Event, COS_WAIT_FOREVER, COS_EVENT_PRI_NORMAL);
}


void OS_GetIMEI(uint8_t *IMEI)
{
//	uint8_t Buf[128];
//	uint8_t i;
//	uint32_t Addr = 0x003FE000;
	uint8_t *P = (uint8_t *)pal_GetImei(SIM_SN);
	if (P)
	{
		memcpy(IMEI, P, IMEI_LEN);
		return ;
	}
	memset(IMEI, 0, IMEI_LEN);
//	__ReadFlash(Addr, Buf, 128);
//	HexTrace(Buf, 16);
//	Addr = 0x003FC000;
//	__ReadFlash(Addr, Buf, 128);
//	HexTrace(Buf, 40);
}
#define IsDigit(c)        ((c >= '0') && (c <= '9'))
void OS_GetIMSI(uint8_t *IMSI, int8_t *Str, uint32_t Len)
{
	uint8_t *Temp;
	uint32_t i,j;
	Len = Len & 0x000000ff;
	Temp = COS_MALLOC(Len + 1);
	memset(Temp, 0, Len + 1);
	j = 1;
	for (i = 0; i < Len; i++)
	{
		if (IsDigit(Str[i]))
		{
			Temp[j++] = Str[i] - '0';
		}
		else
		{
			CORE("OS_GetIMSI %c", Str[i]);
			COS_FREE(Temp);
			memset(IMSI, 0, IMSI_LEN);
			break;
		}
	}
	for (i = 0; i < IMSI_LEN; i++)
	{
		IMSI[i] = (Temp[i * 2] << 4) | (Temp[i * 2 + 1] & 0x0f);
	}
	COS_FREE(Temp);
}

void OS_GetIMSIReq(void)
{
	CFW_SimGetProviderId(UTI_GET_IMSI, SIM_SN);
}

void OS_FlyMode(uint8_t Switch)
{
	uint32_t nFM = 0;
	uint32_t Error;
	if (Switch)
	{
		Error = CFW_SetComm(CFW_DISABLE_COMM, 0, UTI_FLY_MODE, SIM_SN);
		if (Error != ERR_SUCCESS)
		{
			CORE("%s %u:%u",__FUNCTION__, __LINE__, Error);
		}
	}
	else
	{
		Error = CFW_GetComm((CFW_COMM_MODE *)&nFM, SIM_SN);
		if (nFM == CFW_DISABLE_COMM)
		{
			Error = CFW_SetComm(CFW_ENABLE_COMM, 0, UTI_FLY_MODE, SIM_SN);
			if (Error != ERR_SUCCESS)
			{
				CORE("%s %u:%u",__FUNCTION__, __LINE__, Error);
			}
		}
		else
		{
			CORE("%s %u:%u",__FUNCTION__, __LINE__, Error);
		}
	}
}

void OS_GetICCID(uint8_t *ICCID)
{
	uint8_t i;
	uint8_t *Temp = CFW_GetICCID(SIM_SN);
	if (Temp)
	{
		for (i = 0; i < ICCID_LEN; i++)
		{
			ICCID[i] = (Temp[i] >> 4) | (Temp[i] << 4);
		}
	}
	else
	{
		memset(ICCID, 0, ICCID_LEN);
	}
}

uint8_t OS_GetSimStatus(void)
{
	return CFW_GetSimStatus(SIM_SN);
}

void OS_GetGPRSAttach(uint8_t *State)
{
	CFW_GetGprsAttState(State, SIM_SN);
}

void OS_GetCellInfo(CFW_TSM_CURR_CELL_INFO *pCurrCellInfo, CFW_TSM_ALL_NEBCELL_INFO *pNeighborCellInfo)
{
	uint32_t Error = CFW_GetCellInfo(pCurrCellInfo, pNeighborCellInfo, SIM_SN);
	if (Error)
	{
		CORE("OS_GetCellInfo %x", Error);
	}
	else
	{
//		DBG("%x%x%x%x %u", gSys.CurrentCell.nTSM_LAI[3], gSys.CurrentCell.nTSM_LAI[4],
//				gSys.CurrentCell.nTSM_CellID[0], gSys.CurrentCell.nTSM_CellID[1],
//				gSys.CurrentCell.nTSM_AvRxLevel);
	}
}

uint8_t OS_GetRegStatus(void)
{
	CFW_NW_STATUS_INFO nStatusInfo;
    UINT32 nRet;
    nRet = CFW_NwGetStatus(&nStatusInfo, CFW_SIM_0);
    if (ERR_SUCCESS != nRet)
    {
    	return 0;
    }
    else
    {
    	return nStatusInfo.nStatus;
    }
}

void OS_GPRSActReq(uint8_t Req, uint8_t *APNName, uint8_t *APNUser, uint8_t *APNPassword)
{
	uint32_t Error;

	if (Req == CFW_GPRS_ACTIVED)
	{
		OS_SetCIPIPPdpCxt(APNName, APNUser, APNPassword);
		Error = CFW_GprsAct(Req, CID_IP, UTI_CID_IP_ACT, SIM_SN);
	}
	else
	{
		Error = CFW_GprsAct(Req, CID_IP, UTI_CID_IP_DEACT, SIM_SN);
	}
	if (Error)
	{
		CORE("OS_GPRSActReq %x", Error);
	}
}

void OS_GetGPRSActive(uint8_t *State)
{
	uint32_t Error = CFW_GetGprsActState(CID_IP, State, SIM_SN);
	if (Error)
	{
		CORE("OS_GetGPRSActive %x", Error);
	}
}

void OS_GetCIPIPPdpCxt(IP_AddrUnion *LocalIP, IP_AddrUnion *DNS)
{
	uint32_t Error;
	uint8_t IPSize;
	Error = CFW_GprsGetPdpAddr(CID_IP, &IPSize, LocalIP->u8_addr, SIM_SN);
	DNS->u32_addr = (uint32_t)CFW_getDnsServerbyPdp(CID_IP, SIM_SN);
	if (Error)
	{
		CORE("OS_GetCIPIPPdpCxt %x", Error);
	}
}

void OS_SetCIPIPPdpCxt(uint8_t *APNName, uint8_t *APNUser, uint8_t *APNPassword)
{
	uint32_t Error;
	CFW_GPRS_PDPCONT_INFO PdpCont;
	//CFW_GPRS_QOS stTmpQos     = { 0, 0, 0, 0, 0 };
	//CFW_GPRS_QOS stTmpNullQos = { 3, 4, 3, 4, 16 };
	memset(&PdpCont, 0, sizeof(PdpCont));
	PdpCont.nPdpType = CFW_GPRS_PDP_TYPE_IP;
	PdpCont.nApnSize = strlen(APNName);
	PdpCont.nApnUserSize = strlen(APNUser);
	PdpCont.nApnPwdSize = strlen(APNPassword);
//	DBG("%u %u %u", PdpCont.nApnSize, PdpCont.nApnUserSize, PdpCont.nApnPwdSize);
	PdpCont.pApn = APNName;
	PdpCont.pApnUser = APNUser;
	PdpCont.pApnPwd = APNPassword;
//	CFW_GprsGetReqQos(CID_IP, &stTmpQos, SIM_SN);
//	CORE("OS_SetCIPIPPdpCxt %u %u %u %u %u", stTmpQos.nDelay, stTmpQos.nMean, stTmpQos.nPeak, stTmpQos.nPrecedence,
//			stTmpQos.nReliability);
//	if (!stTmpQos.nDelay)
//	{
//		CFW_GprsSetReqQos(CID_IP, &stTmpNullQos, SIM_SN);
//		CORE("OS_SetCIPIPPdpCxt %u %u %u %u %u", stTmpNullQos.nDelay, stTmpNullQos.nMean, stTmpNullQos.nPeak, stTmpNullQos.nPrecedence,
//			stTmpNullQos.nReliability);
//	}
	Error = CFW_GprsSetPdpCxt(CID_IP, &PdpCont, SIM_SN);
	if (Error)
	{
		CORE("OS_SetCIPIPPdpCxt %x", Error);
	}
}

void OS_StartTSM(void)
{
	uint32_t Error;
	CFW_TSM_FUNCTION_SELECT tSelecFUN;
	tSelecFUN.nNeighborCell = 1;
	tSelecFUN.nServingCell = 1;
	tSelecFUN.pad[0] = 0;
	tSelecFUN.pad[1] = 0;
	Error = CFW_EmodOutfieldTestStart(&tSelecFUN, UTI_TSM, SIM_SN);
	if (Error)
	{
		CORE("OS_StartTSM %x", Error);
	}
}

void OS_GPRSAttachReq(uint8_t Req)
{
	uint32_t Error;
	if (Req == CFW_GPRS_ATTACHED)
	{
		Error = CFW_GprsAtt(CFW_GPRS_ATTACHED, UTI_GPRS_ATTACH, SIM_SN);
	}
	else
	{
		Error = CFW_AttDetach(CFW_GPRS_DETACHED, UTI_GPRS_DETACH, API_PS_DETACH_GPRS, SIM_SN);
	}
	if (Error)
	{
		CORE("OS_GPRSAttachReq %x", Error);
	}
}

uint8_t OS_Call(uint8_t *Num, uint8_t NumLen, uint8_t Type)
{
	uint8_t i;
	uint32_t iRet;
	CFW_DIALNUMBER sDailNumber;
	for( i = 0; i <  SIZEOF( gEmergencyNumNoSim ) / SIZEOF( gEmergencyNumNoSim[i]); i++ )
	{
		//if (iLen == 2 && sDailNumber.pDialNumber[0] == gEmergencyNum[i][0] && sDailNumber.pDialNumber[1] == gEmergencyNum[i][1])
		if( (2 == NumLen) && (!memcmp(Num, gEmergencyNumNoSim[i], NumLen)) )
		{
			// [[hameina[+] 2007-10-30:bug 6929
//			CFW_CC_CURRENT_CALL_INFO CallInfo[7];
//			UINT8 nCnt = 0;
//			iRet = CFW_CcGetCurrentCall(CallInfo, &nCnt, SIM_SN);
//			while (nCnt)
//			{
//				if (!CallInfo[nCnt - 1].status) // status==0,active
//				{
//					CFW_CcCallHoldMultiparty(1, CallInfo[nCnt - 1].idx, SIM_SN);
//				}
//				nCnt--;
//			}
//			COS_Sleep(1000);
			iRet = CFW_CcEmcDial(Num, NumLen, SIM_SN);
			if (iRet != ERR_SUCCESS)
			{
				CORE("%x",iRet);
				return 0;
			}
			return 1;
		}
	}
	sDailNumber.pDialNumber = Num;
	sDailNumber.nDialNumberSize = NumLen;
	sDailNumber.nType = Type;
	sDailNumber.nClir = 0;

	iRet = CFW_CcInitiateSpeechCall(&sDailNumber, UTI_MAKE_CALL, SIM_SN);
	if (iRet != ERR_SUCCESS)
	{
		CORE("%x",iRet);
		return 0;
	}
	return 1;
}

void OS_CallAccpet(void)
{
	UINT32 nRet = CFW_CcGetCallStatus(SIM_SN);
	if ((CC_STATE_INCOMING != nRet) && (CC_STATE_WAITING != nRet))
	{
		CORE("OS_CallAccpet %02x", nRet);
		return;
	}
	CFW_CcAcceptSpeechCall(SIM_SN);
}

void OS_CallRelease(void)
{
	UINT32 uStat = CFW_CcGetCallStatus(SIM_SN);
	if (CC_STATE_NULL == uStat)
	{
		return;
	}
	CFW_CcReleaseCall(SIM_SN);
}

void OS_SMSInitStart(uint8_t Param)
{
    CFW_SMS_STORAGE_INFO nStorageInfo;
    //ML_SetCodePage(ML_CP936);
    nStorageInfo.totalSlot = Param;
    CFW_CfgSetSmsStorageInfo(&nStorageInfo, CFW_SMS_STORAGE_SM, SIM_SN);
    nStorageInfo.totalSlot = PHONE_SMS_ENTRY_COUNT / NUMBER_OF_SIM;
    CFW_CfgSetSmsStorageInfo(&nStorageInfo, CFW_SMS_STORAGE_ME, SIM_SN);
    CFW_CfgSetSmsStorageInfo(&nStorageInfo, CFW_SMS_STORAGE_MT, SIM_SN);
    CFW_SmsMoInit(UTI_SMS_INIT, SIM_SN);
}

void OS_SMSInitFinish(uint16_t nUTI, CFW_SMS_PARAMETER *sInfo)
{
//	uint8_t Result;
	UINT32 nOperationRet = ERR_SUCCESS;
	UINT8 nOption        = 0;
	UINT8 nNewSmsStorage = 0;
	CFW_SmsInitComplete(nUTI, SIM_SN);
	nOperationRet = CFW_CfgSetSmsOverflowInd(1, SIM_SN);

	CFW_CfgGetSmsParam(sInfo, 0, SIM_SN);
	sInfo->ssr = (17 & 0x20) >> 5;
	sInfo->dcs = 4;
	nOperationRet = CFW_CfgSetSmsParam(sInfo, 0, SIM_SN);
	nOperationRet = CFW_CfgGetNewSmsOption(&nOption, &nNewSmsStorage, SIM_SN);
	nOption = (nOption & 0x1f) | CFW_SMS_ROUT_DETAIL_INFO;
	nOperationRet = CFW_CfgSetNewSmsOption(nOption, CFW_SMS_STORAGE_ME, SIM_SN);

	CFW_CfgSetSmsFormat(0, SIM_SN);
}

void OS_SMSGetStorageInfo(CFW_SMS_STORAGE_INFO *Info)
{
	CFW_CfgGetSmsStorageInfo(Info, CFW_SMS_STORAGE_ME, SIM_SN);
}

uint32_t OS_SMSTxByPDU(uint8_t *pData, uint16_t nDataSize)
{
	return CFW_SmsSendMessage(NULL, pData, nDataSize, UTI_SMS_SEND, SIM_SN);
}

uint32_t OS_GetHost(int8_t *Name, struct ip_addr *IP)
{
	return CFW_Gethostbyname(Name, IP, CID_IP, SIM_SN);
}

SOCKET OS_CreateSocket(uint8_t nDomain, uint8_t nType, uint8_t nProtocol)
{
	return CFW_TcpipSocket(nDomain, nType, nProtocol);
}

uint32_t OS_SocketConnect(SOCKET SocketID, uint32_t LocalIP, uint16_t LocalPort, uint32_t RemoteIP, uint16_t RemotePort)
{
	CFW_TCPIP_SOCKET_ADDR nDestAddr;
	CFW_TCPIP_SOCKET_ADDR stLocalAddr;
	uint32_t Error;
	memset(&nDestAddr, 0, sizeof(CFW_TCPIP_SOCKET_ADDR));
	memset(&stLocalAddr, 0, sizeof(CFW_TCPIP_SOCKET_ADDR));
	stLocalAddr.sin_family = CFW_TCPIP_AF_INET;
	stLocalAddr.sin_addr.s_addr = LocalIP;
	stLocalAddr.sin_port = htons(LocalPort);
	Error = CFW_TcpipSocketBind(SocketID, &stLocalAddr, sizeof(CFW_TCPIP_SOCKET_ADDR));
	if (Error)
	{
		CORE("OS_SocketConnect %u", CFW_TcpipGetLastError());
		return CFW_TcpipGetLastError();
	}
	if (RemoteIP)
	{
		nDestAddr.sin_family = CFW_TCPIP_AF_INET;
		nDestAddr.sin_addr.s_addr = RemoteIP;
		nDestAddr.sin_port = htons(RemotePort);
		Error = CFW_TcpipSocketConnect(SocketID, &nDestAddr, SIZEOF(CFW_TCPIP_SOCKET_ADDR));
		if (Error)
		{
			CORE("OS_SocketConnect %u", CFW_TcpipGetLastError());
			return CFW_TcpipGetLastError();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}

uint32_t OS_SocketDisconnect(SOCKET SocketID)
{
	return CFW_TcpipSocketClose(SocketID);
}

uint32_t OS_SocketReceive(SOCKET SocketID, uint8_t *Buf, uint32_t Len, CFW_TCPIP_SOCKET_ADDR *from, INT32 *fromlen)
{
	return CFW_TcpipSocketRecvfrom(SocketID, Buf, Len, SIM_SN, from, fromlen);
}

uint32_t OS_SocketSend(SOCKET SocketID, uint8_t *Buf, uint32_t Len, CFW_TCPIP_SOCKET_ADDR *to, INT32 tolen)
{
	if (to)
	{
		return CFW_TcpipSocketSendto(SocketID, Buf, Len, 0, to, tolen);
	}
	else
	{
		return CFW_TcpipSocketSend(SocketID, Buf, Len, 0);
	}
}


/*
函数功能:将UCS编码转化为GB2312编码
*/
uint32_t OS_UCS2ToGB2312(const uint8_t * src, uint8_t * dst,uint32_t srclen, uint8_t IsBigEnding)
{
	uint8_t *Result;
	uint32_t uOutLen;
	if (IsBigEnding)
	{
		ML_Unicode2LocalLanguageBigEnding(src,  srclen, &Result, (UINT32*)&uOutLen,  ML_CP936);
	}
	else
	{
		ML_Unicode2LocalLanguage(src,  srclen, &Result, (UINT32*)&uOutLen,  ML_CP936);
	}
	if (uOutLen)
	{
		memcpy(dst, Result, uOutLen);
		CSW_ML_FREE(Result);
	}
	return uOutLen;
}

/*
 *  将GBK编码的汉字串转换成UCS，但不是以16位的short存储，而是将一个UCS的高底字节分别用一个unsiged char 存储
 *  参数：src为GB2312字符串，以'0'结束
 *  返回的是用来存储UCS的unsiged char 的个数gsmEncodeUcs2(pSrc->TP_UD, &buf[4], nLength)
 */
uint32_t OS_GB2312ToUCS2(const uint8_t* src, uint8_t* dst, uint32_t srclen, uint8_t IsBigEnding)
{
	uint8_t *Result;
	uint32_t uOutLen;
	if (IsBigEnding)
	{
		ML_LocalLanguage2UnicodeBigEnding(src,  srclen, &Result, (UINT32*)&uOutLen,  ML_CP936);
	}
	else
	{
		ML_LocalLanguage2Unicode(src,  srclen, &Result, (UINT32*)&uOutLen,  ML_CP936);
	}
	if (uOutLen)
	{
		memcpy(dst, Result, uOutLen);
		CSW_ML_FREE(Result);
	}
	return uOutLen;
}
