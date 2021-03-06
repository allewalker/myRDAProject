#include "user.h"
#if (__CUST_CODE__ == __CUST_GLEAD__ || __CUST_CODE__ == __CUST_NONE__)
Monitor_CtrlStruct __attribute__((section (".usr_ram"))) GleadCtrl;
const int8_t code_256_64[64]={'0','1','2','3','4','5','6','7','8','9',':',';','A',
	'B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U',
	'V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
	'p','q','r','s','t','u','v','w','x','y','z'};//定义转码函数
//车辆状态及报警
enum
{
    HQ_BUS_ERROR,					//总线故障，历史，指示设备工作在异常状态
    HQ_CAR_LOCK_STATUS,			//锁车状态
    HQ_GPS_ERROR,					//GPS模块故障，报警，GPS模块故障
    HQ_CAR_LOCK_ERROR,				//锁车电路故障
    HQ_ACC_ON,						//ACC开
    HQ_HEAVY_CAR,					//重车
    HQ_CAR_DOOR_OPEN,				//车门开
    HQ_BACKUP_ON,					//备用输出开
    HQ_PRIVATE_MODE_ON,			//私密状态
    HQ_GPS_STATUS_0,				//GPS天线状态
    HQ_GPS_STATUS_1,				//GPS天线状态
    HQ_UNUSE0,						//未使用0，
    HQ_HIGH_BEAM_ON,				//远光灯开，
    HQ_RIGHTTURN_INDICATOR_ON,		//右转灯开，
    HQ_LEFTTURN_INDICATOR_ON,		//左转灯开，
    HQ_BRAKE_LIGHT_ON,				//刹车灯开，
    HQ_REVERSING_LIGHT_ON,			//倒车灯开,
    HQ_FRONT_FOG_LIGHT_ON,			//前雾灯开,
    HQ_CAR_DOOR_CLOSE,				//车门关
    HQ_DIPPED_BEAM_ON,				//近光灯开
    HQ_ROB_ALARM,					//劫警
    HQ_STEAL_ALARM,				//盗警
    HQ_CRASH_ALARM,				//碰撞报警
    HQ_UNUSE1,						//未使用1
    HQ_IN_RANGE_ALARM,				//进范围报警
    HQ_OUT_RANGE_ALARM,			//出范围报警
    HQ_OVER_SPEED_ALARM,			//超速报警
    HQ_DEVIATE_PATH_ALARM,			//偏离路线报警
    HQ_ILLEGAL_TIME_DRIVE_ALARM,	//非法时段行驶报警
    HQ_LESS_RESET_ALARM,			//停车休息时间不足报警
    HQ_OVERTIME_DRIVE_ALARM,		//疲劳驾驶报警
    HQ_ILLEGAL_OPEN_DOOR_ALARM,	//非法开车门
    HQ_ALARM_ON,					//设防
    HQ_CUT_LINE_ALARM,				//剪线报警
    HQ_BATTERY_LOW_ALARM,			//电瓶电压低报警
    HQ_CODE_ERROR_ALARM,			//密码错误报警
    HQ_ILLEGAL_DRIVE_ALARM,		//禁行报警，无
    HQ_ILLEGAL_PARK_ALARM,			//非法停车报警，无
    HQ_UNUSE2,						//未使用2
    HQ_UNUSE3,						//未使用3
    HQ_CAR_STATUS_MAX,
};

enum
{
	GL_STATE_LOGIN,
	GL_STATE_DATA,
	GL_STATE_LOGOUT,
	GL_STATE_SLEEP,
};

int32_t GL_MakeGPSInfo(uint8_t *Buf, Monitor_RecordStruct *Record)
{
	uint8_t StateByte[HQ_CAR_STATUS_MAX];
	uint8_t State[12];
	uint8_t i;
	int32_t GS;
	int32_t Mileage;
	int32_t GPSCN;
	int32_t Speed;
	int32_t Cog;
	Date_UserDataStruct Date;
	Time_UserDataStruct Time;
	memset(StateByte, 0, HQ_CAR_STATUS_MAX);
	memset(State, 0, 12);
	for(i = 0; i < ERROR_MAX; i++)
	{
		if (gSys.Error[i])
		{
			StateByte[HQ_PRIVATE_MODE_ON] = 1;
			break;
		}
	}
	StateByte[HQ_ACC_ON] = Record->IOValUnion.IOVal.ACC;
	StateByte[HQ_ALARM_ON] = gSys.nParam[PARAM_TYPE_ALARM2].Data.ParamDW.Param[PARAM_ALARM_ENABLE];
	StateByte[HQ_GPS_ERROR] = Record->DevStatus[MONITOR_STATUS_GPS_ERROR];
	StateByte[HQ_BUS_ERROR] = !Record->IOValUnion.IOVal.VCC;
	StateByte[HQ_GPS_STATUS_0] = Record->DevStatus[MONITOR_STATUS_ANT_SHORT];
	StateByte[HQ_GPS_STATUS_1] = Record->DevStatus[MONITOR_STATUS_ANT_BREAK];
	switch (Record->Alarm)
	{
	case ALARM_TYPE_CRASH:
		StateByte[HQ_CRASH_ALARM] = 1;
		break;
	case ALARM_TYPE_MOVE:
		StateByte[HQ_STEAL_ALARM] = 1;
		break;
	case ALARM_TYPE_CUTLINE:
		StateByte[HQ_CUT_LINE_ALARM] = 1;
		break;
	case ALARM_TYPE_OVERSPEED:
		StateByte[HQ_OVER_SPEED_ALARM] = 1;
		break;
	case ALARM_TYPE_LOWPOWER:
		StateByte[HQ_BATTERY_LOW_ALARM] = 1;
		break;
	}

	i = 0;
	while (i<HQ_CAR_STATUS_MAX) {
		State[i>>2] = 0x30 + ( StateByte[i] + (StateByte[i+1]<<1) + (StateByte[i+2]<<2) + (StateByte[i+3]<<3) );
		i += 4;
	}
	State[10]='\0';

	if (Record->GsensorVal > 9999)
	{
		GS = 9999;
	}
	else
	{
		GS = Record->GsensorVal;
	}

	Mileage = Record->MileageKM * 10 + Record->MileageM / 100;
	GPSCN = 0;
	for(i = 0; i < 4; i++)
	{
		GPSCN *= 10;
		if (Record->CN[i] <= 9)
		{
			GPSCN += Record->CN[i];
		}
		else
		{
			GPSCN += 9;
		}
	}
	if (Record->RMC.LocatStatus)
	{
		Speed = Record->RMC.Speed / 100;
		if (Speed > 2000)
		{
			Speed = 1999;
		}
	}
	else
	{
		Speed = 0;
	}

	Cog = Record->RMC.Cog / 1000;
	if (Cog > 360)
	{
		Cog = 360;
	}
	Tamp2UTC(UTC2Tamp(&Record->uDate.Date, &Record->uTime.Time) + 8 * 3600, &Date, &Time, 0);
	if (gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param[PARAM_MONITOR_ADD_MILEAGE])
	{
		sprintf(Buf, "&A%02u%02u%02u%02u%06d%03d%06d%c%02u%02u%02u%02u%02u&B%s&J0%04d&F%04d&H0%04d&E%08d",
				Time.Hour, Time.Min, Time.Sec, (int32_t)Record->RMC.LatDegree, (int32_t)Record->RMC.LatMin, (int32_t)Record->RMC.LgtDegree, (int32_t)Record->RMC.LgtMin,
				((((Record->RMC.LgtEW == 'E')?1:0)<<2) + (((Record->RMC.LatNS == 'N')?1:0)<<1) + (((Record->RMC.LocatStatus)?0:1)<<0))|0x30,
				Speed/20, Cog/10, Date.Day, Date.Mon, Date.Year - 2000, State, GS, Speed, GPSCN, Mileage);
	}
	else
	{
		sprintf(Buf, "&A%02u%02u%02u%02u%06d%03d%06d%c%02u%02u%02u%02u%02u&B%s&J0%04d&F%04d&H0%04d",
				Time.Hour, Time.Min, Time.Sec, (int32_t)Record->RMC.LatDegree, (int32_t)Record->RMC.LatMin, (int32_t)Record->RMC.LgtDegree, (int32_t)Record->RMC.LgtMin,
				((((Record->RMC.LgtEW == 'E')?1:0)<<2) + (((Record->RMC.LatNS == 'N')?1:0)<<1) + (((Record->RMC.LocatStatus)?0:1)<<0))|0x30,
				Speed/20, Cog/10, Date.Day, Date.Mon, Date.Year - 2000, State, GS, Speed, GPSCN);
	}
	return 0;
}

int32_t GL_MakeUploadInfo(const int8_t* Cmd, const int8_t *Param, const int8_t *GPSInfo, int8_t *Buf)
{
	return sprintf(Buf, "*HQ200%s,%s%s%s#", GleadCtrl.MonitorID.ucID, Cmd, Param, GPSInfo);
}

//*寻找给定字符在base数组中的位置的,使用了strrchr函数，寻找字符在字符串中最后一次的位置，由于总会存在并且仅存在一次，所以函数结果直接使用 */
 char find_pos(char ch)
{
//	char *ptr = (char*)strrchr(code_256_64, ch);//the last position (the only) in base[]
//	return (ptr - code_256_64);
	unsigned char i = 0;
	while (code_256_64[i]!=ch) {
		i++;
	}
	return i;
}

int32_t code64_to_256(unsigned char* data, int32_t data_len,unsigned char* ret)
{
	unsigned char temp[4] = {0, 0, 0, 0};
	unsigned char i = 0;
	if ( 1 == (data_len % 4) ) {
		DBG("!");
		return -1;
	}
	while (data_len > 3) {
		i++;
//		DBG_INFO("i = %u\r\n", i);
		temp[0] = find_pos(*(data));
		temp[1] = find_pos(*(data+1));
		temp[2] = find_pos(*(data+2));
		temp[3] = find_pos(*(data+3));
		*(ret) = (temp[0]<<2) + (temp[1]>>4);
		*(ret+1) = (temp[1]<<4) + (temp[2]>>2);
		*(ret+2) = (temp[2]<<6) + (temp[3]);
		data_len -= 4;
		data += 4;
		ret += 3;
	}
//	DBG_INFO("data_len = %u\r\n", data_len);
	if (data_len) {
		for (i=0;i<data_len;i++) {
			temp[i] = find_pos(*(data+i));
		}
		for (i=3;i>=data_len;i--) {
			temp[i] = 0;
		}
		*(ret) = (temp[0]<<2) + (temp[1]>>4);
		*(ret+1) = (temp[1]<<4) + (temp[2]>>2);
		*(ret+2) = (temp[2]<<6) + (temp[3]);
		ret += 3;
	}
	*(ret) = 0;
	return 0;

}

///*256码转换成64码函数，返回值为转码后数据的字节数 */
int32_t code256_to_64(const char* data, int32_t data_len, unsigned char* ret)
{
	uint8_t Pos, Bit, T = 0;
	uint32_t Len, i;
	Bit = 0;
	Len = 0;
	for(i = 0; i < data_len; i++)
	{
		switch (Bit)
		{
		case 0:
			Pos = data[i] >> 2;
			T = (data[i] & 0x03) << 4;
			Bit = 2;
			ret[Len++] = code_256_64[Pos];
			break;
		case 2:
			Pos = T + ((data[i] & 0xf0) >> 4);
			T = (data[i] & 0x0f) << 2;
			Bit = 4;
			ret[Len++] = code_256_64[Pos];
			break;
		case 4:
			Pos = T + ((data[i] & 0xC0) >> 6);
			ret[Len++] = code_256_64[Pos];
			Pos = (data[i] & 0x3f);
			ret[Len++] = code_256_64[Pos];
			Bit = 0;
			T = 0;
			break;
		default:
			return -1;
			break;
		}
	}
	if (Bit)
	{
		if (T < 64)
		{
			ret[Len++] = code_256_64[T];
		}
		else
		{
			DBG("%u", T);
			return -1;
		}
	}
	ret[Len] = 0;
	return Len;
}



void GL_NetAnalyze(void)
{
	uint8_t Key[2];
	uint8_t GPSInfo[100];
	uint8_t *DigitalIn;
	uint8_t *DigitalReturn;
	uint8_t *DataStart;
	uint32_t DataLen;
	Monitor_RecordStruct Record;
	memcpy(Key, GleadCtrl.AnalyzeBuf + 7, 2);
	DataStart = GleadCtrl.AnalyzeBuf + 9;
	if (GleadCtrl.AnalzeLen > 10)
	{
		DataLen = GleadCtrl.AnalzeLen - 10;
	}
	else
	{
		return ;
	}

	if (DataLen > 1024)
		return ;

	switch (Key[0])
	{
	case 'B':
		switch (Key[1])
		{
		case 'E':
			Monitor_Record(&Record);
			GL_MakeGPSInfo(GPSInfo, &Record);
			GL_MakeUploadInfo("YBE", "", GPSInfo, GleadCtrl.TempBuf);
			Monitor_RecordResponse(GleadCtrl.TempBuf, strlen(GleadCtrl.TempBuf));
			break;
		}
		break;
	case 'C':
		switch (Key[1])
		{
		case 'A':
			DigitalReturn = COS_MALLOC(1024);
			DigitalIn = COS_MALLOC(1024);
			memset(DigitalReturn, 0, 1024);
			memset(DigitalIn, 0, 1024);
			if (code64_to_256(DataStart, DataLen, DigitalReturn) >= 0)
			{
				DBG("%s", DigitalReturn);
				LV_SMSAnalyze(DigitalReturn, strlen(DigitalReturn), DigitalIn, &DataLen);
				if (!DataLen)
				{
					strcpy(DigitalIn, "error");
					DataLen = 5;
				}
				if (code256_to_64(DigitalIn, DataLen, DigitalReturn) < 0)
				{
					COS_FREE(DigitalIn);
					COS_FREE(DigitalReturn);
					break;
				}
			}
			else
			{
				COS_FREE(DigitalIn);
				COS_FREE(DigitalReturn);
				break;
			}
			Monitor_Record(&Record);
			GL_MakeGPSInfo(GPSInfo, &Record);
			GL_MakeUploadInfo("CD", DigitalReturn, GPSInfo, GleadCtrl.TempBuf);
			Monitor_RecordResponse(GleadCtrl.TempBuf, strlen(GleadCtrl.TempBuf));
			COS_FREE(DigitalIn);
			COS_FREE(DigitalReturn);
			break;
		}
		break;
	}
}

uint8_t GL_Connect(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, int8_t *Url)
{
	uint8_t ProcessFinish = 0;
	Led_Flush(LED_TYPE_GSM, LED_FLUSH_SLOW);

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
		Led_Flush(LED_TYPE_GSM, LED_FLUSH_SLOW);
		if (Net->SocketID != INVALID_SOCKET)
		{
			Net_Disconnect(Net);
		}
		ProcessFinish = 0;
	}
	else
	{
		if (gSys.RecordCollect.IsRunMode)
		{
			Led_Flush(LED_TYPE_GSM, LED_ON);
		}
		else
		{
			Led_Flush(LED_TYPE_GSM, LED_OFF);
		}
		ProcessFinish = 1;
	}
	return ProcessFinish;
}

uint8_t GL_Send(Monitor_CtrlStruct *Monitor, Net_CtrlStruct *Net, uint32_t Len)
{
	Led_Flush(LED_TYPE_GSM, LED_FLUSH_FAST);
	Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
	if (Len > 1)
	{
		DBG("%u %s", Len, Monitor->TxBuf);
	}
	Net_Send(Net, Monitor->TxBuf, Len);
	if (Net->Result != NET_RES_SEND_OK)
	{
		Led_Flush(LED_TYPE_GSM, LED_FLUSH_SLOW);
		return 0;
	}
	else
	{
		if (gSys.RecordCollect.IsRunMode)
		{
			Led_Flush(LED_TYPE_GSM, LED_ON);
		}
		else
		{
			Led_Flush(LED_TYPE_GSM, LED_OFF);
		}
		return 1;
	}
}


int32_t GL_ReceiveAnalyze(void *pData)
{
	uint32_t RxLen = (uint32_t)pData;
	uint32_t FinishLen = 0,i;
	int32_t Error;
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

		Error = (int32_t)OS_SocketReceive(GleadCtrl.Net.SocketID, GleadCtrl.RxBuf, FinishLen, NULL, NULL);
		if (Error <= 0)
		{
			DBG("%d", Error);
			return -1;
		}
		RxLen -= (uint32_t)Error;

		for(i = 0; i < FinishLen; i++)
		{
			if (GleadCtrl.AnalzeLen)
			{
				GleadCtrl.AnalyzeBuf[GleadCtrl.AnalzeLen] = GleadCtrl.RxBuf[i];
				GleadCtrl.AnalzeLen++;
				if (GleadCtrl.RxBuf[i] == '#')
				{
					GleadCtrl.AnalyzeBuf[GleadCtrl.AnalzeLen] = 0;
					DBG("%s", GleadCtrl.AnalyzeBuf);
					if (!memcmp(GleadCtrl.AnalyzeBuf, "*HQ20", 5))
					{
						GL_NetAnalyze();
					}
					GleadCtrl.AnalzeLen = 0;
				}
				else if (GleadCtrl.AnalzeLen > 1400)
				{
					GleadCtrl.AnalzeLen = 0;
				}
				else if (GleadCtrl.RxBuf[i] == '*')
				{
					GleadCtrl.AnalyzeBuf[0] = '*';
					GleadCtrl.AnalzeLen = 1;
				}
			}
			else
			{
				if (GleadCtrl.RxBuf[i] == '*')
				{
					GleadCtrl.AnalyzeBuf[0] = '*';
					GleadCtrl.AnalzeLen = 1;
				}
			}
		}

		if (RxLen)
			DBG("rest %u", RxLen);
	}
	return 0;
}

void GL_Task(void *pData)
{
	Monitor_CtrlStruct *Monitor = &GleadCtrl;
	Net_CtrlStruct *Net = &GleadCtrl.Net;
	//Param_UserStruct *User = &gSys.nParam[PARAM_TYPE_USER].Data.UserInfo;
	Param_MainStruct *MainInfo = &gSys.nParam[PARAM_TYPE_MAIN].Data.MainInfo;
	uint32_t SleepTime = 0;
	uint32_t KeepTime;
	uint8_t ErrorOut = 0;
	COS_EVENT Event;
	uint32_t TxLen = 0;
	uint8_t DataType = 0;
	uint8_t LoginFlag = 0;
	uint8_t First = 1;
	Monitor_RecordStruct MonitorData;
	IO_ValueUnion uIO;
//下面变量为每个协议独有的
	DBG("Task start! %u %u %u %u %u %u %u %u %u" ,
			Monitor->Param[PARAM_GS_WAKEUP_MONITOR], Monitor->Param[PARAM_GS_JUDGE_RUN],
			Monitor->Param[PARAM_UPLOAD_RUN_PERIOD], Monitor->Param[PARAM_UPLOAD_STOP_PERIOD],
			Monitor->Param[PARAM_UPLOAD_HEART_PERIOD], Monitor->Param[PARAM_MONITOR_NET_TO],
			Monitor->Param[PARAM_MONITOR_KEEP_TO], Monitor->Param[PARAM_MONITOR_SLEEP_TO],
			Monitor->Param[PARAM_MONITOR_RECONNECT_MAX]);
	sprintf(Monitor->MonitorID.ucID, "%02u%09u", (int32_t)MainInfo->UID[1], (int32_t)MainInfo->UID[0]);
    DBG("monitor id %s", Monitor->MonitorID.ucID);

    KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
    gSys.State[MONITOR_STATE] = GL_STATE_LOGIN;

    while (!ErrorOut)
    {
		if (gSys.RecordCollect.WakeupFlag || (gSys.State[CRASH_STATE] > ALARM_STATE_IDLE) || (gSys.State[MOVE_STATE] > ALARM_STATE_IDLE))
		{
			KeepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_KEEP_TO];
		}

    	if (gSys.RecordCollect.IsWork && Monitor->Param[PARAM_MONITOR_KEEP_TO])
    	{
    		uIO.Val = gSys.Var[IO_VAL];
    		if (!uIO.IOVal.VCC && (gSys.Var[SYS_TIME] > KeepTime))
    		{
    			DBG("sleep!");
    			gSys.RecordCollect.WakeupFlag = 0;

    			gSys.State[MONITOR_STATE] = GL_STATE_LOGOUT;
    			gSys.RecordCollect.IsWork = 0;
    			SleepTime = gSys.Var[SYS_TIME] + Monitor->Param[PARAM_MONITOR_SLEEP_TO];

    		}
    	}

    	switch (gSys.State[MONITOR_STATE])
    	{

    	case GL_STATE_LOGIN:
    		gSys.RecordCollect.IsWork = 1;
    		Net->TCPPort = MainInfo->TCPPort;
    		Net->UDPPort = MainInfo->UDPPort;
    		if (Monitor->ReConnCnt)
    		{
    			Net->To = Monitor->ReConnCnt * 15;
    			Net_WaitTime(Net);
    		}
    		else if (First)
    		{
    			First = 0;
    		}
    		else
    		{
    			Net->To = 3;
    			Net_WaitTime(Net);
    		}

    		if (GL_Connect(Monitor, Net, MainInfo->MainURL))
    		{
    			if (!LoginFlag)
    			{
					Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
					Monitor_Record(&MonitorData);
					GL_MakeGPSInfo(Monitor->TempBuf, &MonitorData);
					GL_MakeUploadInfo("AB", "1", Monitor->TempBuf, Monitor->TxBuf);
					if (GL_Send(Monitor, Net, strlen(Monitor->TxBuf)))
					{
						Monitor->ReConnCnt = 0;
						LoginFlag = 1;
						gSys.State[MONITOR_STATE] = GL_STATE_DATA;
						break;
					}
    			}
    			else
    			{
    				Monitor->ReConnCnt = 0;
					gSys.State[MONITOR_STATE] = GL_STATE_DATA;
					break;
    			}
    		}
    		else
    		{
    			Monitor->ReConnCnt++;
    			if (Monitor->ReConnCnt == 2)
    			{
    				GPRS_Restart();
    			}
    			if (Monitor->ReConnCnt >= Monitor->Param[PARAM_MONITOR_RECONNECT_MAX])
    			{
    				DBG("too much retry!");
    				ErrorOut = 1;
    			}
    		}
    		break;
    	case GL_STATE_DATA:

    		if (Net->Heart)
			{
				//合成心跳包
				Net->Heart = 0;
				if (!gSys.RecordCollect.IsRunMode)
				{
					Monitor->TxBuf[0] = 0x00;
					GL_Send(Monitor, Net, 1);
					if (Net->Result != NET_RES_SEND_OK)
					{
						gSys.State[MONITOR_STATE] = GL_STATE_LOGIN;
						break;
					}
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
        			GL_MakeGPSInfo(Monitor->TempBuf, &Monitor->Record);
        			GL_MakeUploadInfo("BA", "", Monitor->TempBuf, Monitor->TxBuf);

    			}
    			else if (Monitor_GetCacheLen(CACHE_TYPE_DATA))
    			{
    				DataType = CACHE_TYPE_DATA;
    				Monitor_ExtractData(&Monitor->Record);
        			GL_MakeGPSInfo(Monitor->TempBuf, &Monitor->Record);
        			GL_MakeUploadInfo("BA", "", Monitor->TempBuf, Monitor->TxBuf);
    			}

				GL_Send(Monitor, Net, strlen(Monitor->TxBuf));

				if (Net->Result != NET_RES_SEND_OK)
				{

					gSys.State[MONITOR_STATE] = GL_STATE_LOGIN;
					break;
				}
				else
				{
					Monitor_DelCache(DataType, 0);
				}
    		}
    		else
    		{
    			gSys.RecordCollect.WakeupFlag = 0;
    			if (KeepTime > gSys.Var[SYS_TIME])
    			{
    				Net->To = (KeepTime - gSys.Var[SYS_TIME]) + 2;
    			}
    			else
    			{
    				Net->To = 10;
    			}

    			Net_WaitEvent(Net);
    			if (Net->Result == NET_RES_ERROR)
    			{
    				DBG("error!");
    				gSys.State[MONITOR_STATE] = GL_STATE_LOGIN;
    			}
    		}



			if (Monitor->DevCtrlStatus && !Monitor_GetCacheLen(CACHE_TYPE_ALL))
			{
				SYS_Reset();
			}
    		break;
    	case GL_STATE_LOGOUT:
    		LoginFlag = 0;
			Net->To = Monitor->Param[PARAM_MONITOR_NET_TO];
			//发送认证
			Monitor_Record(&MonitorData);
			GL_MakeGPSInfo(Monitor->TempBuf, &MonitorData);
			GL_MakeUploadInfo("AC", "1", Monitor->TempBuf, Monitor->TxBuf);
			GL_Send(Monitor, Net, strlen(Monitor->TxBuf));
			Net->To = 15;
			Net_Disconnect(Net);
			gSys.State[MONITOR_STATE] = GL_STATE_SLEEP;
			Led_Flush(LED_TYPE_GSM, LED_OFF);
    		break;
    	case GL_STATE_SLEEP:
    		if (Monitor->Param[PARAM_MONITOR_SLEEP_TO])
    		{
    			Net->To = Monitor->Param[PARAM_MONITOR_SLEEP_TO] + 2;
    		}
    		else
    		{
    			Net->To = 43200;
    		}

    		Net_WaitEvent(Net);
    		if (gSys.RecordCollect.WakeupFlag)
    		{
    			DBG("alarm or vacc wakeup!");
    			gSys.State[MONITOR_STATE] = GL_STATE_LOGIN;

    		}
    		if (Monitor->Param[PARAM_MONITOR_SLEEP_TO])
    		{
        		if (gSys.Var[SYS_TIME] > SleepTime)
        		{
        			DBG("time wakeup!");
        			gSys.State[MONITOR_STATE] = GL_STATE_LOGIN;
        		}
    		}
    		break;

    	default:
    		gSys.State[MONITOR_STATE] = GL_STATE_LOGIN;
    		break;
    	}

    }
	SYS_Reset();
	while (1)
	{
		COS_WaitEvent(Net->TaskID, &Event, COS_WAIT_FOREVER);
	}

}

void GL_Config(void)
{
	gSys.TaskID[MONITOR_TASK_ID] = COS_CreateTask(GL_Task, NULL,
				NULL, MMI_TASK_MAX_STACK_SIZE, MMI_TASK_PRIORITY + MONITOR_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI GL Task");
	GleadCtrl.Param = gSys.nParam[PARAM_TYPE_MONITOR].Data.ParamDW.Param;

	GleadCtrl.Net.SocketID = INVALID_SOCKET;
	GleadCtrl.Net.TaskID = gSys.TaskID[MONITOR_TASK_ID];
	GleadCtrl.Net.Channel = GPRS_CH_MAIN_MONITOR;
	GleadCtrl.Net.TimerID = MONITOR_TIMER_ID;
	GleadCtrl.Net.ReceiveFun = GL_ReceiveAnalyze;
	GleadCtrl.AnalzeLen = 0;

	if (!GleadCtrl.Param[PARAM_UPLOAD_RUN_PERIOD])
	{
		GleadCtrl.Param[PARAM_UPLOAD_RUN_PERIOD] = 30;
	}
}
#endif
