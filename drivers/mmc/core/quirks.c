/*
 *  This file contains work-arounds for many known SD/MMC
 *  and SDIO hardware bugs.
 *
 *  Copyright (c) 2011 Andrei Warkentin <andreiw@motorola.com>
 *  Copyright (c) 2011 Pierre Tardy <tardyp@gmail.com>
 *  Inspired from pci fixup code:
 *  Copyright (c) 1999 Martin Mares <mj@ucw.cz>
 *
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/mmc/card.h>

#ifdef CONFIG_HUAWEI_KERNEL
#include <linux/slab.h> 
#include <linux/scatterlist.h> 
#include <linux/mmc/mmc.h> 
#include <linux/mmc/host.h> 
#include <linux/dma-mapping.h>
#include "mmc_ops.h" 
#endif
#ifndef SDIO_VENDOR_ID_TI
#define SDIO_VENDOR_ID_TI		0x0097
#endif

#ifndef SDIO_DEVICE_ID_TI_WL1271
#define SDIO_DEVICE_ID_TI_WL1271	0x4076
#endif

#ifndef SDIO_VENDOR_ID_STE
#define SDIO_VENDOR_ID_STE		0x0020
#endif

#ifndef SDIO_DEVICE_ID_STE_CW1200
#define SDIO_DEVICE_ID_STE_CW1200	0x2280
#endif

#ifndef SDIO_VENDOR_ID_MSM
#define SDIO_VENDOR_ID_MSM		0x0070
#endif

#ifndef SDIO_DEVICE_ID_MSM_WCN1314
#define SDIO_DEVICE_ID_MSM_WCN1314	0x2881
#endif

#ifndef SDIO_VENDOR_ID_MSM_QCA
#define SDIO_VENDOR_ID_MSM_QCA		0x271
#endif

#ifndef SDIO_DEVICE_ID_MSM_QCA_AR6003_1
#define SDIO_DEVICE_ID_MSM_QCA_AR6003_1	0x300
#endif

#ifndef SDIO_DEVICE_ID_MSM_QCA_AR6003_2
#define SDIO_DEVICE_ID_MSM_QCA_AR6003_2	0x301
#endif

/*
 * This hook just adds a quirk for all sdio devices
 */
static void add_quirk_for_sdio_devices(struct mmc_card *card, int data)
{
	if (mmc_card_sdio(card))
		card->quirks |= data;
}

static const struct mmc_fixup mmc_fixup_methods[] = {
	/* by default sdio devices are considered CLK_GATING broken */
	/* good cards will be whitelisted as they are tested */
	SDIO_FIXUP(SDIO_ANY_ID, SDIO_ANY_ID,
		   add_quirk_for_sdio_devices,
		   MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_TI, SDIO_DEVICE_ID_TI_WL1271,
		   remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_MSM, SDIO_DEVICE_ID_MSM_WCN1314,
		   remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_MSM_QCA, SDIO_DEVICE_ID_MSM_QCA_AR6003_1,
		   remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_MSM_QCA, SDIO_DEVICE_ID_MSM_QCA_AR6003_2,
		   remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_TI, SDIO_DEVICE_ID_TI_WL1271,
		   add_quirk, MMC_QUIRK_NONSTD_FUNC_IF),

	SDIO_FIXUP(SDIO_VENDOR_ID_TI, SDIO_DEVICE_ID_TI_WL1271,
		   add_quirk, MMC_QUIRK_DISABLE_CD),

	SDIO_FIXUP(SDIO_VENDOR_ID_STE, SDIO_DEVICE_ID_STE_CW1200,
		   add_quirk, MMC_QUIRK_BROKEN_BYTE_MODE_512),

	END_FIXUP
};

void mmc_fixup_device(struct mmc_card *card, const struct mmc_fixup *table)
{
	const struct mmc_fixup *f;
	u64 rev = cid_rev_card(card);

	/* Non-core specific workarounds. */
	if (!table)
		table = mmc_fixup_methods;

	for (f = table; f->vendor_fixup; f++) {
		if ((f->manfid == CID_MANFID_ANY ||
		     f->manfid == card->cid.manfid) &&
		    (f->oemid == CID_OEMID_ANY ||
		     f->oemid == card->cid.oemid) &&
		    (f->name == CID_NAME_ANY ||
		     !strncmp(f->name, card->cid.prod_name,
			      sizeof(card->cid.prod_name))) &&
		    (f->cis_vendor == card->cis.vendor ||
		     f->cis_vendor == (u16) SDIO_ANY_ID) &&
		    (f->cis_device == card->cis.device ||
		     f->cis_device == (u16) SDIO_ANY_ID) &&
		    rev >= f->rev_start && rev <= f->rev_end) {
			dev_dbg(&card->dev, "calling %pF\n", f->vendor_fixup);
			f->vendor_fixup(card, f->data);
		}
	}
}

#ifdef CONFIG_HUAWEI_KERNEL
/* 
* Quirk code to get smart report from Samsung emmc chips 
*/ 
int mmc_movi_vendor_cmd(struct mmc_card *card, unsigned int arg) 
{ 
    struct mmc_command cmd = {0}; 
    int err; 
    u32 status; 

    /* CMD62 is vendor CMD, it's not defined in eMMC spec. */ 
    cmd.opcode = 62; 
    cmd.flags = MMC_RSP_R1B; 
    cmd.arg = arg; 
    err = mmc_wait_for_cmd(card->host, &cmd, 0); 

    if (err) 
        return err; 

    do { 
        err = mmc_send_status(card, &status); 
        if (err) 
            return err; 

        if (card->host->caps & MMC_CAP_WAIT_WHILE_BUSY) 
            break; 

        if (mmc_host_is_spi(card->host)) 
            break; 
        } while (R1_CURRENT_STATE(status) == R1_STATE_PRG); 

    return err; 
}
static int mmc_movi_read_cmd(struct mmc_card *card, u8 *buffer)
{
    struct mmc_command wcmd={0}; 
    struct mmc_data wdata={0}; 
	struct mmc_request brq={0};
	struct scatterlist sg={0};
    int len = 512;

	brq.cmd = &wcmd;
	brq.data = &wdata;

	wcmd.opcode = MMC_READ_SINGLE_BLOCK;
	wcmd.arg = 0;
	wcmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	wdata.blksz = len;
	brq.stop = NULL;
	wdata.blocks = 1;
	wdata.flags = MMC_DATA_READ;

	wdata.sg = &sg;
	wdata.sg_len = 1;

	sg_init_one(&sg, buffer, len);

	mmc_set_data_timeout(&wdata, card);

	mmc_wait_for_req(card->host, &brq);

	if (wcmd.error)
		return wcmd.error;
	if (wdata.error)
		return wdata.error;
	return 0;
}

extern  int mmc_send_cxd_data(struct mmc_card *card, struct mmc_host *host,
		u32 opcode, void *buf, unsigned len);

static int mmc_samsung_smart_read(struct mmc_card *card, u8 *rdblock) 
{ 
    int err, errx=0; 

    /* enter vendor Smart Report mode */ 
    err = mmc_movi_vendor_cmd(card, 0xEFAC62EC); 
    if (err) 
    { 
        pr_err("Failed entering Smart Report mode(1, %d)\n", err); 
        return err; 
    } 
    err = mmc_movi_vendor_cmd(card, 0x0000CCEE); 
    if (err) { 
        pr_err("Failed entering Smart Report mode(2, %d)\n", err); 
        return err; 
    } 
    /* read Smart Report */ 
    err = mmc_movi_read_cmd(card, rdblock); 
    if (err) 
        pr_err("Failed reading Smart Report(%d)\n", err); 
    
    /* Do NOT return yet; we must leave Smart Report mode.*/ 
    /* exit vendor Smart Report mode */ 
    errx = mmc_movi_vendor_cmd(card, 0xEFAC62EC); 
    if (errx) 
        pr_err("Failed exiting Smart Report mode(1, %d)\n", errx); 
    else { 
        errx = mmc_movi_vendor_cmd(card, 0x00DECCEE); 
        if (errx) 
            pr_err("Failed exiting Smart Report mode(2, %d)\n",errx); 
    } 
    if (err) 
        return err; 
    return errx; 
} 
ssize_t mmc_samsung_smart_parse(u32 *report, char *for_sysfs) 
{ 
    unsigned size = PAGE_SIZE; 
    unsigned wrote; 
    unsigned i; 
    u32 val; 
    char *str; 
    static const struct { 
        char *fmt; 
        unsigned val_index; 
    } to_output[] = { 
            { "super block size              : %u\n", 1 }, 
            { "super page size               : %u\n", 2 }, 
            { "optimal write size            : %u\n", 3 }, 
            { "read reclaim count            : %u\n", 20 }, 
            { "optimal trim size             : %u\n", 21 }, 
            { "number of banks               : %u\n", 4 }, 
            { "initial bad blocks per bank   : %u",   5 }, 
            { ",%u",                                  8 }, 
            { ",%u",                                  11 }, 
            { ",%u\n",                                14 }, 
            { "runtime bad blocks per bank   : %u",   6 }, 
            { ",%u",                                  9 }, 
            { ",%u",                                  12 }, 
            { ",%u\n",                                15 }, 
            { "reserved blocks left per bank : %u",   7 }, 
            { ",%u",                                  10 }, 
            { ",%u",                                  13 }, 
            { ",%u\n",                                16 }, 
            { "all erase counts (min,avg,max): %u",   18 }, 
            { ",%u",                                  19 }, 
            { ",%u\n",                                17 }, 
            { "SLC erase counts (min,avg,max): %u",   31 }, 
            { ",%u",                                  32 }, 
            { ",%u\n",                                30 }, 
            { "MLC erase counts (min,avg,max): %u",   34 }, 
            { ",%u",                                  35 }, 
            { ",%u\n",                                33 }, 
    }; 
    
    /* A version field just in case things change. */ 
    wrote = scnprintf(for_sysfs, size, 
                        "version                       : %u\n", 0); 
    size -= wrote; 
    for_sysfs += wrote; 
    /* The error mode. */ 
    val = le32_to_cpu(report[0]); 
    switch (val) { 
        case 0xD2D2D2D2: 
            str = "Normal"; 
            break; 
        case 0x5C5C5C5C: 
            str = "RuntimeFatalError"; 
            break; 
        case 0xE1E1E1E1: 
            str = "MetaBrokenError"; 
            break; 
        case 0x37373737: 
            str = "OpenFatalError"; 
            val = 0; /* Remaining data is unreliable. */ 
        break; 
        default: 
            str = "Invalid"; 
            val = 0; /* Remaining data is unreliable. */ 
            break; 
    } 
    wrote = scnprintf(for_sysfs, size, 
                        "error mode                    : %s\n", str); 
    size -= wrote; 
    for_sysfs += wrote; 
    /* Exit if we can't rely on the remaining data. */ 
    if (!val) 
        return PAGE_SIZE - size; 
    for (i = 0; i < ARRAY_SIZE(to_output); i++) { 
        wrote = scnprintf(for_sysfs, size, to_output[i].fmt, 
        le32_to_cpu(report[to_output[i].val_index])); 
        size -= wrote; 
        for_sysfs += wrote; 
    } 
    return PAGE_SIZE - size; 
} 

ssize_t mmc_samsung_smart_handle(struct mmc_card *card, char *buf) 
{ 
    int err; 
    u32 *buffer; 
    ssize_t len; 
 
    buffer = kmalloc(512, GFP_KERNEL); 
    if (!buffer) { 
        pr_err("Failed to alloc memory for Smart Report\n"); 
        return 0; 
    } 
     
    mmc_claim_host(card->host); 
    err = mmc_samsung_smart_read(card, (u8 *)buffer); 
    mmc_release_host(card->host); 

    if (err) 
        len = 0; 
    else 
        len = mmc_samsung_smart_parse(buffer, buf); 

    kfree(buffer); 
    return len; 
} 

#endif /* CONFIG_HUAWEI_KERNEL */ 

EXPORT_SYMBOL(mmc_fixup_device);
