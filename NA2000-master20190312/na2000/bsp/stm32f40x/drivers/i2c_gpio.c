
#include <rtthread.h>
#include <rtdevice.h>
#include <stm32f4xx.h>


//I2C1 SDA:PF0 SCL: PF1
#define I2C1_GPIO_SDA         GPIO_Pin_0
#define I2C1_GPIO_SCL         GPIO_Pin_1
#define I2C1_GPIO        	    GPIOF

#define I2C1_SDA_SOURCE				GPIO_PinSource0
#define I2C1_SCL_SOURCE				GPIO_PinSource1
#define I2C1_PIN_CLK_PORT			RCC_AHB1Periph_GPIOF

void stm32_set_sda(void *data, rt_int32_t state)
{
    if(state == 1)
        GPIO_SetBits(I2C1_GPIO , I2C1_GPIO_SDA);   //GPIOB->BSRRL = I2C1_GPIO_SDA
    else if(state == 0)
        GPIO_ResetBits(I2C1_GPIO , I2C1_GPIO_SDA); //GPIOB->BSRRH = I2C1_GPIO_SDA
}

void stm32_set_scl(void *data, rt_int32_t state)
{
    if(state == 1)
        GPIO_SetBits(I2C1_GPIO , I2C1_GPIO_SCL);   //GPIOB->BSRRL = I2C1_GPIO_SCL
    else if(state == 0)
        GPIO_ResetBits(I2C1_GPIO , I2C1_GPIO_SCL); //GPIOB->BSRRH = I2C1_GPIO_SCL
}

rt_int32_t stm32_get_sda(void *data)
{
    return (rt_int32_t)GPIO_ReadInputDataBit(I2C1_GPIO , I2C1_GPIO_SDA);//return(GPIOB->IDR  & I2C1_GPIO_SDA)
}

rt_int32_t stm32_get_scl(void *data)
{
    return (rt_int32_t)GPIO_ReadInputDataBit(I2C1_GPIO , I2C1_GPIO_SCL);//return(GPIOB->IDR  & I2C1_GPIO_SCL)
}

void stm32_udelay(rt_uint32_t us)
{
	rt_uint32_t delta;
	rt_uint32_t current_delay;
	us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
	delta = SysTick->VAL;
	do
	{
		if ( delta > SysTick->VAL )
			current_delay = delta - SysTick->VAL;
		else
			current_delay = SysTick->LOAD + delta - SysTick->VAL;
	} while( current_delay < us );
}

void stm32_mdelay(rt_uint32_t ms)
{
    stm32_udelay(ms * 1000);
}

static const struct  rt_i2c_bit_ops stm32_i2c_bit_ops =
{
    (void*)0xaa,     //no use in set_sda,set_scl,get_sda,get_scl
    stm32_set_sda,
    stm32_set_scl,
    stm32_get_sda,
    stm32_get_scl,
    stm32_udelay,
    20, 
};

static void RCC_Configuration(void)
{
	RCC_APB1PeriphClockCmd( I2C1_PIN_CLK_PORT, ENABLE );    
}

static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	GPIO_InitStructure.GPIO_Pin = I2C1_GPIO_SDA | I2C1_GPIO_SCL;

	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_OUT ;
	GPIO_InitStructure.GPIO_Speed =GPIO_Speed_50MHz;


	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;    //¿ªÂ©Êä³ö
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(I2C1_GPIO, &GPIO_InitStructure);


	GPIO_SetBits(I2C1_GPIO , I2C1_GPIO_SDA);
	GPIO_SetBits(I2C1_GPIO , I2C1_GPIO_SCL);
}

int rt_hw_i2c_gpio_init(void)
{
	static struct rt_i2c_bus_device stm32_i2c;
	
	RCC_Configuration();
	GPIO_Configuration();
	
	rt_memset((void *)&stm32_i2c, 0, sizeof(struct rt_i2c_bus_device));
	stm32_i2c.priv = (void *)&stm32_i2c_bit_ops;
	rt_i2c_bit_add_bus(&stm32_i2c, "i2c1");
			
	return 0;
}
INIT_BOARD_EXPORT(rt_hw_i2c_gpio_init);
//rt_hw_i2c_init will be called in rt_components_board_init()

