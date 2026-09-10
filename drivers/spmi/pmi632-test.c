// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spmi.h>
#include <linux/device.h>

struct bus_type spmi_bus_type;
int spmi_ext_register_readl(struct spmi_device *sdev, u16 addr, u8 *buf, size_t len);

static int match_dev(struct device *dev, const void *data)
{
	return strcmp(dev_name(dev), "0-03") == 0;
}

static int read_lcdb_reg(const u8 reg)
{
	struct device *dev;
	struct spmi_device *sdev;
	u8 buf;
	int rc;

	dev = bus_find_device(&spmi_bus_type, NULL, NULL, match_dev);
	if (!dev) {
		pr_info("LCDB: 0-03 not found\n");
		return -ENODEV;
	}
	sdev = to_spmi_device(dev);
	rc = spmi_ext_register_readl(sdev, 0xec00 + reg, &buf, 1);
	put_device(dev);
	if (rc) {
		pr_info("LCDB+0x%02x: rc=%d\n", reg, rc);
		return rc;
	}
	pr_info("LCDB+0x%02x = 0x%02x\n", reg, buf);
	return 0;
}

static int __init lcdb_test_init(void)
{
	read_lcdb_reg(0x46);
	read_lcdb_reg(0x71);
	read_lcdb_reg(0x81);
	read_lcdb_reg(0x08);
	read_lcdb_reg(0x0a);
	read_lcdb_reg(0x0b);
	return -EAGAIN;
}
module_init(lcdb_test_init);
MODULE_LICENSE("GPL");
