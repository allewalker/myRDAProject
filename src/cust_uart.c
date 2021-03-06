#include "user.h"
#define COM_UART hwp_uart
#define COM_UART_ID HAL_UART_1
#define USP_MODE_TO (5)

#define COM_UART_DMA_TX HAL_IFC_UART1_TX
#define COM_UART_DMA_RX	HAL_IFC_UART1_RX
enum
{
	COM_STATE_DEV,
	COM_STATE_SLIP_IN,
	COM_STATE_SLIP_RUN,
	COM_STATE_SLIP_OUT,
};
COM_CtrlStruct __attribute__((section (".usr_ram"))) COMCtrl;

#ifdef __IRQ_CB_WITH_PARAM__
void COM_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error, UINT32 Param);
#else
void COM_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error);
#endif

uint8_t COM_SleepReq(uint8_t Req)
{
	//COMCtrl.SleepWaitCnt = USP_MODE_TO * 3;
	if (COMCtrl.SleepReq != Req)
	{
		COMCtrl.SleepReq = Req;
		DBG("%d", COMCtrl.SleepReq);
		if (!COMCtrl.SleepReq)
		{
			COM_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);
		}
	}
	return COMCtrl.SleepReq;
}

void COM_StateCheck(void)
{
	if (COMCtrl.SleepReq)
	{
		if (COMCtrl.SleepWaitCnt)
		{
			COMCtrl.SleepWaitCnt--;
		}
		else
		{
			if (!COMCtrl.SleepFlag)
			{
				COM_Sleep();
			}
		}
	}
}

void COM_Sleep(void)
{
	if (PRINT_NORMAL != gSys.State[PRINT_STATE])
	{
		DBG("uart have job, can not sleep!");
		return ;
	}
	if (COMCtrl.SleepFlag)
	{
		DBG("allready sleep!");
		return ;
	}
	COMCtrl.SleepFlag = 1;
	DBG("!");
	OS_UartClose(COM_UART_ID);
#ifdef __UART_485_MODE__
	GPIO_Write(DIR_485_PIN, 0);
#else
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8809)
	hwp_sysCtrl->Cfg_Reserved &= ~SYS_CTRL_UART1_TCO;
#endif
#if (CHIP_ASIC_ID == CHIP_ASIC_ID_8955)
	hwp_iomux->pad_GPIO_0_cfg = IOMUX_PAD_GPIO_0_SEL_FUN_GPIO_0_SEL;
	hwp_iomux->pad_GPIO_1_cfg = IOMUX_PAD_GPIO_1_SEL_FUN_GPIO_1_SEL;
#endif
#endif
}

void COM_Wakeup(uint32_t BR)
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
	DBG("%u", uartCfg.rate);
	OS_UartOpen(COM_UART_ID, &uartCfg, mask, COM_IRQHandle);
	COM_UART->status = UART_ENABLE;
	COM_UART->CMD_Set = UART_RX_FIFO_RESET|UART_TX_FIFO_RESET;
	COMCtrl.SleepFlag = 0;
	COMCtrl.CurrentBR = BR;
	COMCtrl.NeedRxLen = 0;
	COMCtrl.ProtocolType = COM_PROTOCOL_NONE;

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
//		DBG("%u %u", COMCtrl.NeedRxLen, COMCtrl.RxPos);
//		if (COMCtrl.RxPos < 32)
//		{
//			HexTrace(COMCtrl.RxBuf, COMCtrl.RxPos);
//		}
		COMCtrl.AnalyzeLen = COMCtrl.RxPos;
		COMCtrl.RxPos = 0;
		memcpy(COMCtrl.AnalyzeBuf, COMCtrl.RxBuf, COMCtrl.AnalyzeLen);
	}
	else
	{
		COMCtrl.AnalyzeLen = 0;
	}

}

#ifdef __IRQ_CB_WITH_PARAM__
void COM_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error, UINT32 Param)
#else
void COM_IRQHandle(HAL_UART_IRQ_STATUS_T Status, HAL_UART_ERROR_STATUS_T Error)
#endif
{
	uint8_t Temp;
	HAL_UART_IRQ_STATUS_T mask =
	{
		.txModemStatus          = 0,
		.rxDataAvailable        = 0,
		.txDataNeeded           = 0,
		.rxTimeout              = 0,
		.rxLineErr              = 0,
		.txDmaDone              = 1,
		.rxDmaDone              = 0,
		.rxDmaTimeout           = 1,
		.DTR_Rise				= 0,
		.DTR_Fall				= 0,
	};
	if (Status.txDmaDone)
	{
		COMCtrl.TxBusy = 0;
		OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_TX_REQ, 0, 0, 0);
	}
	if (Status.rxDataAvailable)
	{
		OS_StopTimer(gSys.TaskID[COM_TASK_ID], COM_RX_TIMER_ID);
		while(COM_UART->status & UART_RX_FIFO_LEVEL_MASK)
		{
			Temp = COM_UART->rxtx_buffer;
			COMCtrl.RxBuf[COMCtrl.RxPos] = Temp;
			COMCtrl.RxPos++;
		}
		hal_UartIrqSetMask(COM_UART_ID, mask);
		COM_UART->irq_cause = UART_RX_DMA_TIMEOUT;
		COMCtrl.LastRxDMABufSn = (COMCtrl.LastRxDMABufSn + 1) % DMA_BUF_BLOCK;
		COMCtrl.RxDMABuf = &COMCtrl.RxDMABufP[COMCtrl.LastRxDMABufSn][0];
		COMCtrl.RxDMAChannel = OS_DMAStart(COM_UART_DMA_RX, COMCtrl.RxDMABuf, DMA_BUF_LEN, HAL_IFC_SIZE_8_MODE_MANUAL);
		if (COMCtrl.RxDMAChannel == HAL_UNKNOWN_CHANNEL)
		{
			mask.rxDataAvailable = 1;
			hal_UartIrqSetMask(COM_UART_ID, mask);
			DBG("!!!");
		}
	}

	if (Status.rxDmaTimeout)
	{
		if (0 == (hwp_sysIfc->std_ch[COMCtrl.RxDMAChannel].status & SYS_IFC_FIFO_EMPTY))
		{
			hwp_sysIfc->std_ch[COMCtrl.RxDMAChannel].control |= SYS_IFC_FLUSH;
		}
		OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_ANALYZE, 0, 0, 0);

		if (COMCtrl.CurrentBR < 115200)
		{
			OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_RX_TIMER_ID, COS_TIMER_MODE_PERIODIC, SYS_TICK/32);
		}
		else if (COMCtrl.CurrentBR < 230400)
		{
			OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_RX_TIMER_ID, COS_TIMER_MODE_PERIODIC, SYS_TICK/64);
		}
		else
		{
			OS_StartTimer(gSys.TaskID[COM_TASK_ID], COM_RX_TIMER_ID, COS_TIMER_MODE_PERIODIC, SYS_TICK/128);
		}

	}
	COM_UART->status = UART_ENABLE;

}

uint8_t COM_Send(uint8_t *Data, uint32_t Len)
{
	uint32_t TxLen;
	if (COMCtrl.TxBusy)
	{
		return 0;
	}
	if (COMCtrl.SleepFlag)
	{
		COM_Wakeup(gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);
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
		if (COMCtrl.Mode485Tx && !COMCtrl.Mode485TxDone)
		{
			COMCtrl.Mode485TxDone = 1;
			while ((COM_UART->status & UART_TX_ACTIVE) ||
		            (UART_TX_FIFO_SIZE - GET_BITFIELD(COM_UART->status,UART_TX_FIFO_SPACE) > 0))
			{
				;
			}
			//OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_485_DONE, 0, 0, 0);
			GPIO_Write(DIR_485_PIN, 0);
		}

#endif
		return 0;
	}
#ifdef __UART_485_MODE__
	COMCtrl.Mode485Tx = 1;
	COMCtrl.Mode485TxDone = 0;
	GPIO_Write(DIR_485_PIN, 1);
#endif
	COMCtrl.TxBusy = 1;
	if (PRINT_NORMAL == gSys.State[PRINT_STATE])
	{
		DBG("%u", TxLen);
		if (TxLen <= 64)
		{
			HexTrace(COMCtrl.DMABuf, TxLen);
		}
	}
	//SYS_Waketup();
	if (HAL_UNKNOWN_CHANNEL == OS_DMAStart(COM_UART_DMA_TX, COMCtrl.DMABuf, TxLen, HAL_IFC_SIZE_8_MODE_AUTO))
	{
		COMCtrl.TxBusy = 0;
		DBG("Tx fail!");
#ifdef __UART_485_MODE__
		COMCtrl.Mode485Tx = 0;
		COMCtrl.Mode485TxDone = 0;
		GPIO_Write(DIR_485_PIN, 0);
#endif
		OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_TX_REQ, 0, 0, 0);
		return 0;
	}
	COMCtrl.SleepWaitCnt = USP_MODE_TO * 3;
	return 1;
}

void COM_Task(void *pData)
{
	COS_EVENT Event;
	uint32_t TxLen = 0;
	uint32_t RxLen = 0;
	uint8_t Retry;
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
	DBG("Task start! %u", gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);
	COMCtrl.RxPos = 0;
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
    				OS_UartSetBR(COM_UART_ID, gSys.nParam[PARAM_TYPE_SYS].Data.ParamDW.Param[PARAM_COM_BR]);
    				COMCtrl.CurrentBR = Event.nParam1;
    				COMCtrl.NeedRxLen = 0;
    				COMCtrl.ProtocolType = COM_PROTOCOL_NONE;
    			}
    			gSys.State[PRINT_STATE] = PRINT_NORMAL;
    			break;
    		case COM_RX_TIMER_ID:
    			if (!COMCtrl.RxPos)
    			{
    				mask.rxDataAvailable = 1;
    				hal_UartIrqSetMask(COM_UART_ID, mask);
    				COM_Reset();
    				break;
    			}
    			DBG("%d", COMCtrl.RxPos);
    			//HexTrace(COMCtrl.RxBuf, COMCtrl.RxPos);
    			COM_RxFinish();
    			if (USP_CheckHead(COMCtrl.AnalyzeBuf))
    			{
    				COMCtrl.ProtocolType = COM_PROTOCOL_USP;
    			}
    			else if (LV_CheckHead(COMCtrl.AnalyzeBuf[0]))
    			{
    				COMCtrl.ProtocolType = COM_PROTOCOL_LV;
    			}
    #if (__CUST_CODE__ == __CUST_LY__)
    			else if (LY_CheckUartHead(COMCtrl.AnalyzeBuf[0]))

    #elif (__CUST_CODE__ == __CUST_KQ__)
    			else if (KQ_CheckUartHead(COMCtrl.AnalyzeBuf[0]))
    #elif (__CUST_CODE__ == __CUST_LB_V3__ || __CUST_CODE__ == __CUST_LB_V2__)
    			else if (LB_CheckUartHead(COMCtrl.AnalyzeBuf[0]))
    #else
    			else if (0)
    #endif
    			{
    				COMCtrl.ProtocolType = COM_PROTOCOL_DEV;
    			}
    			switch (COMCtrl.ProtocolType)
    			{
    			case COM_PROTOCOL_DEV:
    				OS_SendEvent(gSys.TaskID[USER_TASK_ID], EV_MMI_COM_TO_USER, 0, (uint32_t)&COMCtrl.AnalyzeBuf, COMCtrl.AnalyzeLen);
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
    				TxLen = USP_Analyze(COMCtrl.AnalyzeBuf, COMCtrl.AnalyzeLen, COMCtrl.TempBuf);
    				//HexTrace(COMCtrl.TempBuf, TxLen);
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
    		if (COMCtrl.RxDMAChannel != 0xff)
    		{
        		Retry = 0;
    			while (0 == (hwp_sysIfc->std_ch[COMCtrl.RxDMAChannel].status & SYS_IFC_FIFO_EMPTY))
    			{
    				//DBG("%d", COMCtrl.RxDMAChannel);
    				OS_Sleep(SYS_TICK/256);
    				Retry++;
    				if (Retry >= 2)
    				{
    					break;
    				}
    			}
    			RxLen = (DMA_BUF_LEN - hwp_sysIfc->std_ch[COMCtrl.RxDMAChannel].tc);
    			memcpy(&COMCtrl.RxBuf[COMCtrl.RxPos], COMCtrl.RxDMABuf, RxLen);
    			COMCtrl.RxPos += RxLen;
    			hwp_sysIfc->std_ch[COMCtrl.RxDMAChannel].control |= SYS_IFC_DISABLE;
    			COMCtrl.RxDMAChannel = 0xff;
    		}
			mask.rxDataAvailable = 1;
			hal_UartIrqSetMask(COM_UART_ID, mask);
			break;
		case EV_MMI_COM_TX_REQ:
			break;
		case EV_MMI_COM_NEW_BR:
			DBG("new br %u", Event.nParam1);
			OS_UartSetBR(COM_UART_ID, Event.nParam1);
			COMCtrl.CurrentBR = Event.nParam1;
			COMCtrl.NeedRxLen = 0;
			COMCtrl.ProtocolType = COM_PROTOCOL_NONE;
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

void COM_TxReq(uint8_t *Data, uint32_t Len)
{
	WriteRBufferForce(&COMCtrl.rTxBuf, Data, Len);
	OS_SendEvent(gSys.TaskID[COM_TASK_ID], EV_MMI_COM_TX_REQ, 0, 0, 0);
}

void COM_Tx(uint8_t *Data, uint32_t Len)
{
	WriteRBufferForce(&COMCtrl.rTxBuf, Data, Len);
}
