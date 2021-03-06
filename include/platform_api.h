#ifndef __PLATFORM_API_H__
#define __PLATFORM_API_H__
#define __IEEE_LITTLE_ENDIAN
#define CHIP_ASIC_ID_8955					(16)
#ifdef __PLATFORM_8955__
#undef CHIP_ASIC_ID
#define CHIP_ASIC_ID CHIP_ASIC_ID_8955
#endif
//#include <math.h>
#include "string.h"
#include "cs_types.h"
#include "math.h"
#include "csw.h"
#include "cfw_prv.h"
#include "sxs_io.h"
#include "sxr_mem.h"
#include "sxr_tim.h"
#include "event.h"
#include "chip_id.h"
#include "memd_m.h"
#include "pmd_m.h"
#include "pmd_config.h"

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
#include "8809.h"
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
#include "global_macros.h"
#include "globals.h"
#include "bb_cp2.h"
#include "bb_ifc.h"
#include "bb_irq.h"
#include "bb_sram.h"
#include "comregs.h"
#include "bcpu_cache.h"
#include "bcpu_tag.h"
#include "cipher.h"
#include "debug_host.h"
#include "debug_host_internals.h"
#include "debug_uart.h"
#include "dma.h"
#include "gpadc.h"
#include "gouda.h"
#include "mem_bridge.h"
#include "gpio.h"
#include "i2c_master.h"
#include "itlv.h"
#include "page_spy.h"
#include "rf_if.h"
#include "rf_spi.h"
#include "sci.h"
#include "spi.h"
#include "sys_ctrl.h"
#include "sys_ifc.h"
#include "sys_ifc2.h"
#include "sys_irq.h"
#include "tcu.h"
#include "timer.h"
#include "uart.h"
#include "vitac.h"
#include "xcor.h"
#include "cp0.h"
#include "regdef.h"
#include "xcpu_cache.h"
#include "xcpu_tag.h"
#include "xcpu.h"
#include "keypad.h"
#include "pwm.h"
#include "calendar.h"
#include "aif.h"
#include "usbc.h"
#include "sdmmc.h"
#include "camera.h"
#include "cfg_regs.h"
#include "voc_ram.h"
#include "voc_ahb.h"
#include "voc_cfg.h"
#include "memory_burst_adapter.h"
#include "iomux.h"
#endif

#include "hal_tcu.h"
#include "hal_mem_map.h"
#include "hal_pwm.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_sys.h"
#include "hal_uart.h"
#include "hal_timers.h"
#include "hal_spi.h"
#include "sxr_tls.h"
#include "halp_sys_ifc.h"
#include "halp_sys.h"
#include "dm_audio.h"
#include "mci.h"
#include "tcpip_api.h"
#include "unicodeGBTrans.h"

#ifndef EV_MMI_EV_BASE
#define EV_MMI_EV_BASE              0x00100000
#endif

#define SYS_TICK	(16384)
#define FLASH_BASE			(0x88000000)
#define USER_CODE_START		(0x00360000)
#define USER_CODE_LEN		(0x00020000)
#define BACK_CODE_START		(0x00380000)
#define PARAM_START_ADDR	(0x003A0000)
#define PARAM_PID_ADDR		(0x003B0000)
#define BACK_CODE_PARAM		(0x003B4000)
#define TTS_CODE_ADDR		(0x003B5000)
#define LED_CODE_ADDR		(0x003B6000)
#define USER_FLASH_MAX		(0x003C0000)
#define FLASH_SECTOR_LEN	(0x00001000)
#define USER_RAM_LEN		(0x00020000)
#define CRC32_GEN			(0x04C11DB7)

#define CRC32_START		(0xffffffff)
#define CRC16_START		(0xffff)
#define RAM_BASE 		(0x82000000)
#define AT_ASYN_GET_DLCI(x) ((x == 0)?1:5) // x is sim id

#define MMI_TASK_MAX_STACK_SIZE    (4 * 1024)
#define MMI_TASK_MIN_STACK_SIZE    (1024)
#define MMI_TASK_PRIORITY      (COS_MMI_TASKS_PRIORITY_BASE)

#define IMEI_LEN		(10)
#define IMSI_LEN		(10)
#define URL_LEN_MAX		(48)
#define AUTH_CODE_LEN	(48)
#define ICCID_LEN		(ICCID_LENGTH)

#define RDA_UPGRADE_MAGIC_NUM (0x12345678)
#define RDA_UPGRADE_OK		(1)
#define RDA_UPGRADE_FAIL	(2)

#define CC_STATE_NULL       0x00
#define CC_STATE_ACTIVE     0x01
#define CC_STATE_HOLD       0x02
#define CC_STATE_WAITING    0x04
#define CC_STATE_INCOMING   0x08
#define CC_STATE_DIALING    0x10
#define CC_STATE_ALERTLING  0x20
#define CC_STATE_RELEASE    0x40

#define __CUST_NONE__		(0x0000)
#define __CUST_LY__			(0x0001)
#define __CUST_KQ__			(0x0002)
#define __CUST_GLEAD__		(0x0003)
#define __CUST_LY_IOTDEV__	(0x0004)
#define __CUST_LB_V3__		(0x00fd)
#define __CUST_LB_V2__		(0x00fe)
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
#define __CUST_CODE__		__CUST_NONE__
#elif (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
#define __CUST_CODE__		__CUST_GLEAD__
#endif



#if (__CUST_CODE__ == __CUST_KQ__)
#define __TTS_ENABLE__
#endif

//#define __TTS_ENABLE__
//#define __ANT_TEST__
//#define __MINI_SYSTEM__
//#define __GPS_TEST__
//#define __AD_ENABLE__
//#define __G_SENSOR_ENABLE__
#define __G_SENSOR_POWER__
//#define __CRASH_ENABLE__
//#define __UART_485_MODE__
//#define __NO_GPS__
#define __IO_POLL_CHECK__
//#define __LBS_ENABLE__
//#define __COM_SLEEP_BY_VACC__
//#define __COM_SLEEP_BY_STOP__
//#define __LUAT_ENABLE__
//#define __NTP_ENABLE__
//#define __REMOTE_TRACE_ENABLE__
#ifdef __TTS_ENABLE__
#define __BASE_VERSION__	(0x0805)
#else
#define __BASE_VERSION__	(0x0005)
#endif

//针对合宙模块
#define __AIR200__	(1)
#define __AIR201__	(2)
#define __AIR202__	(3)

#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
#define __BOARD__		__AIR201__
//#define __BOARD__		__AIR202__
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
#define __BOARD__		__AIR200__
#endif

typedef struct
{
	uint32_t Flag;
	uint32_t Len;
	uint32_t FileCRC32;
	uint32_t CRC32;		//代码参数区CRC校验
}Upgrade_ParamStruct;

typedef struct
{
    struct netconn *conn;
    struct netbuf *lastdata;
    UINT16 lastoffset;
    UINT16 rcvevent;
    UINT16 sendevent;
    UINT16 flags;
    INT32 err;
}Socket_DescriptStruct;

typedef struct
{
	uint32_t Data[1024];
}Section_DataStruct;

typedef struct
{
	uint32_t MaigcNum;
	uint32_t CRC32;
	uint32_t MainVersion;
	uint32_t AppVersion;
	uint32_t BinFileLen;
}File_HeadStruct;

typedef struct
{
	File_HeadStruct Head;
	Section_DataStruct SectionData[63];
}Upgrade_FileStruct;

void __Trace(const char *Fmt, ...);
void __SetDefaultTaskHandle(HANDLE ID);
void __SetDeepSleep(uint32_t En);
uint8_t *__GetSafeRam(void);
uint32_t __CRC32(void *Src, uint32_t Size, uint32_t CRC32Last);
int32_t __EraseSector(uint32_t Addr);
int32_t __WriteFlash(uint32_t Addr, void *Src, uint32_t Len);
void __ReadFlash(uint32_t Addr, void *Dst, uint32_t Len);
uint8_t __CheckEmpty(uint32_t Addr, uint32_t Len);
void __MainInit(void);
void __Upgrade_Config(void);
void __FileSet(uint32_t Len);
uint8_t __WriteFile(uint8_t *Data, uint32_t Len);
uint32_t __GetMainVersion(void);
void __UpgradeRun(void);
uint8_t __UpgradeVaildCheck(void);
uint8_t __GetUpgradeState(void);
void __ClearUpgradeState(void);
void __TTS_Init(void);
int32_t __TTS_Play(void *Data, uint32_t Len, void *PCMCB, void *TTSCB);
extern PUBLIC UINT32 HAL_BOOT_FUNC_INTERNAL hal_TimGetUpTime(VOID);
extern struct socket_dsc *get_socket(uint8_t nSocket);
#define CORE(X, Y...) __Trace("%s %d:"X, __FUNCTION__, __LINE__, ##Y)
#endif
