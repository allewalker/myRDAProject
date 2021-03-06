#include "user.h"
#if (__CUST_CODE__ == __CUST_KQ__)
#define __KQ_GPRS_APP_VER_H__	(0x01)
#define __KQ_GPRS_APP_VER_L__	(0x00)
#define __KQ_GPRS_CORE_VER_H__	(0x01)
#define __KQ_GPRS_CORE_VER_L__	(0x00)
//#define KQ_TEST_URL "www.bdclw.net"
#define KQ_TEST_URL "123.56.10.103"
#define KQ_TEST_PORT (9990)
//#define KQ_TEST_MODE
//#define FORCE_REG
Monitor_CtrlStruct __attribute__((section (".usr_ram"))) KQCtrl;
extern User_CtrlStruct __attribute__((section (".usr_ram"))) UserCtrl;
extern Upgrade_FileStruct __attribute__((section (".file_ram"))) FileCache;
#define BLE_REBOOT_TIME	(16)
#define VOICE_DEFAULT_CODE_1 "蓝牙已连接"
#define VOICE_DEFAULT_CODE_2 "开锁成功"
#define VOICE_DEFAULT_CODE_3 "关锁成功"
#define VOICE_DEFAULT_CODE_4 "关锁成功，蓝牙未连接，计费结束"
const int8_t JTTDeviceID[7] =
{
		'G','D','T','M','C','Q','1'
};
const int8_t JTTFactoryID[5] =
{
		'0', '0', '0', '0', '3'
};

const int8_t JTTDeviceType[20] =
{
		'0', '0', '0', '0', '3', '0', '0', '0', '0', '0',
		'0', '0', '0', '0', '0', '0', '0', '0', '0', '4',
};

/*
 * KQ3合1锁的串口协议解析
 */

uint8_t KQ_CheckUartHead(uint8_t Data)
{
	switch (Data)
	{
	case KQ_PRO_HEAD_COMM:
	case KQ_PRO_HEAD_EVENT:
	case KQ_PRO_HEAD_SET_IP:
	case KQ_PRO_HEAD_SET_BT:
	case KQ_PRO_HEAD_PLAY:
	case '0':
		return 1;

	default:
		return 0;
	}
}

uint32_t KQ_ComTxPack(uint8_t KQCmd, uint8_t *Data, uint32_t Len, uint8_t *Buf)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	//uint32_t dwTemp;
	//uint16_t wTemp;
	IP_AddrUnion uIP;
	switch (KQCmd)
	{
	case KQ_CMD_SYS_ON:
	case KQ_CMD_SYS_DOWN:
	case KQ_CMD_DOWNLOAD_BT:
	case KQ_CMD_DOWNLOAD_GPRS:
	case KQ_CMD_UPGRADE_GPRS:
		Buf[0] = KQ_PRO_HEAD_COMM;
		Buf[1] = KQCmd;
		return 2;
	case KQ_CMD_SET_TIME_PARAM:
		Buf[0] = KQ_PRO_HEAD_COMM;
		Buf[1] = KQCmd;
		memcpy(Buf + 2, KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_TIME_SN].uData.ucData + 2, 2);
		memcpy(Buf + 4, KQ->ParamItem[KQ_PARAM_UPLOAD_TIME_SN].uData.ucData + 2, 2);

		return 6;
	case KQ_CMD_SET_IP:
		Buf[0] = KQ_PRO_HEAD_SET_IP;
		uIP.u32_addr = inet_addr(KQ->ParamItem[KQ_PARAM_IP_SN].uData.pData);
		Buf[1] = 0;
		Buf[2] = 0;
		memcpy(Buf + 3, uIP.u8_addr, 4);
		memcpy(Buf + 7, KQ->ParamItem[KQ_PARAM_PORT_SN].uData.ucData + 2, 2);
		return 9;
	case KQ_CMD_SET_BT:
		Buf[0] = KQ_PRO_HEAD_SET_BT;
		Buf[1] = KQ->BLECmd;
		Buf[2] = KQ->BLECmdLen;
		memcpy(Buf + 3, KQ->BLECmdData, KQ->BLECmdLen);
		return 3 + KQ->BLECmdLen;
	}
	return 0;
}

uint8_t KQ_BleEventAnalyze(uint8_t *Data, uint8_t MaxLen)
{
	uint8_t Pos = 0;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	if (Data[Pos] == KQ_BLE_LOCK_OPEN_TIMES)
	{
		KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].ID = KQ_JTT_LOCK_OPEN_TIMES + JTT_CUST_ITEM_START;
		KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].Len = 4;
		memcpy(KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].uData.ucData, &Data[Pos + 1], KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].Len);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].uData.ucData, KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].Len);
		Pos += 1 + KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].Len;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_LOCK_STATE)
	{
		KQ->CustItem[KQ_JTT_LOCK_STATE].ID = KQ_JTT_LOCK_STATE + JTT_CUST_ITEM_START;
		KQ->CustItem[KQ_JTT_LOCK_STATE].Len = 2;
		memcpy(KQ->CustItem[KQ_JTT_LOCK_STATE].uData.ucData, &Data[Pos + 1], KQ->CustItem[KQ_JTT_LOCK_STATE].Len);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->CustItem[KQ_JTT_LOCK_OPEN_TIMES].uData.ucData, KQ->CustItem[KQ_JTT_LOCK_STATE].Len);
		Pos += 1 + KQ->CustItem[KQ_JTT_LOCK_STATE].Len;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_VBAT_STATE)
	{
		KQ->CustItem[KQ_JTT_VBAT_STATE].ID = KQ_JTT_VBAT_STATE + JTT_CUST_ITEM_START;
		KQ->CustItem[KQ_JTT_VBAT_STATE].Len = 6;
		KQ->CustItem[KQ_JTT_VBAT_STATE].uData.pData = COS_MALLOC(KQ->CustItem[KQ_JTT_VBAT_STATE].Len);
		memcpy(KQ->CustItem[KQ_JTT_VBAT_STATE].uData.pData, &Data[Pos + 1], KQ->CustItem[KQ_JTT_VBAT_STATE].Len);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->CustItem[KQ_JTT_VBAT_STATE].uData.pData, KQ->CustItem[KQ_JTT_VBAT_STATE].Len);
		Pos += 1 + KQ->CustItem[KQ_JTT_VBAT_STATE].Len;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_UPOLOAD_INFO)
	{

		memcpy(KQ->UploadInfo, &Data[Pos + 1], 4);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->UploadInfo, 4);
		Pos += 5;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_VERSION)
	{
		KQ->CustItem[KQ_JTT_BLE_VERSION].ID = KQ_JTT_BLE_VERSION + JTT_CUST_ITEM_START;
		KQ->CustItem[KQ_JTT_BLE_VERSION].Len = 2;
		memcpy(KQ->CustItem[KQ_JTT_BLE_VERSION].uData.ucData, &Data[Pos + 1], KQ->CustItem[KQ_JTT_BLE_VERSION].Len);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->CustItem[KQ_JTT_BLE_VERSION].uData.ucData, KQ->CustItem[KQ_JTT_BLE_VERSION].Len);
		Pos += 1 + KQ->CustItem[KQ_JTT_BLE_VERSION].Len;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_NO_CHARGE_TIME)
	{
		KQ->CustItem[KQ_JTT_NO_CHARGE_TIME].ID = KQ_JTT_NO_CHARGE_TIME + JTT_CUST_ITEM_START;
		KQ->CustItem[KQ_JTT_NO_CHARGE_TIME].Len = 2;
		memcpy(KQ->CustItem[KQ_JTT_NO_CHARGE_TIME].uData.ucData, &Data[Pos + 1], KQ->CustItem[KQ_JTT_NO_CHARGE_TIME].Len);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->CustItem[KQ_JTT_NO_CHARGE_TIME].uData.ucData, KQ->CustItem[KQ_JTT_NO_CHARGE_TIME].Len);
		Pos += 1 + KQ->CustItem[KQ_JTT_NO_CHARGE_TIME].Len;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_IP_INFO)
	{
		memcpy(KQ->IP, &Data[Pos + 1], 6);
		KQ->Port = Data[Pos + 7];
		KQ->Port = (KQ->Port << 8) + Data[Pos + 8];
		DBG("kq ble %02X %u:", Data[Pos], KQ->Port);
		DecTrace(KQ->IP, 6);
		Pos += 9;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_MAC)
	{
		memcpy(KQ->Mac, &Data[Pos + 1], 6);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->Mac, 6);
		Pos += 7;
	}

	if (Pos >= MaxLen)
	{
		return Pos;
	}

	if (Data[Pos] == KQ_BLE_CTRL_INFO)
	{
		memcpy(KQ->CtrlInfo, &Data[Pos + 1], 2);
		DBG("kq ble %02X:", Data[Pos]);
		HexTrace(KQ->CtrlInfo, 2);
		Pos += 3;
	}
	return Pos;
}

uint32_t KQ_BLEReport(uint8_t *TxBuf, uint32_t TxBufLen)
{

	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t Temp[20], i, j;
	uint32_t TxLen = 0;
	TxBuf[TxLen++] = '*';
	memcpy(&TxBuf[TxLen], KQ->BLEAddr, 12);
	TxLen += 12;
	TxBuf[TxLen++] = (gSys.State[SIM_STATE]?0x31:0x30);
	TxBuf[TxLen++] = (0x30 + gSys.State[RSSI_STATE]/10);
	TxBuf[TxLen++] = (0x30 + gSys.State[RSSI_STATE]%10);
	if (gSys.Error[GPS_ERROR])
	{
		TxBuf[TxLen++] = '0';
	}
	else
	{
		TxBuf[TxLen++] = '1';
	}

	j = 0;
	for (i = 0; i < 8; i++)
	{
		Temp[j] = ((gSys.IMEI[i] & 0xf0) >> 4) + '0';
		Temp[j + 1] = ((gSys.IMEI[i] & 0x0f) >> 0) + '0';
		j += 2;
	}
	memcpy(&TxBuf[TxLen], Temp + 1,15);
	TxLen += 15;

	j = 0;
	for (i = 0; i < 10; i++)
	{
		Temp[j] = ((gSys.ICCID[i] & 0xf0) >> 4) + '0';
		Temp[j + 1] = ((gSys.ICCID[i] & 0x0f) >> 0) + '0';
		j += 2;
	}
	memcpy(&TxBuf[TxLen], Temp, 20);
	TxLen += 20;
	TxBuf[TxLen++] = '#';
	TxBuf[TxLen++] = 0x00;
	DBG("%s", TxBuf);
	KQ_StartTTSCode(1, 1, 256);
	return TxLen;
}

uint32_t KQ_ComAnalyze(uint8_t *RxBuf, uint32_t RxLen, uint8_t *TxBuf, uint32_t TxBufLen, int32_t *Result)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t Head = 0;
	uint8_t Time, Code, Len;
	uint32_t Pos = 0, TxLen;
	//uint8_t *Start;
	DBG("Rx:%u", RxLen);
	HexTrace(RxBuf, RxLen);
	TxLen = 0;
	*Result = 0;
	if (RxBuf[0] == '0')
	{
		if (RxLen >= 14)
		{
			GPIO_Write(WDG_PIN, 1);
			switch (KQ->BLEReportFlag)
			{
			case 0:
				memcpy(KQ->BLEAddr, &RxBuf[3], 12);
				KQ->BLEReportFlag = 1;
				//重启蓝牙
				GPIO_Write(BLE_REBOOT_H_PIN, 1);
				GPIO_Write(BLE_REBOOT_L_PIN, 0);
				OS_Sleep(SYS_TICK/BLE_REBOOT_TIME);
				GPIO_Write(BLE_REBOOT_H_PIN, 0);
				GPIO_Write(BLE_REBOOT_L_PIN, 1);
				OS_Sleep(SYS_TICK/BLE_REBOOT_TIME);
				break;
			case 1:
				KQ->BLEReportFlag = 2;
				TxLen = KQ_BLEReport(TxBuf, TxBufLen);
				break;
			case 2:
				TxLen = KQ_BLEReport(TxBuf, TxBufLen);
				break;
			}

		}
		return TxLen;
	}
	while (Pos < RxLen)
	{
		if (!Head)
		{
			switch(RxBuf[Pos])
			{
			case KQ_PRO_HEAD_COMM:

			case KQ_PRO_HEAD_EVENT:
			case KQ_PRO_HEAD_SET_IP:
			case KQ_PRO_HEAD_SET_BT:
			case KQ_PRO_HEAD_PLAY:
				Head = RxBuf[Pos];
				break;
			}
			Pos++;
		}
		else
		{
			switch (Head)
			{
			case KQ_PRO_HEAD_COMM:
				DBG("ble %02x com ok", RxBuf[Pos]);
				switch (RxBuf[Pos])
				{

				case KQ_CMD_DOWNLOAD_BT:
					if (!UserCtrl.DevUpgradeFlag)
					{
						DBG("Start upgrade ble");
						gSys.Var[SHUTDOWN_TIME] = 600;
						OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 600 * SYS_TICK);
						GPIO_Write(WDG_PIN, 1);
						FTP_StartCmd(KQ->FTPCmd + 4, (uint8_t *)&FileCache);
						UserCtrl.DevUpgradeFlag = 1;
						KQ->BLEUpgradeStart = 0;
					}
					break;
				case KQ_CMD_DOWNLOAD_GPRS:
					if (!UserCtrl.GPRSUpgradeFlag)
					{
						DBG("Start upgrade gprs");
						gSys.Var[SHUTDOWN_TIME] = 600;
						OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, 600 * SYS_TICK);
						GPIO_Write(WDG_PIN, 1);
						FTP_StartCmd(KQ->FTPCmd + 4, (uint8_t *)&FileCache);
						UserCtrl.GPRSUpgradeFlag = 1;
					}
					break;

				case KQ_CMD_SET_TIME_PARAM:
					break;
				}
				Pos += 2;
				Head = 0;
				break;
			case KQ_PRO_HEAD_EVENT:
				Pos += KQ_BleEventAnalyze(&RxBuf[Pos], (RxLen - Pos));
				if ((TxLen + 2) <= TxBufLen)
				{
					TxBuf[TxLen] = Head;
					TxBuf[TxLen + 1] = KQ_PRO_END;
					TxLen += 2;
				}
				Head = 0;
				Monitor_RecordData();
				break;
			case KQ_PRO_HEAD_SET_IP:
				Pos++;
				Head = 0;
				break;
			case KQ_PRO_HEAD_SET_BT:
				HexTrace(&RxBuf[Pos], 3);
				if (RxBuf[Pos + 1])
				{
					*Result = 1;

				}
				Pos += 3;
				Head = 0;
				break;
			case KQ_PRO_HEAD_PLAY:
				Time = RxBuf[Pos];
				Code = RxBuf[Pos + 1];
				DBG("%02x, %02x", Time, Code);
				if (Time)
				{
					if (!Code)
					{
						Len = RxBuf[Pos + 2];
						UserCtrl.TTSCodeData[0].Len = Len;
						if (Len <= sizeof(UserCtrl.TTSCodeData[0].Data))
						{
							memcpy(UserCtrl.TTSCodeData[0].Data, &RxBuf[Pos + 3], UserCtrl.TTSCodeData[0].Len);
						}
						UserCtrl.TTSCodeData[0].Repeat = Time;
						UserCtrl.TTSCodeData[0].Interval = 1;
						Pos += 3 + Len;
					}
					else
					{
						Pos += 2;
					}
					KQ_StartTTSCode(Code, Time, 256);
				}

				if ((TxLen + 3) <= TxBufLen)
				{
					TxBuf[TxLen] = Head;
					TxBuf[TxLen + 1] = Code;
					TxBuf[TxLen + 2] = KQ_PRO_END;
					TxLen += 3;
				}
				Head = 0;
				break;
			}
		}

	}
	return TxLen;
}

void KQ_TTSInit(void)
{
	uint32_t i;
	uint32_t ErrorBlock;	//错误的区块
	uint32_t BlankBlock;	//空的区块
	uint8_t Temp[64];
	uint16_t wTemp[64];
	uint16_t Len;
	TTS_CodeSaveStruct TTSSave;
	memset(Temp, 0xff, 64);
	for(i = 0; i < TTS_CODE_MAX; i++)
	{
		UserCtrl.TTSCodeData[i].Repeat = 1;
		UserCtrl.TTSCodeData[i].Interval = 1;
	}
	UserCtrl.TTSCodeData[0].Len = 0;
	memset(wTemp, 0, sizeof(wTemp));
	Len = OS_GB2312ToUCS2(VOICE_DEFAULT_CODE_1, (uint8_t *)wTemp, strlen(VOICE_DEFAULT_CODE_1), 0);
	UserCtrl.TTSCodeData[1].Len = Len;
	memcpy(UserCtrl.TTSCodeData[1].Data, wTemp, Len);

	Len = OS_GB2312ToUCS2(VOICE_DEFAULT_CODE_2, (uint8_t *)wTemp, strlen(VOICE_DEFAULT_CODE_2), 0);
	UserCtrl.TTSCodeData[2].Len = Len;
	memcpy(UserCtrl.TTSCodeData[2].Data, wTemp, Len);

	Len = OS_GB2312ToUCS2(VOICE_DEFAULT_CODE_3, (uint8_t *)wTemp, strlen(VOICE_DEFAULT_CODE_3), 0);
	UserCtrl.TTSCodeData[3].Len = Len;
	memcpy(UserCtrl.TTSCodeData[3].Data, wTemp, Len);

	Len = OS_GB2312ToUCS2(VOICE_DEFAULT_CODE_4, (uint8_t *)wTemp, strlen(VOICE_DEFAULT_CODE_4), 0);
	UserCtrl.TTSCodeData[4].Len = Len;
	memcpy(UserCtrl.TTSCodeData[4].Data, wTemp, Len);

	ErrorBlock = 0;
	BlankBlock = 0;
	for (i = 0; i < (4096)/sizeof(TTS_CodeSaveStruct); i++)
	{
		__ReadFlash(TTS_CODE_ADDR + i * sizeof(TTS_CodeSaveStruct), (uint8_t *)&TTSSave, sizeof(TTS_CodeSaveStruct));
		if (memcmp((uint8_t *)&TTSSave, Temp, sizeof(TTS_CodeSaveStruct)))
		{
			if (TTSSave.MagicNum != KQ_TTS_MAGIC_NUM)
			{
				ErrorBlock = i;
				break;
			}
			if ( (TTSSave.Code < TTS_CODE_START) && (TTSSave.Code >= TTS_CODE_MAX) )
			{
				ErrorBlock = i;
				break;
			}
			if (TTSSave.CRC16 != CRC16Cal((uint8_t *)&TTSSave, sizeof(TTS_CodeSaveStruct) - 2, CRC16_START, CRC16_CCITT_GEN, 0))
			{
				ErrorBlock = i;
				break;
			}
		}
		else
		{
			BlankBlock = i;
			break;
		}
		memcpy((uint8_t *)&UserCtrl.TTSCodeData[TTSSave.Code], TTSSave.uTTSData.Pad, sizeof(TTS_CodeDataUnion));
	}

	UserCtrl.VoiceCode = 0xff;

	if ( (BlankBlock != (TTS_CODE_MAX - 1) ) || (ErrorBlock))
	{
		DBG("%u %u", BlankBlock, ErrorBlock);
		__EraseSector(TTS_CODE_ADDR);
		for(i = TTS_CODE_START; i < TTS_CODE_MAX; i++)
		{
			TTSSave.Code = i;
			TTSSave.MagicNum = KQ_TTS_MAGIC_NUM;
			memcpy(TTSSave.uTTSData.Pad, (uint8_t *)&UserCtrl.TTSCodeData[i], sizeof(TTS_CodeDataUnion));
			TTSSave.CRC16 = CRC16Cal((uint8_t *)&TTSSave, sizeof(TTS_CodeSaveStruct) - 2, CRC16_START, CRC16_CCITT_GEN, 0);
			__WriteFlash(TTS_CODE_ADDR + (i - 1) * sizeof(TTS_CodeSaveStruct), (uint8_t *)&TTSSave, sizeof(TTS_CodeSaveStruct));
		}
	}
}

void KQ_StartTTSCode(uint8_t Code, uint8_t Time, uint32_t Delay)
{
	if (Code < TTS_CODE_MAX)
	{
		UserCtrl.PlayTime = 0;
		UserCtrl.VoiceCode = Code;
		if (!Code)
		{
			UserCtrl.TTSCodeData[0].Repeat = Time;
		}
		DBG("ready to play code %u repeat %u", (uint32_t)UserCtrl.VoiceCode, (uint32_t)UserCtrl.TTSCodeData[UserCtrl.VoiceCode].Repeat);
	}

	DBG("start new tts");
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], TTS_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK / Delay);

}

TTS_CodeDataStruct *KQ_GetTTSCodeData(void)
{
	return &UserCtrl.TTSCodeData[0];
}

int32_t KQ_SaveTTSCode(TTS_CodeDataStruct *TTSCodeData, uint8_t Code)
{
	TTS_CodeSaveStruct TTSSave;
	uint8_t Temp[64];
	uint32_t i;
	uint32_t BlankBlock = 0;
	memset(Temp, 0xff, 64);
	for (i = 0; i < (4096)/sizeof(TTS_CodeSaveStruct); i++)
	{
		__ReadFlash(TTS_CODE_ADDR + i * sizeof(TTS_CodeSaveStruct), (uint8_t *)&TTSSave, sizeof(TTS_CodeSaveStruct));
		if (!memcmp((uint8_t *)&TTSSave, Temp, sizeof(TTS_CodeSaveStruct)))
		{
			BlankBlock = i;
			break;
		}
	}

	DBG("block %u blank!", BlankBlock);
	if (Code < TTS_CODE_MAX)
	{
		memcpy((uint8_t *)&UserCtrl.TTSCodeData[Code], (uint8_t *)TTSCodeData, sizeof(TTS_CodeDataStruct));
	}

	if ( (Code >= TTS_CODE_START) && (Code < TTS_CODE_MAX) )
	{
		TTSSave.Code = Code;
		TTSSave.MagicNum = KQ_TTS_MAGIC_NUM;
		memcpy(TTSSave.uTTSData.Pad, (uint8_t *)TTSCodeData, sizeof(TTS_CodeDataUnion));
		TTSSave.CRC16 = CRC16Cal((uint8_t *)&TTSSave, sizeof(TTS_CodeSaveStruct) - 2, CRC16_START, CRC16_CCITT_GEN, 0);
		__WriteFlash(TTS_CODE_ADDR + BlankBlock * sizeof(TTS_CodeSaveStruct), (uint8_t *)&TTSSave, sizeof(TTS_CodeSaveStruct));
	}
	return 0;
}

void KQ_LEDInit(void)
{
#if 0
	uint32_t i;
	uint32_t ErrorBlock;	//错误的区块
	uint32_t BlankBlock;	//空的区块
	uint8_t Temp[64];

	LED_CodeSaveStruct LEDSave;
	memset(Temp, 0xff, 64);
	for(i = 0; i < LED_CODE_MAX; i++)
	{
		UserCtrl.LEDCodeData[i].Color = 0;
	}

	ErrorBlock = 0;
	BlankBlock = 0;
	for (i = 0; i < (4096)/sizeof(LED_CodeSaveStruct); i++)
	{
		__ReadFlash(LED_CODE_ADDR + i * sizeof(LED_CodeSaveStruct), (uint8_t *)&LEDSave, sizeof(LED_CodeSaveStruct));
		if (memcmp((uint8_t *)&LEDSave, Temp, sizeof(LED_CodeSaveStruct)))
		{
			if (LEDSave.MagicNum != KQ_LED_MAGIC_NUM)
			{
				ErrorBlock = i;
				break;
			}
			if ( (LEDSave.Code < LED_CODE_START) && (LEDSave.Code >= LED_CODE_MAX) )
			{
				ErrorBlock = i;
				break;
			}
			if (LEDSave.CRC16 != CRC16Cal((uint8_t *)&LEDSave, sizeof(LED_CodeSaveStruct) - 2, CRC16_START, CRC16_GEN))
			{
				ErrorBlock = i;
				break;
			}
		}
		else
		{
			BlankBlock = i;
			break;
		}
		memcpy((uint8_t *)&UserCtrl.LEDCodeData[LEDSave.Code], (uint8_t *)&LEDSave.uLEDData.LEDData, sizeof(LED_CodeDataUnion));
	}

	UserCtrl.LEDCode = 0xff;

	if ( (BlankBlock != (LED_CODE_MAX - 1) ) || (ErrorBlock))
	{
		DBG("%u %u", BlankBlock, ErrorBlock);
		__EraseSector(LED_CODE_ADDR);
		for(i = LED_CODE_START; i < LED_CODE_MAX; i++)
		{
			LEDSave.Code = i;
			LEDSave.MagicNum = KQ_LED_MAGIC_NUM;
			memcpy(LEDSave.uLEDData.Pad, (uint8_t *)&UserCtrl.LEDCodeData[LEDSave.Code], sizeof(LED_CodeDataUnion));
			LEDSave.CRC16 = CRC16Cal((uint8_t *)&LEDSave, sizeof(LED_CodeSaveStruct) - 2, CRC16_START, CRC16_GEN);
			__WriteFlash(LED_CODE_ADDR + (i - 1) * sizeof(LED_CodeSaveStruct), (uint8_t *)&LEDSave, sizeof(LED_CodeSaveStruct));
		}
	}
#endif
}

void KQ_StartLEDCode(uint8_t Code)
{

}

LED_CodeDataStruct *KQ_GetLEDCodeData(void)
{
	return NULL;
}

int32_t KQ_SaveLEDCode(LED_CodeDataStruct *LEDCodeData, uint8_t Code)
{
//	LED_CodeSaveStruct LEDSave;
//	uint8_t Temp[64];
//	uint32_t i;
//	uint32_t BlankBlock = 0;
//	memset(Temp, 0xff, 64);
//	for (i = 0; i < (4096)/sizeof(LED_CodeSaveStruct); i++)
//	{
//		__ReadFlash(LED_CODE_ADDR + i * sizeof(LED_CodeSaveStruct), (uint8_t *)&LEDSave, sizeof(LED_CodeSaveStruct));
//		if (!memcmp((uint8_t *)&LEDSave, Temp, sizeof(LED_CodeSaveStruct)))
//		{
//			BlankBlock = i;
//			break;
//		}
//	}
//
//	DBG("block %u blank!", BlankBlock);
//	if ( (Code >= LED_CODE_START) && (Code < LED_CODE_MAX) )
//	{
//		memcpy((uint8_t *)&UserCtrl.LEDCodeData[Code], (uint8_t *)LEDCodeData, sizeof(LED_CodeDataStruct));
//		LEDSave.Code = Code;
//		LEDSave.MagicNum = KQ_LED_MAGIC_NUM;
//		memcpy(LEDSave.uLEDData.Pad, (uint8_t *)LEDCodeData, sizeof(LED_CodeDataUnion));
//		LEDSave.CRC16 = CRC16Cal((uint8_t *)&LEDSave, sizeof(LED_CodeSaveStruct) - 2, CRC16_START, CRC16_GEN);
//		__WriteFlash(LED_CODE_ADDR + BlankBlock * sizeof(LED_CodeSaveStruct), (uint8_t *)&LEDSave, sizeof(LED_CodeSaveStruct));
//	}
	return 0;
}



void KQ_TTSParamDownload(uint8_t *Data, uint8_t Len)
{
	uint8_t TTSPackNum = Data[0];
	uint8_t TTSPackPos = 0;
	uint8_t Pos = 1;
	uint8_t Code;
	TTS_CodeDataStruct TTSCodeData;
	DBG("all tts pack %u", TTSPackNum);
	while( ((Pos + 4) < Len) && (TTSPackPos < TTSPackNum) )
	{
		DBG("analyze %u", TTSPackPos + 1);
		Code = Data[Pos++];
		TTSCodeData.Repeat = Data[Pos++];
		TTSCodeData.Interval = Data[Pos++];
		TTSCodeData.Len = Data[Pos++];
		if (TTSCodeData.Len > KQ_TTS_LEN_MAX)
		{
			break;
		}
		if ((Pos + TTSCodeData.Len) <= Len)
		{
			memcpy(TTSCodeData.Data, &Data[Pos], TTSCodeData.Len);
		}
		Pos += TTSCodeData.Len;
		DBG("save code %u R %u I %u L %u", Code, TTSCodeData.Repeat, TTSCodeData.Interval, TTSCodeData.Len);
		KQ_SaveTTSCode(&TTSCodeData, Code);
		if (!Code)
		{
			KQ_StartTTSCode(0, TTSCodeData.Repeat, 1);
		}
	}
}

uint32_t KQ_TTSParamUpload(uint8_t *Buf)
{
	TTS_CodeDataStruct *CodeData = KQ_GetTTSCodeData();
	uint8_t i;
	uint32_t Pos = 1;
	Buf[0] = 0;
	for (i = 0; i < TTS_CODE_MAX; i++)
	{
		if (i && !CodeData[i].Len)
		{
			break;
		}
		Buf[Pos++] = i;
		Buf[Pos++] = CodeData[i].Repeat;
		Buf[Pos++] = CodeData[i].Interval;
		Buf[Pos++] = CodeData[i].Len;
		memcpy(&Buf[Pos], CodeData[i].Data, CodeData[i].Len);
		Pos += CodeData[i].Len;
		Buf[0]++;
	}
	return Pos;
}

void KQ_LEDParamDownload(uint8_t *Data, uint8_t Len)
{
//	uint8_t Code;
//	LED_CodeDataStruct LEDCodeData;
//	uint8_t Pos = 0;
//
//	if (Len % 4)
//	{
//		return ;
//	}
//
//	while( Pos < Len )
//	{
//		Code = Data[Pos++];
//		LEDCodeData.Color = Data[Pos++];
//		LEDCodeData.FlushTime = Data[Pos++];
//		LEDCodeData.KeepTime = Data[Pos++];
//		LEDCodeData.Pad = 0;
//		DBG("save code %u C %u F %u K %u", Code, LEDCodeData.Color, LEDCodeData.FlushTime, LEDCodeData.KeepTime);
//		KQ_SaveLEDCode(&LEDCodeData, Code);
//		if (!Code)
//		{
//			KQ_StartLEDCode(0);
//		}
//	}
	//缓存起来，直接下发给蓝牙
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	KQ->BLECmd = KQ_DT_SET_LED;
//	KQ->BLECmdLen = Len - 1;
//	memcpy(KQ->BLECmdData, Data + 1, KQ->BLECmdLen);
	KQ->BLECmdLen = Len;
	memcpy(KQ->BLECmdData, Data, KQ->BLECmdLen);
	KQ->WaitFlag = 1;
	DBG("%u", KQ->BLECmd);
	HexTrace(KQ->BLECmdData, KQ->BLECmdLen);
	User_Req(KQ_CMD_SET_BT, 0, 0);
}

uint32_t KQ_LEDParamUpload(uint8_t *Buf)
{
//	LED_CodeDataStruct *CodeData = KQ_GetLEDCodeData();
//	uint8_t i;
//	uint32_t Pos = 0;
//	Buf[Pos++] = LED_CODE_MAX - 1;
//	for (i = LED_CODE_START; i < LED_CODE_MAX; i++)
//	{
//		Buf[Pos++] = i;
//		Buf[Pos++] = CodeData[i].Color;
//		Buf[Pos++] = CodeData[i].FlushTime;
//		Buf[Pos++] = CodeData[i].KeepTime;
//	}
//	return Pos;

	return 0;
}
/*
 * 酷骑3合1锁，平台流程
 */

/*
 * 发送相关API
 */
uint32_t KQ_JTTRegTx(uint8_t *Buf)
{
	uint8_t pBuf[128];
	uint8_t CarCode[28];
	uint32_t TxLen;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	memcpy(CarCode, gSys.ICCID, 10);
	sprintf(CarCode + 10, "%02x:%02x:%02x:%02x:%02x:%02x", KQ->Mac[0], KQ->Mac[1], KQ->Mac[2],
			KQ->Mac[3], KQ->Mac[4], KQ->Mac[5]);
	KQ->LastTxMsgSn++;
	TxLen = JTT_RegMsgBoby(0, 0, JTTFactoryID, JTTDeviceType, JTTDeviceID, KQ_JTT_COLOR_GPRS_BLE,
			CarCode, 27, pBuf + JTT_PACK_HEAD_LEN);

	JTT_PacketHead(JTT_TX_REG, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen, 0, pBuf);
	KQ->LastTxMsgID = JTT_TX_REG;
	pBuf[TxLen + JTT_PACK_HEAD_LEN] = XorCheck(pBuf, TxLen + JTT_PACK_HEAD_LEN, 0);
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + JTT_PACK_HEAD_LEN + 1, Buf);
	return TxLen;
}

uint32_t KQ_JTTUnRegTx(uint8_t *Buf)
{
	uint8_t pBuf[128];
	uint32_t TxLen;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	KQ->LastTxMsgSn++;
	JTT_PacketHead(JTT_TX_UNREG, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, 0, 0, pBuf);
	TxLen = JTT_PACK_HEAD_LEN;
	pBuf[TxLen] = XorCheck(pBuf, TxLen, 0);
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, JTT_PACK_HEAD_LEN + 1, Buf);
	return TxLen;
}

uint32_t KQ_JTTAuthTx(uint8_t *Buf)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t pBuf[128];
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint32_t TxLen;
	KQ->LastTxMsgSn++;
	JTT_PacketHead(JTT_TX_AUTH, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, KQ->AuthCodeLen, 0, pBuf);
	memcpy(pBuf + JTT_PACK_HEAD_LEN, KQ->AuthCode, KQ->AuthCodeLen);
	KQ->LastTxMsgID = JTT_TX_AUTH;
	pBuf[KQ->AuthCodeLen + JTT_PACK_HEAD_LEN] = XorCheck(pBuf, KQ->AuthCodeLen + JTT_PACK_HEAD_LEN, 0);
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, KQ->AuthCodeLen + JTT_PACK_HEAD_LEN + 1, Buf);
	return TxLen;
}

uint32_t KQ_JTTLocatInfoTx(uint8_t *Buf, Monitor_RecordStruct *Record)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t *pBuf = KQCtrl.TempBuf;
	uint8_t Temp[32], i, j;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint32_t Pos;
	uint32_t TxLen, AddLen;
	uint16_t wTemp;
	KQ->LastTxMsgSn++;
	Record->IOValUnion.IOVal.ACC = 1;
	TxLen = JTT_LocatBaseInfoMsgBoby(Record, pBuf + JTT_PACK_HEAD_LEN);

	Pos = TxLen + JTT_PACK_HEAD_LEN;
	AddLen = JTT_AddLocatMsgBoby(JTT_LOCAT_ADD_ITEM_CSQ, 1, &gSys.State[RSSI_STATE], pBuf + Pos);
	Pos += AddLen;

	Temp[0] = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];
	AddLen = JTT_AddLocatMsgBoby(JTT_LOCAT_ADD_ITEM_GNSS_NUM, 1, &Temp[0], pBuf + Pos);
	Pos += AddLen;

	memset(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData, 0, KQ->CustItem[KQ_JTT_GSM_INFO].Len);

	wTemp = BCDToInt(gSys.IMSI, 2);
	wTemp = htons(wTemp);
	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData, &wTemp, 2);

	wTemp = BCDToInt(gSys.IMSI + 2, 1);
	wTemp = htons(wTemp);
	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 2, &wTemp, 2);

	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 4, gSys.CurrentCell.nTSM_LAI + 3, 2);
	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 8, gSys.CurrentCell.nTSM_CellID, 2);
	KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData[10] = RssiToCSQ(gSys.CurrentCell.nTSM_AvRxLevel);
	j = 0;
	for (i = 0; i < gSys.NearbyCell.nTSM_NebCellNUM; i++)
	{
		if (gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0])
		{
			memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 11 + j * 7, gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI + 3, 2);
			memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 15 + j * 7, gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID, 2);
			KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData[17 + j * 7] = RssiToCSQ(gSys.NearbyCell.nTSM_NebCell[i].nTSM_AvRxLevel);
			j++;
			if (j >= 2)
				break;
		}
	}


	for (i = 0; i < KQ_JTT_CUST_ITEM_MAX; i++)
	{
		if (KQ->CustItem[i].Len > 4)
		{
			AddLen = JTT_AddLocatMsgBoby(KQ->CustItem[i].ID, KQ->CustItem[i].Len, KQ->CustItem[i].uData.pData, pBuf + Pos);
		}
		else
		{
			AddLen = JTT_AddLocatMsgBoby(KQ->CustItem[i].ID, KQ->CustItem[i].Len, KQ->CustItem[i].uData.ucData, pBuf + Pos);
		}
		Pos += AddLen;
	}
	TxLen = Pos;

	JTT_PacketHead(JTT_TX_UPLOAD, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen - JTT_PACK_HEAD_LEN, 0, pBuf);
	KQ->LastTxMsgID = JTT_TX_UPLOAD;
	pBuf[TxLen] = XorCheck(pBuf, TxLen, 0);
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + 1, Buf);
	return TxLen;
}

uint32_t KQ_JTTHeartTx(uint8_t *Buf)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t pBuf[32];
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint32_t TxLen;
	KQ->LastTxMsgSn++;
	JTT_PacketHead(JTT_TX_HEART, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, 0, 0, pBuf);
	TxLen = JTT_PACK_HEAD_LEN;
	pBuf[TxLen] = XorCheck(pBuf, TxLen, 0);
	KQ->LastTxMsgID = JTT_TX_HEART;
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, JTT_PACK_HEAD_LEN + 1, Buf);
	return 0;
}

uint32_t KQ_JTTDevResTx(uint8_t Result, uint8_t *Buf)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t pBuf[64];
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint32_t Pos;
	uint32_t TxLen;
	KQ->LastTxMsgSn++;
	TxLen = JTT_DevResMsgBoby(KQ->LastRxMsgID, KQ->LastRxMsgSn, Result, pBuf + JTT_PACK_HEAD_LEN);
	Pos = JTT_PacketHead(JTT_TX_RESPONSE, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen, 0, pBuf);
	pBuf[TxLen + JTT_PACK_HEAD_LEN] = XorCheck(pBuf, TxLen + JTT_PACK_HEAD_LEN, 0);
	KQ->LastTxMsgID = JTT_TX_RESPONSE;
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + JTT_PACK_HEAD_LEN + 1, Buf);

	return TxLen;
}

uint32_t KQ_JTTParamTx(uint8_t *Buf)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t *pBuf = KQCtrl.TempBuf;
	uint8_t i, All;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint32_t Pos;
	uint32_t TxLen, dwTemp;
	KQ->LastTxMsgSn++;
	All = 0;
	for (i = 0; i < KQ_PARAM_MAX; i++)
	{
		if (KQ->ParamUpload[i])
		{
			All++;
		}
	}
	TxLen = JTT_ParamMsgBoby(KQ->LastRxMsgSn, All, pBuf + JTT_PACK_HEAD_LEN);
	Pos = TxLen + JTT_PACK_HEAD_LEN;
	KQ->ParamItem[KQ_PARAM_VOICE_SN].Len = KQ_TTSParamUpload(KQ->ParamItem[KQ_PARAM_VOICE_SN].uData.pData);
	KQ->ParamItem[KQ_PARAM_LED_SN].Len = KQ_LEDParamUpload(KQ->ParamItem[KQ_PARAM_LED_SN].uData.pData);
	for (i = 0; i < KQ_PARAM_MAX; i++)
	{
		if (KQ->ParamUpload[i])
		{
			dwTemp = htonl(KQ->ParamItem[i].ID);
			memcpy(pBuf + Pos, &dwTemp, 4);
			Pos += 4;
			pBuf[Pos++] = KQ->ParamItem[i].Len;
			if (KQ->ParamItem[i].Len > 4)
			{
				memcpy(pBuf + Pos, KQ->ParamItem[i].uData.pData, KQ->ParamItem[i].Len);
			}
			else
			{
				memcpy(pBuf + Pos, KQ->ParamItem[i].uData.ucData, KQ->ParamItem[i].Len);
			}
			Pos += KQ->ParamItem[i].Len;
		}
	}

	TxLen = Pos;
	Pos = JTT_PacketHead(JTT_TX_PARAM, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen - JTT_PACK_HEAD_LEN, 0, pBuf);
	KQ->LastTxMsgID = JTT_TX_PARAM;
	pBuf[TxLen] = XorCheck(pBuf, TxLen, 0);
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + 1, Buf);
	return TxLen;
}

uint32_t KQ_JTTDirectToMonitorTx(uint8_t *Buf)
{
	uint8_t pBuf[256];
	uint32_t TxLen;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	KQ->LastTxMsgSn++;
	pBuf[JTT_PACK_HEAD_LEN] = KQ->BLECmd;
	memcpy(&pBuf[JTT_PACK_HEAD_LEN + 1], KQ->BLECmdData, KQ->BLECmdLen);
	TxLen = KQ->BLECmdLen + 1;
	JTT_PacketHead(JTT_TX_DIRECT_TO_MONITOR, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen, 0, pBuf);
	KQ->LastTxMsgID = JTT_TX_DIRECT_TO_MONITOR;
	pBuf[TxLen + JTT_PACK_HEAD_LEN] = XorCheck(pBuf, TxLen + JTT_PACK_HEAD_LEN, 0);
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + JTT_PACK_HEAD_LEN + 1, Buf);
	return TxLen;
}

uint32_t KQ_JTTCarContrlTx(uint8_t *Buf)
{
	return 0;
}

uint32_t KQ_JTTUpgradeCmdTx(uint8_t *Buf)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint8_t pBuf[64];
	uint32_t TxLen;
	KQ->LastTxMsgSn++;
	TxLen = JTT_UpgradeMsgBoby(KQ->UpgradeType, KQ->UpgradeResult, pBuf + JTT_PACK_HEAD_LEN);
	JTT_PacketHead(JTT_TX_UPGRADE_RES, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen, 0, pBuf);
	pBuf[TxLen + JTT_PACK_HEAD_LEN] = XorCheck(pBuf, TxLen + JTT_PACK_HEAD_LEN, 0);
	KQ->LastTxMsgID = JTT_TX_UPGRADE_RES;
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + JTT_PACK_HEAD_LEN + 1, Buf);
	OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_PERIODIC, 10 * SYS_TICK);
	return TxLen;
}

uint32_t KQ_JTTTextTx(uint8_t *Buf, uint8_t *String, uint32_t Len)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint32_t TxLen;
	uint8_t *pBuf = Monitor->TempBuf;
	KQ->LastTxMsgSn++;
	pBuf[JTT_PACK_HEAD_LEN] = 0;
	memcpy(pBuf + JTT_PACK_HEAD_LEN + 1, String, Len);
	TxLen = Len + 1;
	JTT_PacketHead(JTT_TX_USER_TEXT, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen, 0, pBuf);
	pBuf[TxLen + JTT_PACK_HEAD_LEN] = XorCheck(pBuf, TxLen + JTT_PACK_HEAD_LEN, 0);
	KQ->LastTxMsgID = JTT_TX_USER_TEXT;
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + JTT_PACK_HEAD_LEN + 1, Buf);
	return TxLen;
}

/*
 * 接收相关API
 */
int32_t KQ_JTTRegRx(void *pData)
{
	uint16_t MsgSn;
	uint8_t Result;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	JTT_AnalyzeReg(&MsgSn, &Result, KQ->AuthCode, &KQ->AuthCodeLen, Buffer->Data, Buffer->Pos);

	if (KQ->LastTxMsgSn != MsgSn)
	{
		DBG("%04x %04x", KQ->LastTxMsgSn, MsgSn);
		KQ->IsRegOK = 0;
		return 1;
	}
	if (KQ->AuthCodeLen > 0)
	{
		DBG("Auth ok!");
		HexTrace(KQ->AuthCode, KQ->AuthCodeLen);

		User->KQ.AuthCodeLen = KQ->AuthCodeLen;
		memcpy(User->KQ.AuthCode, KQ->AuthCode, KQ->AuthCodeLen);
#ifndef FORCE_REG
		Param_Save(PARAM_TYPE_USER);
#endif
		KQ->IsRegOK = 1;
		return 1;
	}
	return 0;
}

int32_t KQ_JTTMonitorResRx(void *pData)
{
	uint16_t MsgSn;
	uint16_t MsgID;
	uint8_t Result;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	JTT_AnalyzeMonitorRes(&MsgID, &MsgSn, &Result, Buffer->Data);

	if (MsgID == KQ->LastTxMsgID)
	{
		KQ->IsLastTxCmdOK = !Result;
	}
	else
	{
		KQ->IsLastTxCmdOK = 0;
	}
	if ( (MsgID == KQ->LastTxMsgID) && (MsgSn == KQ->LastTxMsgSn) )
	{
		DBG("Cmd %04x, Sn %04x Result %02x", MsgID, MsgSn, Result);
		KQ->IsLastTxCmdOK = 1;
	}
	else
	{
		DBG("%04x %04x %04x %04x", MsgID, KQ->LastTxMsgID, MsgSn, KQ->LastTxMsgSn);
		KQ->IsLastTxCmdOK = 0;
		return 0;
	}
	KQ->IsLastTxCmdOK = !Result;
	return 0;
}

int32_t KQ_JTTSetParamRx(void *pData)
{
	uint32_t TxLen;
	uint8_t Result, Len;
	uint8_t ParamNum, i, j;
	uint32_t dwTemp, Pos;
	uint16_t wTemp;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	ParamNum = Buffer->Data[0];
	Result = 0;
	uint8_t iResult;
	IP_AddrUnion uIP;
	Param_APNStruct *APN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	if (ParamNum > KQ_PARAM_MAX)
	{
		Result = 1;
	}
	else
	{
		Pos = 1;
		for (i = 0;i < ParamNum; i++)
		{
			memcpy(&dwTemp, Buffer->Data + Pos, 4);
			Pos += 4;
			dwTemp = htonl(dwTemp);
			Len = Buffer->Data[Pos];
			Pos++;
			for (j = 0; j < KQ_PARAM_MAX; j++)
			{
				if (dwTemp == KQ->ParamItem[j].ID)
				{
					switch (KQ->ParamItem[j].ID)
					{
					case KQ_PARAM_VOICE:
						KQ_TTSParamDownload(Buffer->Data + Pos, Len);
						break;
					case KQ_PARAM_LED:
						KQ_LEDParamDownload(Buffer->Data + Pos, Len);
						break;
					default:
						if (KQ->ParamItem[j].Len > 4)
						{
							KQ->ParamItem[j].Len = Len;
							memcpy(KQ->ParamItem[j].uData.pData, Buffer->Data + Pos, Len);
							KQ->ParamItem[j].uData.pData[Len] = 0;
							DBG("%08x", dwTemp);
							HexTrace(KQ->ParamItem[j].uData.pData, Len);
						}
						else
						{
							if (KQ->ParamItem[j].Len == Len)
							{
								memcpy(KQ->ParamItem[j].uData.ucData, Buffer->Data + Pos, Len);
								DBG("%08x", dwTemp);
								HexTrace(KQ->ParamItem[j].uData.ucData, Len);
							}
							else
							{
								Result = 1;
							}
						}
						break;
					}
					break;
				}
			}
			Pos += Len;
		}
	}

	if (!Result)
	{
		if (KQ->ParamItem[KQ_PARAM_HEART_TIME_SN].uData.dwData != KQCtrl.Param[PARAM_UPLOAD_HEART_PERIOD])
		{
			DBG("new heart time %u", KQ->ParamItem[KQ_PARAM_HEART_TIME_SN].uData.dwData);
			KQCtrl.Param[PARAM_UPLOAD_HEART_PERIOD] = KQ->ParamItem[KQ_PARAM_HEART_TIME_SN].uData.dwData;
			if ( Param_Save(PARAM_TYPE_MONITOR) < 0 )
			{
				iResult = 1;
				TxLen = KQ_JTTDevResTx(iResult, KQCtrl.TxBuf);
				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
				return 0;
			}
		}

		if ( memcmp(APN->APNName, KQ->ParamItem[KQ_PARAM_APN_SN].uData.pData, strlen(APN->APNName)) )
		{
			DBG("new apn name %s", KQ->ParamItem[KQ_PARAM_APN_SN].uData.pData);
			strcpy(APN->APNName, KQ->ParamItem[KQ_PARAM_APN_SN].uData.pData);
			if ( Param_Save(PARAM_TYPE_APN) < 0 )
			{
				iResult = 1;
				TxLen = KQ_JTTDevResTx(iResult, KQCtrl.TxBuf);
				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
				return 0;
			}
		}

		uIP.u32_addr = inet_addr(KQ->ParamItem[KQ_PARAM_IP_SN].uData.pData);
		DecTrace(uIP.u8_addr, 4);
		dwTemp = htonl(KQ->ParamItem[KQ_PARAM_PORT_SN].uData.dwData);
		wTemp = dwTemp;
		if ( memcmp(&KQ->IP[2], uIP.u8_addr, 4) || (wTemp != KQ->Port) )
		{
			DBG("new addr %u", wTemp);
			DecTrace(uIP.u8_addr, 4);
			KQ->WaitFlag = 1;
			User_Req(KQ_CMD_SET_IP, 0, 0);

		}

		if ( (memcmp(KQ->UploadInfo, KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_TIME_SN].uData.ucData + 2, 2))
				|| memcmp(KQ->UploadInfo + 2, KQ->ParamItem[KQ_PARAM_UPLOAD_TIME_SN].uData.ucData + 2, 2) )
		{
			DBG("new timing");
			HexTrace(KQ->UploadInfo, 4);
			HexTrace(KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_TIME_SN].uData.ucData, 4);
			HexTrace(KQ->ParamItem[KQ_PARAM_UPLOAD_TIME_SN].uData.ucData, 4);
			memcpy(KQ->UploadInfo, KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_TIME_SN].uData.ucData + 2, 2);
			memcpy(KQ->UploadInfo + 2, KQ->ParamItem[KQ_PARAM_UPLOAD_TIME_SN].uData.ucData + 2, 2);
			KQ->WaitFlag = 1;
			User_Req(KQ_CMD_SET_TIME_PARAM, 0, 0);
		}

		if (!KQ->WaitFlag)
		{
			TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
			Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
		}
	}
	else
	{
		TxLen = KQ_JTTDevResTx(Result, KQCtrl.TxBuf);
		Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	}
	return 0;
}

int32_t KQ_JTTGetParamRx(void *pData)
{
	uint8_t ParamNum, i, j;
	uint32_t dwTemp;
	uint32_t TxLen;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	//Param_APNStruct *APN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	memset(KQ->ParamUpload, 0, KQ_PARAM_MAX);

	if ( !((Buffer->Pos - 1) % 4) )
	{
		ParamNum = Buffer->Data[0];
		//根据酷骑的要求，不检查参数总数
		//if ( (Buffer->Pos - 1) == (ParamNum * 4) )
		if ( 1 )
		{
			//搜索需要上传的参数ID
			for (i = 0; i < ParamNum; i++)
			{
				memcpy(&dwTemp, &Buffer->Data[1 + i * 4], 4);
				dwTemp = htonl(dwTemp);
				for (j = 0; j < KQ_PARAM_MAX; j++)
				{
					if (dwTemp == KQ->ParamItem[j].ID)
					{
						KQ->ParamUpload[j] = 1;
						break;
					}
				}
			}
		}
		else
		{
			DBG("param len %u error", Buffer->Pos - 1);
		}
	}
	else
	{
		DBG("param len %u error", Buffer->Pos - 1);
	}

	TxLen = KQ_JTTParamTx(KQCtrl.TxBuf);
	Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	return 0;
}

int32_t KQ_JTTGetAllParamRx(void *pData)
{
	uint32_t TxLen;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	//Param_APNStruct *APN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	memset(KQ->ParamUpload, 1, KQ_PARAM_MAX);
	TxLen = KQ_JTTParamTx(KQCtrl.TxBuf);
	Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	return 0;
}

int32_t KQ_JTTDevCtrlRx(void *pData)
{
	uint32_t TxLen;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	if (Buffer->Data[0] == JTT_DEV_CTRL_RESET)
	{
		KQCtrl.DevCtrlStatus = JTT_DEV_CTRL_RESET;
		TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
	}
	else
	{
		TxLen = KQ_JTTDevResTx(3, KQCtrl.TxBuf);
	}

	Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	return 0;
}

int32_t KQ_JTTUpgradeCmdRx(void *pData)
{
	uint8_t *Buf;
	uint32_t TxLen, Version;
	uint8_t Result, Len;
	uint32_t dwTemp, VersionLen;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;

	DBG("Type %02x", Buffer->Data[0]);

	VersionLen = Buffer->Data[6];
	Buf = COS_MALLOC(VersionLen + 1);
	memset(Buf, 0, VersionLen + 1);
	memcpy(Buf, &Buffer->Data[7], VersionLen);
	Version = strtoul(Buf, NULL, 10);
	DBG("Version %u", Version);
	memcpy(&dwTemp, &Buffer->Data[VersionLen + 7], 4);
	Len = htonl(dwTemp);
	COS_FREE(Buf);

	memset(KQ->FTPCmd, 0, Len + 1);
	memcpy(KQ->FTPCmd, &Buffer->Data[VersionLen + 11], Len);
	DBG("%s", KQ->FTPCmd);
	if (Buffer->Data[0] == 0)
	{
		//if (Version > gSys.Var[SOFTWARE_VERSION])
		//{
			User_Req(KQ_CMD_DOWNLOAD_GPRS, 0, 0);
			OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
		//}
		Result = 0;
	}
	else if (Buffer->Data[0] == 1)
	{
		User_Req(KQ_CMD_DOWNLOAD_BT, 0, 0);
		OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
		Result = 0;
	}
	else
	{
		Result = 1;
	}
	TxLen = KQ_JTTDevResTx(Result, KQCtrl.TxBuf);
	Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	return 0;
}

int32_t KQ_JTTForceUploadRx(void *pData)
{
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	uint8_t *pBuf = KQCtrl.TempBuf + 2;
	uint8_t Temp[32], i, j;
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	uint32_t Pos;
	uint32_t TxLen, AddLen;
	uint16_t wTemp;
	Monitor_RecordStruct Data;
	Monitor_RecordStruct *Record = &Data;
	memset(&Data, 0, sizeof(Monitor_DataStruct));
	Monitor_Record(Record);
	KQ->LastTxMsgSn++;

	wTemp = htons(KQ->LastRxMsgSn);
	memcpy(KQCtrl.TempBuf, &wTemp, 2);
	TxLen = JTT_LocatBaseInfoMsgBoby(Record, pBuf + JTT_PACK_HEAD_LEN);

	Pos = TxLen + JTT_PACK_HEAD_LEN;
	AddLen = JTT_AddLocatMsgBoby(JTT_LOCAT_ADD_ITEM_CSQ, 1, &gSys.State[RSSI_STATE], pBuf + Pos);
	Pos += AddLen;

	Temp[0] = Record->CN[0] + Record->CN[1] + Record->CN[2] + Record->CN[3];
	AddLen = JTT_AddLocatMsgBoby(JTT_LOCAT_ADD_ITEM_GNSS_NUM, 1, &Temp[0], pBuf + Pos);
	Pos += AddLen;

	memset(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData, 0, KQ->CustItem[KQ_JTT_GSM_INFO].Len);

	wTemp = BCDToInt(gSys.IMSI, 2);
	wTemp = htons(wTemp);
	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData, &wTemp, 2);

	wTemp = BCDToInt(gSys.IMSI + 2, 1);
	wTemp = htons(wTemp);
	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 2, &wTemp, 2);

	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 4, gSys.CurrentCell.nTSM_LAI + 3, 2);
	memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 8, gSys.CurrentCell.nTSM_CellID, 2);
	KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData[10] = RssiToCSQ(gSys.CurrentCell.nTSM_AvRxLevel);
	j = 0;
	for (i = 0; i < gSys.NearbyCell.nTSM_NebCellNUM; i++)
	{
		if (gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID[0])
		{
			memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 11 + j * 7, gSys.NearbyCell.nTSM_NebCell[i].nTSM_LAI + 3, 2);
			memcpy(KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData + 15 + j * 7, gSys.NearbyCell.nTSM_NebCell[i].nTSM_CellID, 2);
			KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData[17 + j * 7] = RssiToCSQ(gSys.NearbyCell.nTSM_NebCell[i].nTSM_AvRxLevel);
			j++;
			if (j >= 2)
				break;
		}
	}


	for (i = 0; i < KQ_JTT_CUST_ITEM_MAX; i++)
	{
		if (KQ->CustItem[i].Len > 4)
		{
			AddLen = JTT_AddLocatMsgBoby(KQ->CustItem[i].ID, KQ->CustItem[i].Len, KQ->CustItem[i].uData.pData, pBuf + Pos);
		}
		else
		{
			AddLen = JTT_AddLocatMsgBoby(KQ->CustItem[i].ID, KQ->CustItem[i].Len, KQ->CustItem[i].uData.ucData, pBuf + Pos);
		}
		Pos += AddLen;
	}
	//比0x0200命令，增加了2个字节
	TxLen = Pos + 2;
	pBuf = KQCtrl.TempBuf;
	JTT_PacketHead(JTT_TX_FORCE_UPLOAD, KQ->LastTxMsgSn, Monitor->MonitorID.ucID, TxLen - JTT_PACK_HEAD_LEN, 0, pBuf);
	KQ->LastTxMsgID = JTT_TX_FORCE_UPLOAD;
	pBuf[TxLen] = XorCheck(pBuf, TxLen, 0);
	TxLen = TransferPack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, pBuf, TxLen + 1, Monitor->TxBuf);
	Monitor_RecordResponse(Monitor->TxBuf, TxLen);
	return 0;
}

int32_t KQ_JTTTextRx(void *pData)
{
	uint32_t TxLen;
	TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
	Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	return 0;
}

int32_t KQ_JTTCarCtrlRx(void *pData)
{
	uint32_t TxLen;
	TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
	Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	return 0;
}


int32_t KQ_JTTSetRSARx(void *pData)
{
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	KQ->BLECmd = KQ_DT_SET_RSA;
	KQ->BLECmdLen = 22;
	memcpy(KQ->BLECmdData, Buffer->Data, 22);
	KQ->WaitFlag = 1;
	User_Req(KQ_CMD_SET_BT, 0, 0);
	return 0;
}

int32_t KQ_JTTDirectToDevRx(void *pData)
{
	uint32_t TxLen;
	Buffer_Struct *Buffer = (Buffer_Struct *)pData;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	if (0x41 == Buffer->Data[0])
	{
		KQ->BLECmd = KQ_DT_SET_OTHER;
		KQ->BLECmdLen = ( (Buffer->Pos - 1) > sizeof(KQ->BLECmdData) )?( sizeof(KQ->BLECmdData) ):(Buffer->Pos - 1);
		memcpy(KQ->BLECmdData, Buffer->Data + 1, KQ->BLECmdLen);
		KQ->WaitFlag = 1;
		DBG("%u", KQ->BLECmd);
		HexTrace(KQ->BLECmdData, KQ->BLECmdLen);
		User_Req(KQ_CMD_SET_BT, 0, 0);
	}
	else
	{
		TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
		Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	}
	return 0;
}

const CmdFunStruct KQCmdFun[]=
{
		{
				JTT_RX_NORMAIL_RESPONSE,
				KQ_JTTMonitorResRx,
		},
		{
				JTT_RX_REG_RESPONSE,
				KQ_JTTRegRx,
		},
		{
				JTT_RX_GET_PARAM_ALL,
				KQ_JTTGetAllParamRx,
		},
		{
				JTT_RX_SET_PARAM,
				KQ_JTTSetParamRx,
		},
		{
				JTT_RX_CONTROL_DEVICE,
				KQ_JTTDevCtrlRx,
		},
		{
				JTT_RX_GET_PARAM,
				KQ_JTTGetParamRx,
		},
		{
				JTT_RX_UPGRADE_CMD,
				KQ_JTTUpgradeCmdRx,
		},
		{
				JTT_RX_FORCE_UPLOAD,
				KQ_JTTForceUploadRx,
		},
		{
				JTT_RX_TEXT_INFO,
				KQ_JTTTextRx,
		},
		{
				JTT_RX_CAR_CONTROL,
				KQ_JTTCarCtrlRx,
		},
		{
				JTT_RX_DIRECT_TO_DEV,
				KQ_JTTDirectToDevRx,
		},
		{
				JTT_RX_SET_RSA,
				KQ_JTTSetRSARx,
		}
};

int32_t KQ_ReceiveAnalyze(void *pData)
{
	uint8_t *Buf = KQCtrl.TempBuf;
	uint32_t RxLen = (uint32_t)pData;
	uint32_t FinishLen = 0,i, TxLen;
	uint16_t MsgID, MsgSn;
	uint8_t Check,FindCmd;
	uint8_t SimID[6];
	int32_t Result;
	int32_t Error;
	Buffer_Struct Buffer;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	DBG("Receive %u", RxLen);

	while (RxLen)
	{
		if (RxLen > MONITOR_RXBUF_LEN)
		{
			FinishLen = MONITOR_RXBUF_LEN;
		}
		else
		{
			FinishLen = RxLen;
		}

		Error = (int32_t)OS_SocketReceive(KQCtrl.Net.SocketID, KQCtrl.RxBuf, FinishLen, NULL, NULL);
		if (Error <= 0)
		{
			DBG("%d", Error);
			return -1;
		}
		RxLen -= (uint32_t)Error;
		//加入协议分析
		HexTrace(KQCtrl.RxBuf, FinishLen);
		for (i = 0; i < FinishLen; i++)
		{
			switch (KQCtrl.RxState)
			{
			case JTT_PRO_FIND_HEAD:
				if (JTT_PACK_FLAG == KQCtrl.RxBuf[i])
				{
					KQCtrl.RxState = JTT_PRO_FIND_TAIL;
					KQCtrl.RxLen = 0;
				}
				break;

			case JTT_PRO_FIND_TAIL:
				if (JTT_PACK_FLAG == KQCtrl.RxBuf[i])
				{
					//接收完成
					FindCmd = 0;
					KQCtrl.AnalzeLen = TransferUnpack(JTT_PACK_FLAG, JTT_PACK_CODE, JTT_PACK_CODE_F1, JTT_PACK_CODE_F2, KQCtrl.AnalyzeBuf, KQCtrl.RxLen, Buf);

					if (KQCtrl.AnalzeLen > 12)
					{
						Check = XorCheck(Buf, KQCtrl.AnalzeLen - 1, 0);
						if (Check != Buf[KQCtrl.AnalzeLen - 1])
						{
							DBG("check error %02x %02x", Check, Buf[KQCtrl.AnalzeLen - 1]);
							KQCtrl.RxState = JTT_PRO_FIND_HEAD;
							break;
						}
						KQCtrl.AnalzeLen--;
						Result = JTT_AnalyzeHead(&MsgID, &MsgSn, SimID, Buf, KQCtrl.AnalzeLen, &KQCtrl.AnalzeLen);
						if (Result < 0)
						{
							KQCtrl.RxState = JTT_PRO_FIND_HEAD;
							break;
						}
						if (MsgID != JTT_RX_NORMAIL_RESPONSE)
						{
							KQ->LastRxMsgSn = MsgSn;
							KQ->LastRxMsgID = MsgID;
						}
						DBG("Rx result %u MsgID %x MsgSn %x", Result, MsgID, MsgSn);
						for (i = 0; i < sizeof(KQCmdFun)/sizeof(CmdFunStruct); i++)
						{
							if (KQCmdFun[i].Cmd == (uint32_t)MsgID)
							{
								Buffer.Data = &Buf[Result];
								Buffer.Pos = KQCtrl.AnalzeLen;
								KQCmdFun[i].Func(&Buffer);
								FindCmd = 1;
								break;
							}
						}
						if (!FindCmd)
						{
							TxLen = KQ_JTTDevResTx(3, KQCtrl.TxBuf);
							Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
						}
						KQCtrl.RxState = JTT_PRO_FIND_HEAD;
					}
					else
					{
						DBG("%u", KQCtrl.AnalzeLen);
					}
					KQCtrl.RxState = JTT_PRO_FIND_HEAD;
				}
				else
				{
					KQCtrl.AnalyzeBuf[KQCtrl.RxLen++] = KQCtrl.RxBuf[i];
					if (KQCtrl.RxLen >= MONITOR_RXBUF_LEN)
					{
						KQCtrl.RxState = JTT_PRO_FIND_HEAD;
					}
				}
				break;
			default:
				KQCtrl.RxState = JTT_PRO_FIND_HEAD;
				break;
			}
		}
		if (RxLen)
		{
			DBG("rest %u", RxLen);
		}
	}
	KQCtrl.RxState = JTT_PRO_FIND_HEAD;
	return 0;
}

uint8_t KQ_Connect(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, int8_t *Url)
{
	uint8_t ProcessFinish = 0;
	Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
	if (Net->SocketID != INVALID_SOCKET)
	{
		DBG("Need close socket before connect!");
		Net_Disconnect(Net);
	}
	if (Url)
	{
		Net_Connect(Net, 0, Url);
	}
	else
	{
		Net_Connect(Net, Net->IPAddr.s_addr, Url);
	}

	if (Net->Result != NET_RES_CONNECT_OK)
	{
		if (Net->SocketID != INVALID_SOCKET)
		{
			Net_Disconnect(Net);
		}
		ProcessFinish = 0;
	}
	else
	{
		ProcessFinish = 1;
	}
	return ProcessFinish;
}

uint8_t KQ_Send(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, uint32_t Len)
{
	Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
	DBG("%u", Len);
	HexTrace(Monitor->TxBuf, Len);
	Net_Send(Net, Monitor->TxBuf, Len);
	if (Net->Result != NET_RES_SEND_OK)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void KQ_Task(void *pData)
{
	Monitor_CtrlStruct *Monitor = &KQCtrl;
	Net_CtrlStruct *Net = &KQCtrl.Net;
	KQ_CustDataStruct *KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	Param_APNStruct *APN = &gSys.nParam[PARAM_TYPE_APN].Data.APN;
	IP_AddrUnion uIP;
	//uint32_t SleepTime;
	uint32_t KeepTime;
	uint32_t ConnectTime = 1;
	//uint8_t FindIP = 0;
	uint8_t ErrorOut = 0;

	COS_EVENT Event;
	uint8_t DataType = 0;
	uint32_t TxLen = 0;
	uint32_t dwTemp;
	uint16_t wTemp;
	uint8_t iResult;
//下面变量为每个协议独有的
	DBG("Task start! %u %u %u %u %u %u %u %u %u %u" ,
			(int)Monitor->Param[PARAM_GS_WAKEUP_MONITOR], (int)Monitor->Param[PARAM_GS_JUDGE_RUN],
			(int)Monitor->Param[PARAM_UPLOAD_RUN_PERIOD], (int)Monitor->Param[PARAM_UPLOAD_STOP_PERIOD],
			(int)Monitor->Param[PARAM_UPLOAD_HEART_PERIOD], (int)Monitor->Param[PARAM_MONITOR_NET_TO],
			(int)Monitor->Param[PARAM_MONITOR_KEEP_TO], (int)Monitor->Param[PARAM_MONITOR_SLEEP_TO],
			(int)Monitor->Param[PARAM_MONITOR_RECONNECT_MAX], (int)Monitor->Param[PARAM_MONITOR_ADD_MILEAGE]);
//	Monitor->MonitorID.ucID[0] = 0x01;
//	Monitor->MonitorID.ucID[1] = 0x36;
//	memcpy(Monitor->MonitorID.ucID + 2, gSys.IMEI + 4, 4);
//	memcpy(Monitor->MonitorID.ucID, gSys.IMEI + 1, 6);
	Monitor->MonitorID.ucID[0] = ((gSys.IMEI[1] & 0x0f) << 4) | ((gSys.IMEI[2] & 0xf0) >> 4);
	Monitor->MonitorID.ucID[1] = ((gSys.IMEI[2] & 0x0f) << 4) | ((gSys.IMEI[3] & 0xf0) >> 4);
	Monitor->MonitorID.ucID[2] = ((gSys.IMEI[3] & 0x0f) << 4) | ((gSys.IMEI[4] & 0xf0) >> 4);
	Monitor->MonitorID.ucID[3] = ((gSys.IMEI[4] & 0x0f) << 4) | ((gSys.IMEI[5] & 0xf0) >> 4);
	Monitor->MonitorID.ucID[4] = ((gSys.IMEI[5] & 0x0f) << 4) | ((gSys.IMEI[6] & 0xf0) >> 4);
	Monitor->MonitorID.ucID[5] = ((gSys.IMEI[6] & 0x0f) << 4) | ((gSys.IMEI[7] & 0xf0) >> 4);
	//JTT_MakeMonitorID(Monitor);
    DBG("monitor id");
    HexTrace(Monitor->MonitorID.ucID, 6);

    KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    KQ->LastTxMsgSn = 0xffff;
    KQ->LastRxMsgSn = 0;
    gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    if (User->KQ.AuthCodeLen)
    {
    	KQ->IsRegOK = 1;
    	DBG("last reg data");
    	HexTrace(User->KQ.AuthCode, User->KQ.AuthCodeLen);
    	memcpy(KQ->AuthCode, User->KQ.AuthCode, User->KQ.AuthCodeLen);
    	KQ->AuthCodeLen = User->KQ.AuthCodeLen;
    }
#ifdef FORCE_REG
    KQ->IsRegOK = 0;
#endif
    while (!ErrorOut)
    {
//    	if (Monitor->IsWork && Monitor->Param[PARAM_MONITOR_KEEP_TO])
//    	{
//    		if (gSys.Var[SYS_TIME] > KeepTime)
//    		{
//    			DBG("sleep!");
//    			gSys.RecordCollect.WakeupFlag = 0;
//    			if (Net->SocketID != INVALID_SOCKET)
//    			{
//    				DBG("Need close socket before sleep!");
//    				Net_Disconnect(Net);
//    			}
//    			gSys.State[MONITOR_STATE] = JTT_STATE_SLEEP;
//    			Monitor->IsWork = 0;
//    			SleepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_SLEEP_TO];
//    		}
//    	}

    	switch (gSys.State[MONITOR_STATE])
    	{
    	case JTT_STATE_CONNECT:
    		if (KQ->Port)
    		{

				memcpy(uIP.u8_addr, &KQ->IP[2], 4);
				KQ->ParamItem[KQ_PARAM_IP_SN].Len = sprintf(KQ->ParamItem[KQ_PARAM_IP_SN].uData.pData, "%u.%u.%u.%u", (int)uIP.u8_addr[0], (int)uIP.u8_addr[1],
						(int)uIP.u8_addr[2], (int)uIP.u8_addr[3]);
    			dwTemp = KQ->Port;
    			KQ->ParamItem[KQ_PARAM_PORT_SN].uData.dwData = htonl(dwTemp);

    			DBG("IP %s %u", KQ->ParamItem[KQ_PARAM_IP_SN].uData.pData, KQ->Port);
    			Net->IPAddr.s_addr = uIP.u32_addr;
    			Monitor->Net.UDPPort = 0;
#ifdef KQ_TEST_MODE
    			Monitor->Net.TCPPort = KQ_TEST_PORT;
#else

				Monitor->Net.TCPPort = KQ->Port;

#endif
    			memcpy(&wTemp, KQ->UploadInfo, 2);
    			dwTemp = htons(wTemp);
    			KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_TIME_SN].uData.dwData = htonl(dwTemp);

    			memcpy(&wTemp, KQ->UploadInfo + 2, 2);
    			dwTemp = htons(wTemp);
    			KQ->ParamItem[KQ_PARAM_UPLOAD_TIME_SN].uData.dwData = htonl(dwTemp);

    			if (KQ->CustItem[KQ_JTT_LOCK_STATE].uData.ucData[1] & KQ_BLE_MODE_LOCK_UPLOAD)
    			{
    				KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_EN_SN].uData.dwData = htonl(0x00000002);
    			}
    			else
    			{
    				KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_EN_SN].uData.dwData = htonl(0x00000001);
    			}

    			KQ->ParamItem[KQ_PARAM_GSENSOR_EN_SN].uData.ucData[0] = KQ->CtrlInfo[0];
    			KQ->ParamItem[KQ_PARAM_GSENSOR_UPLOAD_SN].uData.ucData[0] = KQ->CtrlInfo[1];
#ifdef KQ_TEST_MODE
    			KQ_Connect(Monitor, Net, KQ_TEST_URL);
#else
    			KQ_Connect(Monitor, Net, NULL);
#endif
    			KQ->ParamItem[KQ_PARAM_APN_SN].Len = strlen(APN->APNName);
    			KQ->ParamItem[KQ_PARAM_APN_SN].uData.pData = COS_MALLOC(32);
    			strcpy(KQ->ParamItem[KQ_PARAM_APN_SN].uData.pData, APN->APNName);

        		if (Net->Result == NET_RES_CONNECT_OK)
        		{
        			if (KQ->IsRegOK)
        			{
        				gSys.State[MONITOR_STATE] = JTT_STATE_AUTH;
        			}
        			else
        			{
        				gSys.State[MONITOR_STATE] = JTT_STATE_REG;
        			}

        			break;
        		}
        		Monitor->ReConnCnt++;
        		if (Monitor->ReConnCnt > Monitor->Param[PARAM_MONITOR_RECONNECT_MAX])
        		{
        			DBG("Reconnect %u times, reboot");
        			ErrorOut = 1;
        			break;
        		}
        		if (!ConnectTime)
        		{
        			ConnectTime = 1;
        		}
        		Net->To = ConnectTime * Monitor->ReConnCnt;
        		if (Net->To > 120)
        		{
        			Net->To = 120;
        			ConnectTime = 120;
        		}
        		else
        		{
        			ConnectTime = Net->To;
        		}
        		DBG("fail %u times, Wait %usec reconnect", Monitor->ReConnCnt, Net->To);
        		Net_WaitTime(Net);
    		}
    		else
    		{
    			if ( gSys.Var[SYS_TIME] < 15 )
    			{
    				if ( (gSys.Var[SYS_TIME] > 6) && !(KQ->BLEReportFlag) )
    				//if ( 1 )
    				{
    					OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_COM_TO_USER, KQ_CMD_SYS_ON, 0, 0);
    				}
					Net->To = 1;
					Net_WaitTime(Net);
    			}
    			else
    			{
    				KQ->Port = MainInfo->TCPPort;
    				uIP.u32_addr = MainInfo->MainIP;
    				memcpy(&KQ->IP[2], uIP.u8_addr, 4);
    			}
    		}
    		break;
    	case JTT_STATE_REG:
    		DBG("start jtt reg");
    		TxLen = KQ_JTTRegTx(Monitor->TxBuf);
    		KQ_Send(Monitor, Net, TxLen);
    		if (Net->Result != NET_RES_SEND_OK)
    		{
    			gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    			break;
    		}
    		if (Net->IsReceive)
    		{
        		if (KQ->IsRegOK)
        		{
        			gSys.State[MONITOR_STATE] = JTT_STATE_AUTH;
        		}
        		break;
    		}
    		Net->To = 10;
    		Net_WaitReceive(Net);
    		if (Net->Result != NET_RES_UPLOAD)
    		{
    			DBG("To!");
    			gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    			break;
    		}
    		if (KQ->IsRegOK)
    		{
    			gSys.State[MONITOR_STATE] = JTT_STATE_AUTH;
    		}
    		else
    		{
    			DBG("jtt reg Fail!");
        		TxLen = KQ_JTTUnRegTx(Monitor->TxBuf);
        		KQ_Send(Monitor, Net, TxLen);
        		gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
        		Net->To = 10;
        		Net_WaitReceive(Net);
    		}
			break;

    	case JTT_STATE_AUTH:
    		DBG("start jtt auth");
    		KQ->IsLastTxCmdOK = 0;
    		TxLen = KQ_JTTAuthTx(Monitor->TxBuf);
    		KQ_Send(Monitor, Net, TxLen);
    		if (Net->Result != NET_RES_SEND_OK)
    		{
    			gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    			break;
    		}
    		Net->To = 10;
    		Net_WaitReceive(Net);
    		if (Net->Result != NET_RES_UPLOAD)
    		{
    			DBG("To!");
    			gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    			break;
    		}
    		if (KQ->IsLastTxCmdOK)
    		{
    			gSys.State[MONITOR_STATE] = JTT_STATE_USER;
    			Net->To = 60;
    		}
    		else
    		{
    			DBG("Auth fail!");
    			gSys.State[MONITOR_STATE] = JTT_STATE_REG;
    			Net->To = 60;
    		}
    		break;

    	case JTT_STATE_DATA:
    		ConnectTime = 1;
    		Monitor->ReConnCnt = 0;
			if (Net->Heart)
			{
				//合成心跳包
				Net->Heart = 0;
				TxLen = KQ_JTTHeartTx(KQCtrl.TxBuf);
				KQ_Send(Monitor, Net, TxLen);
				if (Net->Result != NET_RES_SEND_OK)
				{
					gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;;
					break;
				}
			}

    		if (Monitor_GetCacheLen(CACHE_TYPE_ALL))
    		{
    			if (Monitor_GetCacheLen(CACHE_TYPE_RES))
    			{
    				DataType = CACHE_TYPE_RES;
    				TxLen = Monitor_ExtractResponse(Monitor->TxBuf);

    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_ALARM))
    			{
    				DataType = CACHE_TYPE_ALARM;
    				Monitor_ExtractAlarm(&Monitor->Record);
    				TxLen = KQ_JTTLocatInfoTx(Monitor->TxBuf, &Monitor->Record);

    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_DATA))
    			{
    				DataType = CACHE_TYPE_DATA;
    				Monitor_ExtractData(&Monitor->Record);
    				TxLen = KQ_JTTLocatInfoTx(Monitor->TxBuf, &Monitor->Record);
    			}
    			KQ->WaitFlag = 0;
    			KQ->IsWaitOk = 0;
				KQ_Send(Monitor, Net, TxLen);

				if (Net->Result != NET_RES_SEND_OK)
				{
					gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
					break;
				}
				else
				{
					Monitor_DelCache(DataType, 0);
				}


//				if (DataType != CACHE_TYPE_RES)
//				{

					//Net->To = 1;
					//Net_WaitEvent(Net);

	        		if (KQ->WaitFlag)
	        		{
	        			OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
	        			Net->To = 10;
	        			Net_WaitTime(Net);
	        			DBG("%u", KQ->IsWaitOk);
	        			if (KQ->IsWaitOk)
	        			{
	        				TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
	        				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	        			}
	        			else
	        			{
	        				TxLen = KQ_JTTDevResTx(1, KQCtrl.TxBuf);
	        				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
	        			}
	        		}

	        		if ( (KQ->UploadInfo[2] == 0) && (KQ->UploadInfo[3] == 5) )
	        		{

	        		}
	        		else
	        		{
		        		if ( DataType == CACHE_TYPE_DATA )
		        		{
		        			DBG("%u %u %u", Monitor_GetCacheLen(CACHE_TYPE_DATA), UserCtrl.DevUpgradeFlag, UserCtrl.GPRSUpgradeFlag);
		        			if (!Monitor_GetCacheLen(CACHE_TYPE_DATA) && !UserCtrl.DevUpgradeFlag && !UserCtrl.GPRSUpgradeFlag)
		        			{
		        				if (gSys.State[FIRST_LOCAT_STATE])
		        				{
		        					gSys.Var[SHUTDOWN_TIME] = gSys.Var[SYS_TIME] + 10;//立刻关机
		        					OS_StartTimer(gSys.TaskID[USER_TASK_ID], USER_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK * 10);
		        				}
		        			}
		        		}
	        		}

//				}
    		}
    		else
    		{
    			if (Monitor->DevCtrlStatus == JTT_DEV_CTRL_RESET)
    			{
    				SYS_Reset();
    			}
    			if (Monitor->Param[PARAM_MONITOR_KEEP_TO])
    			{
    				Net->To = Monitor->Param[PARAM_MONITOR_KEEP_TO];
    			}
    			else if (Monitor->Param[PARAM_UPLOAD_STOP_PERIOD])
    			{
    				Net->To = Monitor->Param[PARAM_UPLOAD_STOP_PERIOD] * 2;
    			}
    			else
    			{
    				Net->To = Monitor->Param[PARAM_UPLOAD_RUN_PERIOD] * 2;
    			}
    			KQ->WaitFlag = 0;
    			KQ->IsWaitOk = 0;
    			Net_WaitEvent(Net);
    			if (Net->Result != NET_RES_UPLOAD)
    			{
    				DBG("error!");
    				gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    			}

        		if (KQ->WaitFlag)
        		{
        			OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
        			Net->To = 10;
        			Net_WaitTime(Net);
        			DBG("%u", KQ->IsWaitOk);
        			if (KQ->IsWaitOk)
        			{
        				TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
        				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
        			}
        			else
        			{
        				TxLen = KQ_JTTDevResTx(1, KQCtrl.TxBuf);
        				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
        			}
        		}
    		}
    		if (gSys.RecordCollect.WakeupFlag)
    		{
    			KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    		}
    		gSys.RecordCollect.WakeupFlag = 0;
    		break;

    	case JTT_STATE_SLEEP:
    		gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    		//酷骑平台等待是否参数下发
    		break;
    	case JTT_STATE_USER:
    		Net->To = 10;

    		KQ->WaitFlag = 0;
    		KQ->IsWaitOk = 0;
    		iResult = 0;
    		Net_WaitEvent(Net);
    		if (Net->Result == NET_RES_ERROR)
    		{
    			gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    			break;
    		}
    		if (KQ->WaitFlag)
    		{
    			OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_USER_REQ, 0, 0, 0);
    			Net->To = 10;
    			Net_WaitTime(Net);
    			if (KQ->IsWaitOk)
    			{
    				TxLen = KQ_JTTDevResTx(0, KQCtrl.TxBuf);
    				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
    			}
    			else
    			{
    				TxLen = KQ_JTTDevResTx(1, KQCtrl.TxBuf);
    				Monitor_RecordResponse(KQCtrl.TxBuf, TxLen);
    			}
    		}
    		gSys.State[MONITOR_STATE] = JTT_STATE_DATA;
    		if (__GetUpgradeState() == RDA_UPGRADE_OK)
    		{
        		KQ->UpgradeType = 0;
        		KQ->UpgradeResult = 0;
        		TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
        		Monitor_RecordResponse(Monitor->TempBuf, TxLen);
    		}
    		else if(__GetUpgradeState() == RDA_UPGRADE_FAIL)
    		{
        		KQ->UpgradeType = 0;
        		KQ->UpgradeResult = 1;
        		TxLen = KQ_JTTUpgradeCmdTx(Monitor->TempBuf);
        		Monitor_RecordResponse(Monitor->TempBuf, TxLen);
    		}
    		__ClearUpgradeState();
    		gSys.RecordCollect.RecordStartTime = gSys.Var[SYS_TIME] + 45;
    		break;
    	default:
    		gSys.State[MONITOR_STATE] = JTT_STATE_CONNECT;
    		break;
    	}
    }
	SYS_Reset();
	while (1)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
	}
}

void KQ_Config(void)
{
	KQ_CustDataStruct *KQ;
	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(KQ_Task, NULL,
				NULL, MMI_TASK_MAX_STACK_SIZE , MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI KQ Task");
	KQCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;
	KQCtrl.Net.SocketID = INVALID_SOCKET;
	KQCtrl.Net.TaskID = gSys.TaskID[MONITOR_TASK_ID];
	KQCtrl.Net.Channel = GPRS_CH_MAIN_MONITOR;
	KQCtrl.Net.TimerID = MONITOR_TIMER_ID;
	KQCtrl.RxState = JTT_PRO_FIND_HEAD;
	KQCtrl.Net.ReceiveFun = KQ_ReceiveAnalyze;

	if (!KQCtrl.Param[PARAM_UPLOAD_RUN_PERIOD])
	{
		KQCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 60;
	}
	KQCtrl.CustData = (KQ_CustDataStruct *)COS_MALLOC(sizeof(KQ_CustDataStruct));
	memset(KQCtrl.CustData, 0, sizeof(KQ_CustDataStruct));
	Monitor_DelCache(CACHE_TYPE_DATA, 1);
	KQ = (KQ_CustDataStruct *)KQCtrl.CustData;
	KQ->CustItem[KQ_JTT_GSM_VERSION].ID = KQ_JTT_GSM_VERSION + JTT_CUST_ITEM_START;
	KQ->CustItem[KQ_JTT_GSM_VERSION].Len = 4;
	//KQ->CustItem[KQ_JTT_GSM_VERSION].uData.dwData = htonl(gSys.Var[SOFTWARE_VERSION]);
	KQ->CustItem[KQ_JTT_GSM_VERSION].uData.ucData[0] = __KQ_GPRS_APP_VER_H__;
	KQ->CustItem[KQ_JTT_GSM_VERSION].uData.ucData[1] = __KQ_GPRS_APP_VER_L__;
	KQ->CustItem[KQ_JTT_GSM_VERSION].uData.ucData[2] = __KQ_GPRS_CORE_VER_H__;
	KQ->CustItem[KQ_JTT_GSM_VERSION].uData.ucData[3] = __KQ_GPRS_CORE_VER_L__;

	KQ->CustItem[KQ_JTT_GSM_INFO].ID = KQ_JTT_GSM_INFO + JTT_CUST_ITEM_START;
	KQ->CustItem[KQ_JTT_GSM_INFO].Len = 0x19;
	KQ->CustItem[KQ_JTT_GSM_INFO].uData.pData = COS_MALLOC(32);

	KQ->ParamItem[KQ_PARAM_HEART_TIME_SN].ID = JTT_PARAM_UPLOAD_HEART_PERIOD;
	KQ->ParamItem[KQ_PARAM_HEART_TIME_SN].Len = 4;
	KQ->ParamItem[KQ_PARAM_HEART_TIME_SN].uData.dwData = KQCtrl.Param[PARAM_UPLOAD_HEART_PERIOD];

	KQ->ParamItem[KQ_PARAM_APN_SN].ID = JTT_PARAM_APN_NAME;

	KQ->ParamItem[KQ_PARAM_IP_SN].ID = JTT_PARAM_MAIN_IP_URL;
	KQ->ParamItem[KQ_PARAM_IP_SN].uData.pData = COS_MALLOC(URL_LEN_MAX);

	KQ->ParamItem[KQ_PARAM_PORT_SN].ID = JTT_PARAM_TCP_PORT;
	KQ->ParamItem[KQ_PARAM_PORT_SN].Len = 4;

	KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_TIME_SN].ID = JTT_PARAM_UPLOAD_SLEEP_PERIOD;
	KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_TIME_SN].Len = 4;

	KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_EN_SN].ID = JTT_PARAM_UPLOAD_ALARM_PERIOD;
	KQ->ParamItem[KQ_PARAM_LOCK_UPLOAD_EN_SN].Len = 4;

	KQ->ParamItem[KQ_PARAM_UPLOAD_TIME_SN].ID = JTT_PARAM_UPLOAD_RUN_PERIOD;
	KQ->ParamItem[KQ_PARAM_UPLOAD_TIME_SN].Len = 4;


	KQ->ParamItem[KQ_PARAM_GSENSOR_EN_SN].ID = KQ_PARAM_GSENSOR_EN;
	KQ->ParamItem[KQ_PARAM_GSENSOR_EN_SN].Len = 1;

	KQ->ParamItem[KQ_PARAM_GSENSOR_UPLOAD_SN].ID = KQ_PARAM_GSENSOR_UPLOAD;
	KQ->ParamItem[KQ_PARAM_GSENSOR_UPLOAD_SN].Len = 1;

	KQ->ParamItem[KQ_PARAM_VOICE_SN].ID = KQ_PARAM_VOICE;
	KQ->ParamItem[KQ_PARAM_VOICE_SN].Len = 0;
	KQ->ParamItem[KQ_PARAM_VOICE_SN].uData.pData = COS_MALLOC(16 * sizeof(TTS_CodeDataStruct));

	KQ->ParamItem[KQ_PARAM_LED_SN].ID = KQ_PARAM_LED;
	KQ->ParamItem[KQ_PARAM_LED_SN].Len = 0;
	//KQ->ParamItem[KQ_PARAM_LED_SN].uData.pData = COS_MALLOC(16 * sizeof(LED_CodeDataStruct));
}
#endif
