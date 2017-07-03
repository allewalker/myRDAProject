#include "user.h"
#define COM_UART hwp_uart
#define COM_UART_ID HAL_UART_1
#define USP_MODE_TO (30)

enum
{
	COM_STATE_DEV,
	COM_STATE_SLIP_IN,
	COM_STATE_SLIP_RUN,
	COM_STATE_SLIP_OUT,
};
COM_CtrlStruct __attribute__((section (".usr_ram"))) COMCtrl;
void COM_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error);
void COM_Sleep(void)
{
	COMCtrl.SleepFlag = 1;
	if (COMCtrl.TxBusy)
	{
		return ;
	}
	OS_UartClose(COM_UART_ID);
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_sysCtrl->Cfg_Reserved &= ~SYS_CTRL_UART1_TCO;
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	hwp_iomux->pad_GPIO_0_cfg = IOMUX_PAD_GPIO_0_SEL_FUN_GPIO_0_SEL;
	hwp_iomux->pad_GPIO_1_cfg = IOMUX_PAD_GPIO_1_SEL_FUN_GPIO_1_SEL;
#endif
}

void COM_CalTo(void)
{
	if (COMCtrl.CurrentBR <= 115200)
	{
		COMCtrl.To = 32;
	}
	else if (COMCtrl.CurrentBR <= 230400)
	{
		COMCtrl.To = 64;
	}
	else if (COMCtrl.CurrentBR <= 460800)
	{
		COMCtrl.To = 128;
	}
	else
	{
		COMCtrl.To = 256;
	}
	DBG("%d %d", COMCtrl.CurrentBR, COMCtrl.To);
}

void COM_Wakeup(u32 BR)
{
	HAL_UART_CFG_T uartCfg;
	HAL_UART_IRQ_STATUS_T mask =
	{
		.txModemStatus          = 0,
		.rxDataAvailable        = 1,
		.txDataNeeded           = 0,
		.rxTimeout              = 0,
		.rxLineErr              = 0,
		.txDmaDone              = 1,
		.rxDmaDone              = 0,
		.rxDmaTimeout           = 0,
		.DTR_Rise				= 0,
		.DTR_Fall				= 0,
	};

	if (!COMCtrl.SleepFlag)
		return ;
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_sysCtrl->Cfg_Reserved |= SYS_CTRL_UART1_TCO;
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	hwp_iomux->pad_GPIO_0_cfg = IOMUX_PAD_GPIO_0_SEL_FUN_UART1_RXD_SEL;
	hwp_iomux->pad_GPIO_1_cfg = IOMUX_PAD_GPIO_1_SEL_FUN_UART1_TXD_SEL;
#endif

	uartCfg.afc = HAL_UART_AFC_MODE_DISABLE;
	uartCfg.data = HAL_UART_8_DATA_BITS;
	uartCfg.irda = HAL_UART_IRDA_MODE_DISABLE;
	uartCfg.parity = HAL_UART_NO_PARITY;
	uartCfg.stop = HAL_UART_1_STOP_BIT;
	uartCfg.rx_mode = HAL_UART_TRANSFERT_MODE_DIRECT_IRQ;
	uartCfg.rx_trigger = HAL_UART_RX_TRIG_1;
	uartCfg.tx_mode = HAL_UART_TRANSFERT_MODE_DMA_IRQ;
	uartCfg.tx_trigger = HAL_UART_TX_TRIG_EMPTY;
	uartCfg.rate = BR;
	DBG("%d", uartCfg.rate);
	OS_UartOpen(COM_UART_ID, &uartCfg, mask, COM_IRQHandle);
	COM_UART->status = UART_ENABLE;
	COM_UART->CMD_Set = UART_RX_FIFO_RESET|UART_TX_FIFO_RESET;
	COMCtrl.SleepFlag = 0;
	COMCtrl.CurrentBR = BR;
	COMCtrl.NeedRxLen = 0;
	COMCtrl.ProtocolType = COM_PROTOCOL_NONE;
	COM_CalTo();

}

void COM_Reset(void)
{
	COMCtrl.ProtocolType = COM_PROTOCOL_NONE;
	COMCtrl.RxPos = 0;
	COMCtrl.NeedRxLen = 0;
	OS_StopTimer(gSys.TaskID[COM_TASK_ID], COM_RX_TIMER_ID);
}

void COM_RxFinish(void)
{
	if (COMCtrl.RxPos)
	{
//		DBG("%d %d", COMCtrl.NeedRxLen, COMCtrl.RxPos);
//		if (COMCtrl.RxPos < 32)
//		{
//			__HexTrace(COMCtrl.RxBuf, COMCtrl.RxPos);
//		}
		COMCtrl.AnalyzeLen = COMCtrl.RxPos;
		memcpy(COMCtrl.AnalyzeBuf, COMCtrl.RxBuf, COMCtrl.AnalyzeLen);
	}
	else
	{
		COMCtrl.AnalyzeLen = 0;
	}

}

void COM_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error)
{
	u8 Temp;
	u8 i;
	u32 TxLen;
	if (Status.txDmaDone)
	{
		COMCtrl.TxBusy = 0;
		if (COMCtrl.SleepFlag)
		{
			OS_UartClose(COM_UART_ID);
		}
		else
		{
			COM_Send(NULL, 0);
		}

	}
	if (Status.rxDataAvailable)
	{
		OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_RX_TIMER_ID, COS_TIMER_MODE_PERIODIC, SYS_TICK/COMCtrl.To);

		while(COM_UART->status & UART_RX_FIFO_LEVEL_MASK)
		{
			Temp = COM_UART->rxtx_buffer;
			if (!COMCtrl.ProtocolType)
			{

				if (USP_CheckHead(Temp))
				{
					COMCtrl.ProtocolType = COM_PROTOCOL_USP;
					COMCtrl.RxBuf[0] = Temp;
					COMCtrl.RxPos = 1;
					break;
				}
				else if (LV_CheckHead(Temp))
				{
					COMCtrl.ProtocolType = COM_PROTOCOL_LV;
					COMCtrl.RxBuf[0] = Temp;
					COMCtrl.RxPos = 1;
					break;
				}
#if (__CUST_CODE__ == __CUST_LY__)
				else if (LY_CheckUartHead(Temp))

#elif (__CUST_CODE__ == __CUST_KQ__)
				else if (KQ_CheckUartHead(Temp))
#elif (__CUST_CODE__ == __CUST_LB__)
				else if (LB_CheckUartHead(Temp))
#else
				else if (0)
#endif
				{
					COMCtrl.ProtocolType = COM_PROTOCOL_DEV;
					COMCtrl.RxBuf[0] = Temp;
					COMCtrl.RxPos = 1;
				}

			}
			else
			{
				COMCtrl.RxBuf[COMCtrl.RxPos++] = Temp;
				if (COMCtrl.NeedRxLen)
				{
					if (COMCtrl.RxPos >= COMCtrl.NeedRxLen)
					{
						COM_RxFinish();
						if (COM_PROTOCOL_USP == COMCtrl.ProtocolType)
						{
							OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_ANALYZE, COMCtrl.ProtocolType, 0, 0);
						}
						else
						{

						}
						COM_Reset();
						COM_UART->triggers &= ~(0x0000001F);
						COM_UART->triggers |= HAL_UART_RX_TRIG_1;
					}
					else
					{
						if ( (COMCtrl.NeedRxLen - COMCtrl.RxPos) > HAL_UART_RX_TRIG_HALF)
						{
							COM_UART->triggers &= ~(0x0000001F);
							COM_UART->triggers |= HAL_UART_RX_TRIG_HALF;
						}
						else
						{
							COM_UART->triggers &= ~(0x0000001F);
							COM_UART->triggers |= HAL_UART_RX_TRIG_1;
						}
					}
				}
				else
				{
					if (COM_PROTOCOL_USP == COMCtrl.ProtocolType)
					{
						if (COMCtrl.RxPos >= 10)
						{
							COMCtrl.NeedRxLen = USP_CheckLen(COMCtrl.RxBuf);
							if (!COMCtrl.NeedRxLen)
							{
								DBG("!");
								COMCtrl.ProtocolType = COM_PROTOCOL_NONE;
								COMCtrl.RxPos = 0;
							}
							else
							{
								if ( (COMCtrl.NeedRxLen - COMCtrl.RxPos) > HAL_UART_RX_TRIG_HALF)
								{
									COM_UART->triggers &= ~(0x0000001F);
									COM_UART->triggers |= HAL_UART_RX_TRIG_HALF;
								}
								else if (COMCtrl.NeedRxLen <= COMCtrl.RxPos)
								{
									COM_RxFinish();
									OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_ANALYZE, COMCtrl.ProtocolType, 0, 0);
									COM_Reset();
								}
								else
								{
									COM_UART->triggers &= ~(0x0000001F);
									COM_UART->triggers |= HAL_UART_RX_TRIG_1;
								}
							}

						}
					}
				}
			}
		}
	}

	COM_UART->status = UART_ENABLE;

}

u8 COM_Send(u8 *Data, u32 Len)
{
	u32 TxLen;
	if (COMCtrl.TxBusy)
	{
		return 0;
	}
	if (COMCtrl.SleepFlag)
	{
		return 0;
	}

	if (Len > COM_BUF_LEN)
	{
		Len = COM_BUF_LEN;
	}
	if (!Data)
	{
		TxLen = ReadRBuffer(&COMCtrl.rTxBuf, COMCtrl.DMABuf, COM_BUF_LEN);
	}
	else if (Data != COMCtrl.DMABuf)
	{
		memcpy(COMCtrl.DMABuf, Data, Len);
		TxLen = Len;
	}
	else if (Data == COMCtrl.DMABuf)
	{
		TxLen = Len;
	}
	if (!TxLen)
	{
#ifdef __UART_485_MODE__
		GPIO_Write(DIR_485_PIN, 0);
#endif
		return 0;
	}
#ifdef __UART_485_MODE__
	GPIO_Write(DIR_485_PIN, 1);
#endif
	COMCtrl.TxBusy = 1;
	if (PRINT_TEST != gSys.State[PRINT_STATE])
	{
		DBG("%d", TxLen);
		if (TxLen <= 64)
		{
			__HexTrace(COMCtrl.DMABuf, TxLen);
		}
	}
	//SYS_Waketup();
	if (HAL_UNKNOWN_CHANNEL == OS_UartDMASend(HAL_IFC_UART1_TX, COMCtrl.DMABuf, TxLen))
	{
		COMCtrl.TxBusy = 0;
		DBG("Tx fail!");
#ifdef __UART_485_MODE__
		GPIO_Write(DIR_485_PIN, 0);
#endif
		OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_TX_REQ, 0, 0, 0);
		return 0;
	}
	return 1;
}

void COM_Task(void *pData)
{
	COS_EVENT Event;
	u32 LastTime;
	u32 TxLen = 0;
	u8 Temp, i, j, k;

	DBG("Task start! %d", gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);

	COM_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);

    while(1)
    {
    	COS_WaitEvent(gSys.TaskID[COM_TASK_ID], &Event, COS_WAIT_FOREVER);
    	switch (Event.nEventId)
    	{
    	case EV_TIMER:
    		switch (Event.nParam1)
    		{
    		case COM_MODE_TIMER_ID:
    			if (COMCtrl.CurrentBR != gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR])
    			{
    				OS_UartClose(COM_UART_ID);
    				COMCtrl.SleepFlag = 1;
    				COM_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);
    			}
    			gSys.State[PRINT_STATE] = PRINT_NORMAL;
    			break;
    		case COM_RX_TIMER_ID:
    			DBG("!");
				while(COM_UART->status & UART_RX_FIFO_LEVEL_MASK)
				{
					Temp = COM_UART->rxtx_buffer;
					COMCtrl.RxBuf[COMCtrl.RxPos++] = Temp;
				}
				COM_RxFinish();
    			switch (COMCtrl.ProtocolType)
    			{
    			case COM_PROTOCOL_DEV:
    				OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_COM_TO_USER, 0, &COMCtrl.AnalyzeBuf, COMCtrl.AnalyzeLen);
    				break;
    			case COM_PROTOCOL_LV:
    				COMCtrl.AnalyzeBuf[COMCtrl.AnalyzeLen] = 0;
    				DBG("%s", COMCtrl.AnalyzeBuf);
    				LV_ComAnalyze(&COMCtrl);
    				if (PRINT_TEST == gSys.State[PRINT_STATE])
    				{
    					OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_MODE_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK * 900);
    				}
    				break;
    			case COM_PROTOCOL_USP:
    				//USP协议超时
    				TxLen = USP_Analyze(COMCtrl.AnalyzeBuf, COMCtrl.AnalyzeLen, COMCtrl.TempBuf);
    				__HexTrace(COMCtrl.TempBuf, TxLen);
    				COM_Tx(COMCtrl.TempBuf, TxLen);
    				OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_MODE_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK * USP_MODE_TO);
    				break;
    			}
    			COM_Reset();
    			break;
	    	default:
	    		OS_StopTimer(gSys.TaskID[COM_TASK_ID], Event.nParam1);
	    		break;
    		}
    		break;

    	case EV_MMI_COM_ANALYZE:

			switch (Event.nParam1)
			{
			case COM_PROTOCOL_USP:
				TxLen = USP_Analyze(COMCtrl.AnalyzeBuf, COMCtrl.AnalyzeLen, COMCtrl.TempBuf);
				//__HexTrace(COMCtrl.TempBuf, TxLen);
				COM_Tx(COMCtrl.TempBuf, TxLen);
				OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_MODE_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK * USP_MODE_TO);
				break;
			}
			break;
		case EV_MMI_COM_TX_REQ:
			break;
		case EV_MMI_COM_NEW_BR:
			DBG("new br %d", Event.nParam1);
			COM_Sleep();
			OS_Sleep(SYS_TICK/128);
			COMCtrl.SleepFlag = 1;
			COM_Wakeup(Event.nParam1);
			OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_MODE_TIMER_ID, COS_TIMER_MODE_SINGLE, SYS_TICK * USP_MODE_TO);
			break;
    	}
		COM_Send(NULL, 0);
    }
}

void Uart_Config(void)
{
	InitRBuffer(&COMCtrl.rTxBuf, COMCtrl.TxBuf, COM_BUF_LEN * 2, 1);
//低功耗模式下，串口休眠
	gSys.TaskID[COM_TASK_ID] = COS_CreateTask(COM_Task, NULL,
			NULL, MMI_TASK_MAX_STACK_SIZE, MMI_TASK_PRIORITY + COM_TASK_ID, COS_CREATE_DEFAULT, 0, "MMI COM Task");
	COMCtrl.SleepFlag = 1;

}

void COM_TxReq(u8 *Data, u32 Len)
{
	WriteRBufferForce(&COMCtrl.rTxBuf, Data, Len);
	OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_TX_REQ, 0, 0, 0);
}

void COM_Tx(u8 *Data, u32 Len)
{
	WriteRBufferForce(&COMCtrl.rTxBuf, Data, Len);
}
