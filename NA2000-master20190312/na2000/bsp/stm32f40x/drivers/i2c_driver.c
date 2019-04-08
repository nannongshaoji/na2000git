/*
 * File      : i2c_driver.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2017, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-14     aubr.cool    1st version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "i2c_driver.h"

struct na_i2c_device
{
    struct rt_device         parent;
    struct rt_i2c_bus_device *bus;
};

/* RT-Thread device interface */

static rt_err_t na_i2c_init(rt_device_t dev)
{
    return RT_EOK;
}
static rt_err_t na_i2c_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t na_i2c_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_err_t na_i2c_control(rt_device_t dev, int cmd, void *args)
{
    return RT_EOK;
}

static rt_size_t na_i2c_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    struct na_i2c_device *i2c_device;
    const struct na_i2c_config *cfg;
    struct rt_i2c_msg msg[2];
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);

    i2c_device = (struct na_i2c_device *) dev;

    RT_ASSERT(i2c_device->parent.user_data != 0);
    cfg = (const struct na_i2c_config *) i2c_device->parent.user_data;

    if(pos > cfg->size)
    {
         return 0;
    }

    if(pos + size > cfg->size)
    {
         size = cfg->size - pos;
    }
		
		msg[0].addr     = cfg->addr;
    msg[0].flags    = cfg->flags | RT_I2C_RD;
    msg[0].buf      = (rt_uint8_t *) buffer;
    msg[0].len      = size;

    ret = rt_i2c_transfer(i2c_device->bus, msg, 1);
    return (ret == 2) ? size : 0;
}

static rt_size_t na_i2c_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    struct na_i2c_device *i2c_device;
    const struct na_i2c_config *cfg;
    struct rt_i2c_msg msg[2];
    rt_size_t ret = 0;
    RT_ASSERT(dev != 0);

    i2c_device = (struct na_i2c_device *) dev;

    RT_ASSERT(i2c_device->parent.user_data != 0);
    cfg = (const struct na_i2c_config *) i2c_device->parent.user_data;

    if(pos > cfg->size)
    {
         return 0;
    }

    if(pos + size > cfg->size)
    {
         size = cfg->size - pos;
    }
		
		msg[0].addr     = cfg->addr;
		msg[0].flags    = cfg->flags | RT_I2C_WR;
		msg[0].buf      = (rt_uint8_t *) buffer;
		msg[0].len      = size;

		ret = rt_i2c_transfer(i2c_device->bus, msg, 1);
		return (ret == 1) ? size : 0;
}

rt_err_t na_i2c_register(const char *fm_device_name, const char *i2c_bus, void *user_data)
{
    static struct na_i2c_device na_i2c_drv;
    struct rt_i2c_bus_device *bus;

    bus = rt_i2c_bus_device_find(i2c_bus);
    if (bus == RT_NULL)
    {
        return RT_ENOSYS;
    }

    na_i2c_drv.bus = bus;
    na_i2c_drv.parent.type      = RT_Device_Class_Char;
    na_i2c_drv.parent.init      = na_i2c_init;
    na_i2c_drv.parent.open      = na_i2c_open;
    na_i2c_drv.parent.close     = na_i2c_close;
    na_i2c_drv.parent.read      = na_i2c_read;
    na_i2c_drv.parent.write     = na_i2c_write;
    na_i2c_drv.parent.control   = na_i2c_control;
    na_i2c_drv.parent.user_data = user_data;

    return rt_device_register(&na_i2c_drv.parent, fm_device_name, RT_DEVICE_FLAG_RDWR);
}

extern int rt_hw_i2c_gpio_init(void);


struct na_i2c_config na_i2c_xxx=
{
	.size=0x1000,
	.addr=0x28,
	.flags=0x00,
};

static char has_set_addr = 0;

void set_submodule_addr_flag(char set_flag)
{
	has_set_addr = set_flag;
}

char get_submodule_addr_flag()
{
	return has_set_addr;
}

char i2c_write_addr(unsigned char can_addr)
{
	static char first_flag = 0;
	rt_device_t device = RT_NULL;
	unsigned char buff[2] = {0};
	
	if(first_flag == 0)
	{
		rt_hw_i2c_gpio_init();
		na_i2c_register("na_i2c","i2c1",&na_i2c_xxx);
		first_flag = 1;
	}
	
	device = rt_device_find("na_i2c");
	
	if( device == RT_NULL)
	{
			rt_kprintf("device %s: not found!\r\n");
			return 1;
	}
	
	device->open(device,RT_DEVICE_FLAG_RDWR);
	buff[0] = can_addr;
	buff[1] = ~can_addr;
	
	if(device->write(device,0,buff,2) != 2)
	{
		device->close(device);
		return 1;
	}
	device->close(device);
	return 0;
}
