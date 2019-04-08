/*
 * File      : bxcan.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author            Notes
 * 2015-05-14     aubrcool@qq.com   first version
 */
#include <rthw.h>
#include <rtdevice.h>
#include <board.h>
#include <bxcan.h>

#ifdef RT_USING_COMPONENTS_INIT
#include <components.h>
#endif
#ifdef RT_USING_CAN

#ifndef STM32F10X_CL
#define BX_CAN_FMRNUMBER 14
#define BX_CAN2_FMRSTART 7
#else
#define BX_CAN_FMRNUMBER 28
#define BX_CAN2_FMRSTART 14
#endif

struct stm_bxcan
{
    CAN_TypeDef *reg;
    void *mfrbase;
    IRQn_Type sndirq;
    IRQn_Type rcvirq0;
    IRQn_Type rcvirq1;
    IRQn_Type errirq;
    const rt_uint32_t fifo1filteroff;
};

static void bxcan1_filter_init(struct rt_can_device *can)
{
//		extern int get_module_addr();
//		int id = get_module_addr();
//		int id = 5;
//		int can_id[2],can_mask[2];
//	
//		can_id[0]  = ((id & 0x7F) << 21);
//		can_mask[0] = (0x7F<<21);

//		can_id[1]   = 0x7F << 21;
//		can_mask[1] = 0x7F << 21;
	
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	  CAN_FilterInitStructure.CAN_FilterNumber=0;	  //???0
  	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;//CAN_FilterMode_IdMask; 
  	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; //32? 
  	CAN_FilterInitStructure.CAN_FilterIdHigh=0x0000;////32?ID
  	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
  	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x0000;//32?MASK
  	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
   	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;//???0???FIFO0
  	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE; //?????0
  	CAN_FilterInit(&CAN_FilterInitStructure);//??????

	
//		CAN_FilterInitTypeDef  CAN_FilterInitStructure;
//		//两个滤波区间
//		CAN_FilterInitStructure.CAN_FilterNumber = 0;
//		CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
//		CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
//		CAN_FilterInitStructure.CAN_FilterIdHigh = (can_id[0])>>16;
//		CAN_FilterInitStructure.CAN_FilterIdLow = (can_id[0])&0x0000FFFF;//(9<<21)&0X0000FFFF;
//		CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (can_mask[0])>>16;
//		CAN_FilterInitStructure.CAN_FilterMaskIdLow = CAN_ID_EXT; 
//		CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
//		CAN_FilterInit(&CAN_FilterInitStructure);
//	
//		CAN_FilterInitStructure.CAN_FilterNumber = 0;
//		CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
//		CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
//		CAN_FilterInitStructure.CAN_FilterIdHigh = (can_id[1])>>16;
//		CAN_FilterInitStructure.CAN_FilterIdLow = (can_id[1])&0x0000FFFF;//(9<<21)&0X0000FFFF;
//		CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (can_mask[1])>>16;
//		CAN_FilterInitStructure.CAN_FilterMaskIdLow = CAN_ID_EXT; 
//		CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
//		CAN_FilterInit(&CAN_FilterInitStructure);
	
}

static void bxcan_init(CAN_TypeDef *pcan, rt_uint32_t baud, rt_uint32_t mode)
{
    CAN_InitTypeDef        CAN_InitStructure;

    CAN_InitStructure.CAN_TTCM = DISABLE;
    CAN_InitStructure.CAN_ABOM = ENABLE;
    CAN_InitStructure.CAN_AWUM = DISABLE;
    CAN_InitStructure.CAN_NART = DISABLE;
    CAN_InitStructure.CAN_RFLM = DISABLE;
    CAN_InitStructure.CAN_TXFP = ENABLE;
    switch (mode)
    {
    case RT_CAN_MODE_NORMAL:
        CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
        break;
//    case RT_CAN_MODE_LISEN:
//        CAN_InitStructure.CAN_Mode = CAN_Mode_Silent;
//        break;
//    case RT_CAN_MODE_LOOPBACK:
//        CAN_InitStructure.CAN_Mode = CAN_Mode_LoopBack;
//        break;
//    case RT_CAN_MODE_LOOPBACKANLISEN:
//        CAN_InitStructure.CAN_Mode = CAN_Mode_Silent_LoopBack;
//        break;
    }
//    CAN_InitStructure.CAN_SJW = BAUD_DATA(SJW, baud);
//    CAN_InitStructure.CAN_BS1 = BAUD_DATA(BS1, baud);
//    CAN_InitStructure.CAN_BS2 = BAUD_DATA(BS2, baud);
//    CAN_InitStructure.CAN_Prescaler = BAUD_DATA(RRESCL, baud);
		
		CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
		CAN_InitStructure.CAN_BS1 = CAN_BS1_12tq;
		CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
		CAN_InitStructure.CAN_Prescaler = 2;

    CAN_Init(pcan, &CAN_InitStructure);
}

static void bxcan1_hw_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1);
}

static rt_err_t configure(struct rt_can_device *can, struct can_configure *cfg)
{
    CAN_TypeDef *pbxcan;

    pbxcan = ((struct stm_bxcan *) can->parent.user_data)->reg;
    assert_param(IS_CAN_ALL_PERIPH(pbxcan));
    if (pbxcan == CAN1)
    {
        bxcan1_hw_init();
        bxcan_init(pbxcan, cfg->baud_rate, can->config.mode);
        bxcan1_filter_init(can);
    }
    return RT_EOK;
}
static rt_err_t control(struct rt_can_device *can, int cmd, void *arg)
{
    struct stm_bxcan *pbxcan;
    rt_uint32_t argval;
    NVIC_InitTypeDef  NVIC_InitStructure;

    pbxcan = (struct stm_bxcan *) can->parent.user_data;
    assert_param(pbxcan != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        argval = (rt_uint32_t) arg;
        if (argval == RT_DEVICE_FLAG_INT_RX)
        {
            NVIC_DisableIRQ(pbxcan->rcvirq0);
            NVIC_DisableIRQ(pbxcan->rcvirq1);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FMP0 , DISABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FF0 , DISABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FOV0 , DISABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FMP1 , DISABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FF1 , DISABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FOV1 , DISABLE);
        }
        else if (argval == RT_DEVICE_FLAG_INT_TX)
        {
            NVIC_DisableIRQ(pbxcan->sndirq);
            CAN_ITConfig(pbxcan->reg, CAN_IT_TME, DISABLE);
        }
        else if (argval == RT_DEVICE_CAN_INT_ERR)
        {
            CAN_ITConfig(pbxcan->reg, CAN_IT_BOF , DISABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_LEC , DISABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_ERR , DISABLE);
            NVIC_DisableIRQ(pbxcan->errirq);
        }
        break;
    case RT_DEVICE_CTRL_SET_INT:
        argval = (rt_uint32_t) arg;
        if (argval == RT_DEVICE_FLAG_INT_RX)
        {
            CAN_ITConfig(pbxcan->reg, CAN_IT_FMP0 , ENABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FF0 , ENABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FOV0 , ENABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FMP1 , ENABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FF1 , ENABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_FOV1 , ENABLE);
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_InitStructure.NVIC_IRQChannel = pbxcan->rcvirq0;
            NVIC_Init(&NVIC_InitStructure);
            NVIC_InitStructure.NVIC_IRQChannel = pbxcan->rcvirq1;
            NVIC_Init(&NVIC_InitStructure);
        }
        else if (argval == RT_DEVICE_FLAG_INT_TX)
        {
            CAN_ITConfig(pbxcan->reg, CAN_IT_TME, ENABLE);
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_InitStructure.NVIC_IRQChannel = pbxcan->sndirq;
            NVIC_Init(&NVIC_InitStructure);
        }
        else if (argval == RT_DEVICE_CAN_INT_ERR)
        {
            CAN_ITConfig(pbxcan->reg, CAN_IT_BOF , ENABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_LEC , ENABLE);
            CAN_ITConfig(pbxcan->reg, CAN_IT_ERR , ENABLE);
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_InitStructure.NVIC_IRQChannel = pbxcan->errirq;
            NVIC_Init(&NVIC_InitStructure);
        }
        break;
    case RT_CAN_CMD_SET_FILTER:
        break;
    case RT_CAN_CMD_SET_MODE:
    case RT_CAN_CMD_SET_BAUD:
        break;
    case RT_CAN_CMD_SET_PRIV:
        break;
    case RT_CAN_CMD_GET_STATUS:
    {
        rt_uint32_t errtype;
        errtype = pbxcan->reg->ESR;
        can->status.rcverrcnt = errtype >> 24;
        can->status.snderrcnt = (errtype >> 16 & 0xFF);
        can->status.errcode = errtype & 0x07;
        if (arg != &can->status)
        {
            rt_memcpy(arg, &can->status, sizeof(can->status));
        }
    }
    break;
    }

    return RT_EOK;
}
static int sendmsg(struct rt_can_device *can, const void *buf, rt_uint32_t boxno)
{
	#define TMIDxR_TXRQ 1
	
    CAN_TypeDef *pbxcan;
    struct rt_can_msg *pmsg = (struct rt_can_msg *) buf;

    pbxcan = ((struct stm_bxcan *) can->parent.user_data)->reg;
    assert_param(IS_CAN_ALL_PERIPH(pbxcan));

    pbxcan->sTxMailBox[boxno].TIR &= TMIDxR_TXRQ;
    if (pmsg->ide == RT_CAN_STDID)
    {
        assert_param(IS_CAN_STDID(pmsg->id));
        pbxcan->sTxMailBox[boxno].TIR |= ((pmsg->id << 21) | \
                                          pmsg->rtr);
    }
    else
    {
        assert_param(IS_CAN_EXTID(pmsg->id));
        pbxcan->sTxMailBox[boxno].TIR |= ((pmsg->id << 3) | \
                                          pmsg->ide << 2 | \
                                          pmsg->rtr);
    }

    pmsg->len &= (uint8_t)0x0000000F;
    pbxcan->sTxMailBox[boxno].TDTR &= (uint32_t)0xFFFFFFF0;
    pbxcan->sTxMailBox[boxno].TDTR |= pmsg->len;

    pbxcan->sTxMailBox[boxno].TDLR = (((uint32_t)pmsg->data[3] << 24) |
                                      ((uint32_t)pmsg->data[2] << 16) |
                                      ((uint32_t)pmsg->data[1] << 8) |
                                      ((uint32_t)pmsg->data[0]));
    if (pmsg->len > 4)
    {
        pbxcan->sTxMailBox[boxno].TDHR = (((uint32_t)pmsg->data[7] << 24) |
                                          ((uint32_t)pmsg->data[6] << 16) |
                                          ((uint32_t)pmsg->data[5] << 8) |
                                          ((uint32_t)pmsg->data[4]));
    }
    pbxcan->sTxMailBox[boxno].TIR |= TMIDxR_TXRQ;

    return RT_EOK;
}
static int recvmsg(struct rt_can_device *can, void *buf, rt_uint32_t boxno)
{
    CAN_TypeDef *pbxcan;
    struct rt_can_msg *pmsg = (struct rt_can_msg *) buf;

    pbxcan = ((struct stm_bxcan *) can->parent.user_data)->reg;
    assert_param(IS_CAN_ALL_PERIPH(pbxcan));
    assert_param(IS_CAN_FIFO(boxno));
    pmsg->ide = ((uint8_t)0x04 & pbxcan->sFIFOMailBox[boxno].RIR) >> 2;
    if (pmsg->ide == CAN_Id_Standard)
    {
        pmsg->id = (uint32_t)0x000007FF & (pbxcan->sFIFOMailBox[boxno].RIR >> 21);
    }
    else
    {
        pmsg->id = (uint32_t)0x1FFFFFFF & (pbxcan->sFIFOMailBox[boxno].RIR >> 3);
    }

    pmsg->rtr = (uint8_t)((0x02 & pbxcan->sFIFOMailBox[boxno].RIR) >> 1);
    pmsg->len = (uint8_t)0x0F & pbxcan->sFIFOMailBox[boxno].RDTR;
    pmsg->data[0] = (uint8_t)0xFF & pbxcan->sFIFOMailBox[boxno].RDLR;
    pmsg->data[1] = (uint8_t)0xFF & (pbxcan->sFIFOMailBox[boxno].RDLR >> 8);
    pmsg->data[2] = (uint8_t)0xFF & (pbxcan->sFIFOMailBox[boxno].RDLR >> 16);
    pmsg->data[3] = (uint8_t)0xFF & (pbxcan->sFIFOMailBox[boxno].RDLR >> 24);
    if (pmsg->len > 4)
    {
        pmsg->data[4] = (uint8_t)0xFF & pbxcan->sFIFOMailBox[boxno].RDHR;
        pmsg->data[5] = (uint8_t)0xFF & (pbxcan->sFIFOMailBox[boxno].RDHR >> 8);
        pmsg->data[6] = (uint8_t)0xFF & (pbxcan->sFIFOMailBox[boxno].RDHR >> 16);
        pmsg->data[7] = (uint8_t)0xFF & (pbxcan->sFIFOMailBox[boxno].RDHR >> 24);
    }
    pmsg->hdr = (uint8_t)0xFF & (pbxcan->sFIFOMailBox[boxno].RDTR >> 8);
    if (boxno) pmsg->hdr += ((struct stm_bxcan *) can->parent.user_data)->fifo1filteroff * 4;
    return RT_EOK;
}

static const struct rt_can_ops canops =
{
    configure,
    control,
    sendmsg,
    recvmsg,
};
#ifdef USING_BXCAN1
static struct stm_bxcan bxcan1data =
{
		.reg = CAN1,
    .mfrbase = (void *) &CAN1->sFilterRegister[0],
    .sndirq = CAN1_TX_IRQn,
    .rcvirq0 = CAN1_RX0_IRQn,
    .rcvirq1 = CAN1_RX1_IRQn,
    .errirq =  CAN1_SCE_IRQn,
};
struct rt_can_device bxcan1;
void CAN1_RX0_IRQHandler(void)
{
    if (CAN1->RF0R & 0x03)
    {
				CAN_ClearITPendingBit( CAN1, CAN_IT_FF0 );
				while( CAN_MessagePending( CAN1, CAN_FIFO0 ) !=0 )
				{
					if ((CAN1->RF0R & CAN_RF0R_FOVR0) != 0)
					{
							CAN1->RF0R = CAN_RF0R_FOVR0;
							rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_RXOF_IND | 0 << 8);
					}
					else
					{
							rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_RX_IND | 0 << 8);
					}
					CAN1->RF0R |= CAN_RF0R_RFOM0;
				}
    }
}
void CAN1_RX1_IRQHandler(void)
{
    if (CAN1->RF1R & 0x03)
    {
				CAN_ClearITPendingBit( CAN1, CAN_IT_FF1 );
				while( CAN_MessagePending( CAN1, CAN_FIFO1 ) !=0 )
				{
					if ((CAN1->RF1R & CAN_RF1R_FOVR1) != 0)
					{
							CAN1->RF1R = CAN_RF1R_FOVR1;
							rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_RXOF_IND | 1 << 8);
					}
					else
					{
							rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_RX_IND | 1 << 8);
					}
					CAN1->RF1R |= CAN_RF1R_RFOM1;
				}
    }
}
void CAN1_TX_IRQHandler(void)
{
    rt_uint32_t state;
    if (CAN1->TSR & (CAN_TSR_RQCP0))
    {
        state =  CAN1->TSR & (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0);
        CAN1->TSR |= CAN_TSR_RQCP0;
        if (state == (CAN_TSR_RQCP0 | CAN_TSR_TXOK0 | CAN_TSR_TME0))
        {
            rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_TX_DONE | 0 << 8);
        }
        else
        {
            rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_TX_FAIL | 0 << 8);
        }
    }
    if (CAN1->TSR & (CAN_TSR_RQCP1))
    {
        state =  CAN1->TSR & (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1);
        CAN1->TSR |= CAN_TSR_RQCP1;
        if (state == (CAN_TSR_RQCP1 | CAN_TSR_TXOK1 | CAN_TSR_TME1))
        {
            rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_TX_DONE | 1 << 8);
        }
        else
        {
            rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_TX_FAIL | 1 << 8);
        }
    }
    if (CAN1->TSR & (CAN_TSR_RQCP2))
    {
        state =  CAN1->TSR & (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2);
        CAN1->TSR |= CAN_TSR_RQCP2;
        if (state == (CAN_TSR_RQCP2 | CAN_TSR_TXOK2 | CAN_TSR_TME2))
        {
            rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_TX_DONE | 2 << 8);
        }
        else
        {
            rt_hw_can_isr(&bxcan1, RT_CAN_EVENT_TX_FAIL | 2 << 8);
        }
    }
}
void CAN1_SCE_IRQHandler(void)
{
    rt_uint32_t errtype;
    errtype = CAN1->ESR;
    if (errtype & 0x70 && bxcan1.status.lasterrtype == (errtype & 0x70))
    {
        switch ((errtype & 0x70) >> 4)
        {
        case RT_CAN_BUS_BIT_PAD_ERR:
            bxcan1.status.bitpaderrcnt++;
            break;
        case RT_CAN_BUS_FORMAT_ERR:
            bxcan1.status.formaterrcnt++;
            break;
        case RT_CAN_BUS_ACK_ERR:
            bxcan1.status.ackerrcnt++;
            break;
        case RT_CAN_BUS_IMPLICIT_BIT_ERR:
        case RT_CAN_BUS_EXPLICIT_BIT_ERR:
            bxcan1.status.biterrcnt++;
            break;
        case RT_CAN_BUS_CRC_ERR:
            bxcan1.status.crcerrcnt++;
            break;
        }
        bxcan1.status.lasterrtype = errtype & 0x70;
        CAN1->ESR &= ~0x70;
    }
    bxcan1.status.rcverrcnt = errtype >> 24;
    bxcan1.status.snderrcnt = (errtype >> 16 & 0xFF);
    bxcan1.status.errcode = errtype & 0x07;
    CAN1->MSR |= CAN_MSR_ERRI;
}
#endif /*USING_BXCAN1*/

int stm32_bxcan_init(void)
{

#ifdef USING_BXCAN1
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1 , ENABLE);
    CAN_DeInit(CAN1);
    bxcan1.config.baud_rate = CAN1MBaud;
    bxcan1.config.msgboxsz = 16;
    bxcan1.config.sndboxnumber = 3;
    bxcan1.config.mode = RT_CAN_MODE_NORMAL;
    bxcan1.config.privmode = 0;
    bxcan1.config.ticks = 50;
#ifdef RT_CAN_USING_HDR
    bxcan1.config.maxhdr = BX_CAN2_FMRSTART * 4;
#endif
    rt_hw_can_register(&bxcan1, "bxcan1", &canops, &bxcan1data);
#endif

    return RT_EOK;
}

#endif /*RT_USING_CAN2*/
