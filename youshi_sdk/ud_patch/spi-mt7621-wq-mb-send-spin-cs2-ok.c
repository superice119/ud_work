/*
 * spi-mt7621.c -- MediaTek MT7621 SPI controller driver
 *
 * Copyright (C) 2011 Sergiy <piratfm@gmail.com>
 * Copyright (C) 2011-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 *
 * Some parts are based on spi-orion.c:
 *   Author: Shadi Ammouri <shadi@marvell.com>
 *   Copyright (C) 2007-2008 Marvell Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/reset.h>
#include <linux/spi/spi.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/swab.h>

//add by zzk 20170828 
#include <linux/gpio.h>
#include <linux/delay.h>	/*time*/

#include <ralink_regs.h>

#define SPI_BPW_MASK(bits) BIT((bits) - 1)

#define DRIVER_NAME			"spi-mt7621"
/* in usec */
#define RALINK_SPI_WAIT_MAX_LOOP	2000

/* SPISTAT register bit field */
#define SPISTAT_BUSY			BIT(0)

#define MT7621_SPI_TRANS	0x00
#define SPITRANS_BUSY		BIT(16)

#define MT7621_SPI_OPCODE	0x04
#define MT7621_SPI_DATA0	0x08
#define MT7621_SPI_DATA4	0x18
#define SPI_CTL_TX_RX_CNT_MASK	0xff
#define SPI_CTL_START		BIT(8)

#define MT7621_SPI_POLAR	0x38
#define MT7621_SPI_MASTER	0x28
#define MT7621_SPI_MOREBUF	0x2c
#define MT7621_SPI_SPACE	0x3c

#define MT7621_SPI_STATUS   0x34

#define MT7621_CPHA		BIT(5)
#define MT7621_CPOL		BIT(4)
#define MT7621_LSB_FIRST	BIT(3)

#define RT2880_SPI_MODE_BITS	(SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST | SPI_CS_HIGH)

//add by zzk 20170828 
int  spi_cs[]={-1, 6, 0, 1, 2, 3, 4, 5, 11, 20, 21}; //4, i2c_clk; 5, i2c_sda

#define MT7621_MB_STRANS	/* allow word transfer */

//#undef  MT7621_MB_STRANS

struct mt7621_spi;

struct mt7621_spi {
	struct spi_master	*master;
	void __iomem		*base;
	unsigned int		sys_freq;
	unsigned int		speed;
	struct clk		*clk;
	spinlock_t		lock;

	struct mt7621_spi_ops	*ops;
};

static inline struct mt7621_spi *spidev_to_mt7621_spi(struct spi_device *spi)
{
	return spi_master_get_devdata(spi->master);
}

static inline u32 mt7621_spi_read(struct mt7621_spi *rs, u32 reg)
{
	return ioread32(rs->base + reg);
}

static inline void mt7621_spi_write(struct mt7621_spi *rs, u32 reg, u32 val)
{
	iowrite32(val, rs->base + reg);
}

static void mt7621_spi_reset(struct mt7621_spi *rs)
{
	u32 master = mt7621_spi_read(rs, MT7621_SPI_MASTER);

	master |= 7 << 29;
	master |= 1 << 2;
	master &= ~(1 << 10);

	mt7621_spi_write(rs, MT7621_SPI_MASTER, master);
}

static void mt7621_spi_set_cs(struct spi_device *spi, int enable)
{
	struct mt7621_spi *rs = spidev_to_mt7621_spi(spi);
	int cs = spi->chip_select;
	u32 polar = 0;

	mt7621_spi_reset(rs);

	/*****************************************
	  modfiy  zkz 
	 ******************************************/
#if 1
	if(cs >= 2) {

		//int ret = gpio_request_one(spi_cs[cs], (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW), NULL);

		gpio_set_value_cansleep(spi_cs[cs], (spi->mode&SPI_CS_HIGH)?(enable):(!enable));
		//gpio_set_value_cansleep(spi_cs[cs], enable);
		dev_info(spi->master->dev.parent,"in %s --- cs%d , active = %2X \n", __func__, cs, enable);

		//gpio_free(spi_cs[cs]);
	}
	else
	{
#endif
		//if (enable)
		//polar = BIT(cs);
		//mt7621_spi_write(rs, MT7621_SPI_POLAR, polar);
		unsigned long flags;

		spin_lock_irqsave(&rs->lock, flags);
		u32 reg = mt7621_spi_read(rs, MT7621_SPI_POLAR);

		if (enable)
			reg |= (BIT(cs));
		else
			reg &= ~(BIT(cs));

		mt7621_spi_write(rs, MT7621_SPI_POLAR, reg);
		spin_unlock_irqrestore(&rs->lock, flags);
#if 1
		if(cs == 1) {
			u32 reg1 = mt7621_spi_read(rs, MT7621_SPI_STATUS);
			udelay(10);
#if 0
			u32 reg2 = mt7621_spi_read(rs, MT7621_SPI_POLAR);
			u32 reg3 = mt7621_spi_read(rs, MT7621_SPI_SPACE);

			dev_dbg(spi->master->dev.parent,"cs func in %s , SPI_MASTER = %2X,  status = %x,  polar = %x, space = %x\n", __func__, reg, reg1, reg2, reg3);
#endif

		}
#endif
#if 1
	}
#endif


}


static int mt7621_spi_prepare(struct spi_device *spi, unsigned int speed)
{
	struct mt7621_spi *rs = spidev_to_mt7621_spi(spi);

	int cs = spi->chip_select;

	u32 rate;
	u32 reg;

	dev_dbg(&spi->dev, "speed:%u\n", speed);

	rate = DIV_ROUND_UP(rs->sys_freq, speed);
	dev_dbg(&spi->dev, "rate-1:%u\n", rate);

	if (rate > 4097)
		return -EINVAL;

	if (rate < 2)
		rate = 2;

	reg = mt7621_spi_read(rs, MT7621_SPI_MASTER);

	if (cs == 1) 
	reg &= ~(0xcfff << 16);
	else	
	reg &= ~(0xfff << 16);

	reg |= (rate - 2) << 16;
	rs->speed = speed;

	reg &= ~MT7621_LSB_FIRST;
	if (spi->mode & SPI_LSB_FIRST)
		reg |= MT7621_LSB_FIRST;

	reg &= ~(MT7621_CPHA | MT7621_CPOL);
	switch(spi->mode & (SPI_CPOL | SPI_CPHA)) {
		case SPI_MODE_0:
			break;
		case SPI_MODE_1:
			reg |= MT7621_CPHA;
			break;
		case SPI_MODE_2:
			reg |= MT7621_CPOL;
			break;
		case SPI_MODE_3:
			reg |= MT7621_CPOL | MT7621_CPHA;
			break;
	}
	mt7621_spi_write(rs, MT7621_SPI_MASTER, reg);

#if 1

	if (cs == 1) {
		u32 reg1 = mt7621_spi_read(rs, MT7621_SPI_STATUS);
		udelay(10);
#if 0
		u32 reg2 = mt7621_spi_read(rs, MT7621_SPI_POLAR);
		u32 reg3 = mt7621_spi_read(rs, MT7621_SPI_SPACE);

		dev_dbg(spi->master->dev.parent,"in %s , SPI_MASTER = %2X,  status = %x,  polar = %x, space = %x\n", __func__, reg, reg1, reg2, reg3);
#endif
	}
#endif
	return 0;
}

static inline int mt7621_spi_wait_till_ready(struct spi_device *spi)
{
#if 1
	struct mt7621_spi *rs = spidev_to_mt7621_spi(spi);
	int i;

	for (i = 0; i < RALINK_SPI_WAIT_MAX_LOOP; i++) {
		u32 status;

		status = mt7621_spi_read(rs, MT7621_SPI_TRANS);
		if ((status & SPITRANS_BUSY) == 0) {
			return 0;
		}
		cpu_relax();
		udelay(1);
	}

	return -ETIMEDOUT;
#else
	udelay(1);
	return 0;
#endif
}

static int mt7621_spi_transfer_half_duplex(struct spi_master *master,
		struct spi_message *m)
{
	struct mt7621_spi *rs = spi_master_get_devdata(master);
	struct spi_device *spi = m->spi;
	unsigned int speed = spi->max_speed_hz;
	struct spi_transfer *t = NULL;
	int status = 0;
	int i, len = 0;
	int rx_len = 0;
	u32 data[9] = { 0 };
	u32 val;
	int cs_active = 0;

	mt7621_spi_wait_till_ready(spi);

	list_for_each_entry(t, &m->transfers, transfer_list) {
		const u8 *buf = t->tx_buf;
		int rlen = t->len;

		if (t->rx_buf)
			rx_len += rlen;

		if (!buf)
			continue;

		if (WARN_ON(len + rlen > 36)) {
			status = -EIO;
			goto msg_done;
		}

		for (i = 0; i < rlen; i++, len++)
			data[len / 4] |= buf[i] << (8 * (len & 3));
	}

	if (WARN_ON(rx_len > 32)) {
		status = -EIO;
		goto msg_done;
	}

	if (mt7621_spi_prepare(spi, speed)) {
		status = -EIO;
		goto msg_done;
	}

	data[0] = swab32(data[0]);
	if (len < 4)
		data[0] >>= (4 - len) * 8;

	for (i = 0; i < len; i += 4) {
		mt7621_spi_write(rs, MT7621_SPI_OPCODE + i, data[i / 4]);
	}

	val = (min_t(int, len, 4) * 8) << 24;
	if (len > 4)
		val |= (len - 4) * 8;
	val |= (rx_len * 8) << 12;
	mt7621_spi_write(rs, MT7621_SPI_MOREBUF, val);

	//mt7621_spi_set_cs(spi, 1);
	if (!cs_active) {
		mt7621_spi_set_cs(spi, 1);
		cs_active = 1;
	}

	val = mt7621_spi_read(rs, MT7621_SPI_TRANS);
	val |= SPI_CTL_START;
	mt7621_spi_write(rs, MT7621_SPI_TRANS, val);


	mt7621_spi_wait_till_ready(spi);

	//mt7621_spi_set_cs(spi, 0);
	//if (t->cs_change) {
	if (cs_active) {
		mt7621_spi_set_cs(spi, 0);
		cs_active = 0;
	}			

	for (i = 0; i < rx_len; i += 4)
		data[i / 4] = mt7621_spi_read(rs, MT7621_SPI_DATA0 + i);

	m->actual_length = len + rx_len;

	len = 0;
	list_for_each_entry(t, &m->transfers, transfer_list) {
		u8 *buf = t->rx_buf;

		if (!buf)
			continue;

		for (i = 0; i < t->len; i++, len++)
			buf[i] = data[len / 4] >> (8 * (len & 3));
	}

msg_done:
	if (cs_active)
		mt7621_spi_set_cs(spi, 0);

	m->status = status;
	spi_finalize_current_message(master);

	return 0;
}

// add start
#ifdef MT7621_MB_STRANS
/*
 * more-byte-mode for reading and writing data
 */
static int mt7621_spi_mb_transfer_half_duplex
(
 struct spi_master *master,
 struct spi_message *m
 )
{
	struct mt7621_spi *rs = spi_master_get_devdata(master);
	struct spi_device *spi = m->spi;
	unsigned int speed = spi->max_speed_hz;
	struct spi_transfer *t = NULL;
	int status = 0;
	int i = 0, len = 0;
	u8 is_write = 0;
	u32 data[9] = { 0 };
	u32 val = 0;
	u32 transfer_len = 0;
	int cs_active = 0;
		unsigned long flags;


	mt7621_spi_wait_till_ready(spi);

	spin_lock_irqsave(&rs->lock, flags);

	udelay(100);

	//dev_dbg(&spi->dev, "seven spidev test ->cs: %d\n", spi->chip_select);

	list_for_each_entry(t, &m->transfers, transfer_list) {
		const u8 *txbuf = t->tx_buf;
		u8 *rxbuf = t->rx_buf;

		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			dev_err(&spi->dev,
					"message rejected: invalid transfer data buffers\n");
			status = -EIO;
			goto msg_done;
		}

		if (rxbuf)
			is_write = 0;
		else if(txbuf)
			is_write = 1;

		if (mt7621_spi_prepare(spi, speed)) {
			status = -EIO;
			goto msg_done;
		}

		transfer_len = t->len/4; 
		//mt7621_spi_set_cs(spi, 1);
		if (!cs_active) {
			mt7621_spi_set_cs(spi, 1);
			cs_active = 1;
		}
		//printk("seven %s %d tlen:%d, transfer_len:%d, write:%d\n", __func__, __LINE__, t->len, transfer_len, is_write);

		if(transfer_len){	/* for word transfer */
			u32 u32TxNum = 0;

			while ( transfer_len > 0 )
			{
				u32TxNum = transfer_len%8;
				if ( !u32TxNum )
					u32TxNum = 8;

				for ( i=0; i<u32TxNum*4; i++)
				{
					if ( is_write ){	/* for write transfer */
						data[i / 4] |= *txbuf++ << (8 * (i & 3));
					}
					//else	/* for read transfer */

				}
#if 0
				for(i=0; i<u32TxNum*4; i += 4)
					printk("0x%x, ", data[i/4]);

				printk("\n");
#endif
				data[0] = swab32(data[0]);
				val = 0;
				if(is_write){
					for(i=0; i<u32TxNum*4; i += 4)
						mt7621_spi_write(rs, MT7621_SPI_OPCODE + i, data[i / 4]);

					val = (min_t(int, u32TxNum*4, 4) * 8) << 24;	/* must be set 32 */
					val |= ((u32TxNum*4) - 4) * 8;					/* mosi_cnt */
				}else
					val |= ((u32TxNum*4) * 8) << 12;				/* miso_cnt */

				mt7621_spi_write(rs, MT7621_SPI_MOREBUF, val);
				//mt7621_spi_set_cs(spi, 1);
				val = mt7621_spi_read(rs, MT7621_SPI_TRANS);
				val |= SPI_CTL_START;
				mt7621_spi_write(rs, MT7621_SPI_TRANS, val);

				mt7621_spi_wait_till_ready(spi);

				if(!is_write){

					for (i = 0; i < u32TxNum*4; i += 4)
						data[i / 4] = mt7621_spi_read(rs, MT7621_SPI_DATA0 + i);

					for (i = 0; i < u32TxNum*4; i++)
						*rxbuf++ = data[i / 4] >> (8 * (i & 3));
				}

				len += u32TxNum*4;
				transfer_len -= u32TxNum;
				memset(data, 0, sizeof(data));

			}

		}

		transfer_len = t->len%4;
		if(transfer_len){	/* for bytes transfer 0-3bytes*/

			for ( i=0; i<transfer_len; i++ ){
				if(is_write)
					data[i / 4] |= *txbuf++ << (8 * (i & 3));
			}

			data[0] = swab32(data[0]);
			data[0] >>= (4 - transfer_len) * 8;

			val = 0;
			if(is_write){
				for(i=0; i<transfer_len; i += 4)
					mt7621_spi_write(rs, MT7621_SPI_OPCODE + i, data[i / 4]);

				val = (min_t(int, transfer_len, 4) * 8) << 24;	/* must be 32 */
				//val |= (transfer_len - 4) * 8;		/* mosi_cnt */
			}else{
				val |= (transfer_len* 8) << 12;			/* miso_cnt */
			}
			mt7621_spi_write(rs, MT7621_SPI_MOREBUF, val);
			val = mt7621_spi_read(rs, MT7621_SPI_TRANS);
			val |= SPI_CTL_START;
			mt7621_spi_write(rs, MT7621_SPI_TRANS, val);

			mt7621_spi_wait_till_ready(spi);

			if(!is_write){

				for (i = 0; i < transfer_len; i += 4)
					data[i / 4] = mt7621_spi_read(rs, MT7621_SPI_DATA0 + i);

				for (i = 0; i < transfer_len; i++)
					*rxbuf++ = data[i / 4] >> (8 * (i & 3));
			}
			len += transfer_len;
			memset(data, 0, sizeof(data));		
		}

		m->actual_length = len; 	//+ rx_len;
		//mt7621_spi_set_cs(spi, 0);
		if (t->cs_change) {
			mt7621_spi_set_cs(spi, 0);
			cs_active = 0;
		}			
	}

msg_done:
	if (cs_active)
		mt7621_spi_set_cs(spi, 0);

	spin_unlock_irqrestore(&rs->lock, flags);
	m->status = status;
	spi_finalize_current_message(master);

	return 0;
}
#else
/*
 * one-byte-one for reading and writing data
 */
	static int mt7621_spi_single_transfer_half_duplex
(
 struct spi_master *master,
 struct spi_message *m
 )
{
	struct mt7621_spi *rs = spi_master_get_devdata(master);
	struct spi_device *spi = m->spi;
	unsigned int speed = spi->max_speed_hz;
	struct spi_transfer *t = NULL;
	int status = 0;
	int len = 0;
	u8 is_write = 0;
	u32 data[9] = { 0 };
	u32 val = 0;
	u32 transfer_len = 0;
	int cs_active = 0;
	//dev_info(&spi->dev, "seven spidev test ->cs:\n");
	
	udelay(10);

	mt7621_spi_wait_till_ready(spi);

	list_for_each_entry(t, &m->transfers, transfer_list) {
		const u8 *txbuf = t->tx_buf;
		u8 *rxbuf = t->rx_buf;

		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			dev_err(&spi->dev,
					"message rejected: invalid transfer data buffers\n");
			status = -EIO;
			goto msg_done;
		}

		if (rxbuf)
			is_write = 0;
		else if(txbuf)
			is_write = 1;

		if (mt7621_spi_prepare(spi, speed)) {
			status = -EIO;
			goto msg_done;
		}

		//mt7621_spi_set_cs(spi, 1);
		if (!cs_active) {
			mt7621_spi_set_cs(spi, 1);
			cs_active = 1;
		}
		transfer_len = t->len;
		if(transfer_len){	/* for bytes transfer one byte one */
			while(transfer_len){
				memset(data, 0, sizeof(data));

				if(is_write)
					data[0] = *txbuf++;	
				data[0] = swab32(data[0]);
				data[0] >>= 24;	

				val = 0;
				if(is_write){
					mt7621_spi_write(rs, MT7621_SPI_OPCODE, data[0]);

					val = 8 << 24;	/* must be 8 for single byte */
					//val |= (transfer_len - 4) * 8;		/* mosi_cnt */
				}else{
					//val |= (transfer_len* 8) << 12;			/* miso_cnt */
					val |= 8 << 12;			/* miso_cnt */
				}
				mt7621_spi_write(rs, MT7621_SPI_MOREBUF, val);
				val = mt7621_spi_read(rs, MT7621_SPI_TRANS);
				val |= SPI_CTL_START;
				mt7621_spi_write(rs, MT7621_SPI_TRANS, val);

				mt7621_spi_wait_till_ready(spi);

				if(!is_write)
					*rxbuf++ = mt7621_spi_read(rs, MT7621_SPI_DATA0);

				len ++;
				transfer_len--;
			}	//while
		}

		m->actual_length = len; 	//+ rx_len;
		//mt7621_spi_set_cs(spi, 0);
		if (t->cs_change) {
			mt7621_spi_set_cs(spi, 0);
			cs_active = 0;
		}		
	}

msg_done:
	if (cs_active)
		mt7621_spi_set_cs(spi, 0);
	m->status = status;
	spi_finalize_current_message(master);

	return 0;
}
#endif



// add end

static int mt7621_spi_transfer_one_message(struct spi_master *master,
		struct spi_message *m)
{
	struct spi_device *spi = m->spi;
	int cs = spi->chip_select;

	if (cs)
#ifdef MT7621_MB_STRANS		
		return mt7621_spi_mb_transfer_half_duplex(master, m);		/* more bytes */
#else
	return mt7621_spi_single_transfer_half_duplex(master, m);	/* single byte transfer */
#endif

	return mt7621_spi_transfer_half_duplex(master, m);
}

static int mt7621_spi_setup(struct spi_device *spi)
{
	struct mt7621_spi *rs = spidev_to_mt7621_spi(spi);

	if ((spi->max_speed_hz == 0) ||
			(spi->max_speed_hz > (rs->sys_freq / 2)))
		spi->max_speed_hz = (rs->sys_freq / 2);

	if (spi->max_speed_hz < (rs->sys_freq / 4097)) {
		dev_err(&spi->dev, "setup: requested speed is too low %d Hz\n",
				spi->max_speed_hz);
		return -EINVAL;
	}

	/**********************************************************
	  Modefiy  zkz on here
	 ***********************************************************/

	int cs  = spi->chip_select;
	int status = 0;
#if 1
	if(cs>=2) {

		//status = gpio_request_one(spi_cs[cs], (GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW), NULL);

		status = gpio_direction_output(spi_cs[cs], 1);

		dev_info(spi->master->dev.parent,"in %s , cs_gpio = %d , status = %d\n", __func__, spi_cs[cs], status);

		//gpio_free(spi_cs[cs]);
	}
#endif
	return status;
}

static const struct of_device_id mt7621_spi_match[] = {
	{ .compatible = "ralink,mt7621-spi" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7621_spi_match);

static int mt7621_spi_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct spi_master *master;
	struct mt7621_spi *rs;
	unsigned long flags;
	void __iomem *base;
	struct resource *r;
	int status = 0;
	struct clk *clk;
	struct mt7621_spi_ops *ops;

	match = of_match_device(mt7621_spi_match, &pdev->dev);
	if (!match)
		return -EINVAL;
	ops = (struct mt7621_spi_ops *)match->data;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(base))
		return PTR_ERR(base);

	clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "unable to get SYS clock, err=%d\n",
				status);
		return PTR_ERR(clk);
	}

	status = clk_prepare_enable(clk);
	if (status)
		return status;

	master = spi_alloc_master(&pdev->dev, sizeof(*rs));
	if (master == NULL) {
		dev_info(&pdev->dev, "master allocation failed\n");
		return -ENOMEM;
	}

	master->mode_bits = RT2880_SPI_MODE_BITS;

	master->setup = mt7621_spi_setup;
	master->transfer_one_message = mt7621_spi_transfer_one_message;
	//master->bits_per_word_mask = SPI_BPW_RANGE_MASK(8, 16);
	master->bits_per_word_mask = SPI_BPW_MASK(8);
	master->dev.of_node = pdev->dev.of_node;

	/**********************************************************
	  Modefiy  zkz on here
	 ***********************************************************/
#if 0
	master->num_chipselect = 2;
#else
	master->num_chipselect = 10;
#endif 

	dev_set_drvdata(&pdev->dev, master);

	rs = spi_master_get_devdata(master);
	rs->base = base;
	rs->clk = clk;
	rs->master = master;
	rs->sys_freq = clk_get_rate(rs->clk);
	rs->ops = ops;
	dev_info(&pdev->dev, "sys_freq: %u\n", rs->sys_freq);
	spin_lock_irqsave(&rs->lock, flags);

	device_reset(&pdev->dev);

	mt7621_spi_reset(rs);

	return spi_register_master(master);
}

static int mt7621_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master;
	struct mt7621_spi *rs;

	master = dev_get_drvdata(&pdev->dev);
	rs = spi_master_get_devdata(master);

	clk_disable(rs->clk);
	spi_unregister_master(master);

	return 0;
}

MODULE_ALIAS("platform:" DRIVER_NAME);

static struct platform_driver mt7621_spi_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = mt7621_spi_match,
	},
	.probe = mt7621_spi_probe,
	.remove = mt7621_spi_remove,
};

module_platform_driver(mt7621_spi_driver);

MODULE_DESCRIPTION("MT7621 SPI driver");
MODULE_AUTHOR("Felix Fietkau <nbd@openwrt.org>");
MODULE_LICENSE("GPL");
