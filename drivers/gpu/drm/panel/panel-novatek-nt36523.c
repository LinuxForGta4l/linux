// SPDX-License-Identifier: GPL-2.0-only
/*
 * Novatek NT36523 DriverIC panels driver
 *
 * Copyright (c) 2022, 2023 Jianhua Lu <lujianhua000@gmail.com>
 */

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_graph.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_connector.h>
#include <drm/drm_crtc.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

#define DSI_NUM_MIN 1

struct panel_info {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi[2];
	const struct panel_desc *desc;
	enum drm_panel_orientation orientation;

	struct gpio_desc *reset_gpio;
	struct gpio_desc *enable_gpio;
	struct backlight_device *backlight;
	struct regulator *vddio;
};

struct panel_desc {
	unsigned int width_mm;
	unsigned int height_mm;

	unsigned int bpc;
	unsigned int lanes;
	unsigned long mode_flags;
	enum mipi_dsi_pixel_format format;

	const struct drm_display_mode *modes;
	unsigned int num_modes;
	const struct mipi_dsi_device_info dsi_info;
	int (*init_sequence)(struct panel_info *pinfo);
	bool (*panel_id_check)(u8 *buf, int len);

	bool is_dual_dsi;
	bool has_dcs_backlight;
};

static inline struct panel_info *to_panel_info(struct drm_panel *panel)
{
	return container_of(panel, struct panel_info, panel);
}

static int elish_boe_init_sequence(struct panel_info *pinfo)
{
	struct mipi_dsi_device *dsi0 = pinfo->dsi[0];
	struct mipi_dsi_device *dsi1 = pinfo->dsi[1];
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = NULL };
	/* No datasheet, so write magic init sequence directly */
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb9, 0x05);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x20);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x18, 0x40);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb9, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x23);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x00, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x01, 0x84);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x05, 0x2d);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x06, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x07, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x08, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x09, 0x45);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x11, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x12, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x15, 0x83);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x16, 0x0c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x29, 0x0a);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x30, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x31, 0xfe);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x32, 0xfd);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x33, 0xfb);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x34, 0xf8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x35, 0xf5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x36, 0xf3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x37, 0xf2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x38, 0xf2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x39, 0xf2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3a, 0xef);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3b, 0xec);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3d, 0xe9);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3f, 0xe5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x40, 0xe5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x41, 0xe5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2a, 0x13);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x45, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x46, 0xf4);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x47, 0xe7);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x48, 0xda);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x49, 0xcd);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4a, 0xc0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4b, 0xb3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4c, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4d, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4e, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4f, 0x99);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x50, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x51, 0x68);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x52, 0x66);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x53, 0x66);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x54, 0x66);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2b, 0x0e);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x58, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x59, 0xfb);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5a, 0xf7);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5b, 0xf3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5c, 0xef);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5d, 0xe3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5e, 0xda);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5f, 0xd8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x60, 0xd8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x61, 0xd8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x62, 0xcb);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x63, 0xbf);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x64, 0xb3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x65, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x66, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x67, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x2a);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x25, 0x47);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x30, 0x47);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x39, 0x47);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x26);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x19, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1a, 0xe0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1b, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1c, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2a, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2b, 0xe0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xf0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x84, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x85, 0x0c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x20);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x51, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x25);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x91, 0x1f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x92, 0x0f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x93, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x94, 0x18);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x95, 0x03);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x96, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb0, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x25);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x19, 0x1f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1b, 0x1b);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x24);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb8, 0x28);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x27);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd0, 0x31);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd1, 0x20);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd2, 0x30);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd4, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xde, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xdf, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x26);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x00, 0x81);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x01, 0xb0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x22);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9f, 0x50);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x6f, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x70, 0x11);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x73, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x74, 0x49);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x76, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x77, 0x49);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xa0, 0x3f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xa9, 0x50);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xaa, 0x28);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xab, 0x28);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xad, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb8, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb9, 0x49);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xba, 0x49);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbb, 0x49);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbe, 0x04);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbf, 0x49);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc0, 0x04);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc1, 0x59);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc2, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc5, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc6, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc7, 0x48);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xca, 0x43);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xcb, 0x3c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xce, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xcf, 0x43);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd0, 0x3c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd3, 0x43);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd4, 0x3c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd7, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xdc, 0x43);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xdd, 0x3c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xe1, 0x43);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xe2, 0x3c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xf2, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xf3, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xf4, 0x48);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x25);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x13, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x14, 0x23);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbc, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbd, 0x23);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x2a);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x97, 0x3c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x98, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x99, 0x95);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9a, 0x03);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9b, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9c, 0x0b);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9d, 0x0a);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9e, 0x90);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x22);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9f, 0x50);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x23);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xa3, 0x50);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xe0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x14, 0x60);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x16, 0xc0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4f, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xf0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3a, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xd0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x02, 0xaf);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x09, 0xee);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1c, 0x99);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1d, 0x09);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x51, 0x0f, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x53, 0x2c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x35, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbb, 0x13);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3b, 0x03, 0xac, 0x1a, 0x04, 0x04);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 70);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x29);

	return dsi_ctx.accum_err;
}

static int elish_csot_init_sequence(struct panel_info *pinfo)
{
	struct mipi_dsi_device *dsi0 = pinfo->dsi[0];
	struct mipi_dsi_device *dsi1 = pinfo->dsi[1];
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = NULL };
	/* No datasheet, so write magic init sequence directly */
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb9, 0x05);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x20);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x18, 0x40);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb9, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xd0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x02, 0xaf);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x00, 0x30);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x09, 0xee);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1c, 0x99);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1d, 0x09);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xf0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3a, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xe0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4f, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x20);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x58, 0x40);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x35, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x23);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x00, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x01, 0x84);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x05, 0x2d);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x06, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x07, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x08, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x09, 0x45);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x11, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x12, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x15, 0x83);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x16, 0x0c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x29, 0x0a);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x30, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x31, 0xfe);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x32, 0xfd);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x33, 0xfb);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x34, 0xf8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x35, 0xf5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x36, 0xf3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x37, 0xf2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x38, 0xf2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x39, 0xf2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3a, 0xef);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3b, 0xec);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3d, 0xe9);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3f, 0xe5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x40, 0xe5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x41, 0xe5);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2a, 0x13);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x45, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x46, 0xf4);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x47, 0xe7);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x48, 0xda);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x49, 0xcd);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4a, 0xc0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4b, 0xb3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4c, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4d, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4e, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x4f, 0x99);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x50, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x51, 0x68);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x52, 0x66);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x53, 0x66);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x54, 0x66);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2b, 0x0e);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x58, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x59, 0xfb);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5a, 0xf7);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5b, 0xf3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5c, 0xef);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5d, 0xe3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5e, 0xda);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x5f, 0xd8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x60, 0xd8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x61, 0xd8);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x62, 0xcb);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x63, 0xbf);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x64, 0xb3);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x65, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x66, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x67, 0xb2);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x51, 0x0f, 0xff);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x53, 0x2c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x55, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbb, 0x13);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x3b, 0x03, 0xac, 0x1a, 0x04, 0x04);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x2a);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x25, 0x46);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x30, 0x46);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x39, 0x46);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x26);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x01, 0xb0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x19, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1a, 0xe0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1b, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1c, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2a, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x2b, 0xe0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0xf0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x84, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x85, 0x0c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x20);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x51, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x25);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x91, 0x1f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x92, 0x0f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x93, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x94, 0x18);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x95, 0x03);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x96, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb0, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x25);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x19, 0x1f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x1b, 0x1b);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x24);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb8, 0x28);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x27);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd0, 0x31);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd1, 0x20);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd4, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xde, 0x80);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xdf, 0x02);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x26);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x00, 0x81);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x01, 0xb0);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x22);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x6f, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x70, 0x11);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x73, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x74, 0x4d);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xa0, 0x3f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xa9, 0x50);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xaa, 0x28);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xab, 0x28);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xad, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb8, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xb9, 0x4b);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xba, 0x96);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbb, 0x4b);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbe, 0x07);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbf, 0x4b);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc0, 0x07);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc1, 0x5c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc2, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc5, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc6, 0x3f);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xc7, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xca, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xcb, 0x40);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xce, 0x00);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xcf, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd0, 0x40);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd3, 0x08);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xd4, 0x40);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x25);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbc, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xbd, 0x1c);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x2a);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xfb, 0x01);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x9a, 0x03);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0xff, 0x10);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 70);
	mipi_dsi_dual_dcs_write_seq_multi(&dsi_ctx, dsi0, dsi1, 0x29);

	return dsi_ctx.accum_err;
}

static int j606f_boe_init_sequence(struct panel_info *pinfo)
{
	struct mipi_dsi_device *dsi = pinfo->dsi[0];
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x78);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x5a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x63);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0xeb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0xeb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_PARTIAL_ROWS, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6d, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0xa2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x77, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x00, 0x08, 0x00, 0x23, 0x00, 0x4d, 0x00, 0x6d,
				     0x00, 0x89, 0x00, 0xa1, 0x00, 0xb6, 0x00, 0xc9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0x00, 0xda, 0x01, 0x13, 0x01, 0x3c, 0x01, 0x7e,
				     0x01, 0xab, 0x01, 0xf7, 0x02, 0x2f, 0x02, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x02, 0x67, 0x02, 0xa6, 0x02, 0xd1, 0x03, 0x08,
				     0x03, 0x2e, 0x03, 0x5b, 0x03, 0x6b, 0x03, 0x7b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0x03, 0x8e, 0x03, 0xa2, 0x03, 0xb7, 0x03, 0xe7,
				     0x03, 0xfd, 0x03, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4, 0x00, 0x08, 0x00, 0x23, 0x00, 0x4d, 0x00, 0x6d,
				     0x00, 0x89, 0x00, 0xa1, 0x00, 0xb6, 0x00, 0xc9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0x00, 0xda, 0x01, 0x13, 0x01, 0x3c, 0x01, 0x7e,
				     0x01, 0xab, 0x01, 0xf7, 0x02, 0x2f, 0x02, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0x02, 0x67, 0x02, 0xa6, 0x02, 0xd1, 0x03, 0x08,
				     0x03, 0x2e, 0x03, 0x5b, 0x03, 0x6b, 0x03, 0x7b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7, 0x03, 0x8e, 0x03, 0xa2, 0x03, 0xb7, 0x03, 0xe7,
				     0x03, 0xfd, 0x03, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8, 0x00, 0x08, 0x00, 0x23, 0x00, 0x4d, 0x00, 0x6d,
				     0x00, 0x89, 0x00, 0xa1, 0x00, 0xb6, 0x00, 0xc9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x00, 0xda, 0x01, 0x13, 0x01, 0x3c, 0x01, 0x7e,
				     0x01, 0xab, 0x01, 0xf7, 0x02, 0x2f, 0x02, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba, 0x02, 0x67, 0x02, 0xa6, 0x02, 0xd1, 0x03, 0x08,
				     0x03, 0x2e, 0x03, 0x5b, 0x03, 0x6b, 0x03, 0x7b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x03, 0x8e, 0x03, 0xa2, 0x03, 0xb7, 0x03, 0xe7,
				     0x03, 0xfd, 0x03, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x45, 0x00, 0x65,
				     0x00, 0x81, 0x00, 0x99, 0x00, 0xae, 0x00, 0xc1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0x00, 0xd2, 0x01, 0x0b, 0x01, 0x34, 0x01, 0x76,
				     0x01, 0xa3, 0x01, 0xef, 0x02, 0x27, 0x02, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x02, 0x5f, 0x02, 0x9e, 0x02, 0xc9, 0x03, 0x00,
				     0x03, 0x26, 0x03, 0x53, 0x03, 0x63, 0x03, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0x03, 0x86, 0x03, 0x9a, 0x03, 0xaf, 0x03, 0xdf,
				     0x03, 0xf5, 0x03, 0xf7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x45, 0x00, 0x65,
				     0x00, 0x81, 0x00, 0x99, 0x00, 0xae, 0x00, 0xc1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0x00, 0xd2, 0x01, 0x0b, 0x01, 0x34, 0x01, 0x76,
				     0x01, 0xa3, 0x01, 0xef, 0x02, 0x27, 0x02, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0x02, 0x5f, 0x02, 0x9e, 0x02, 0xc9, 0x03, 0x00,
				     0x03, 0x26, 0x03, 0x53, 0x03, 0x63, 0x03, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7, 0x03, 0x86, 0x03, 0x9a, 0x03, 0xaf, 0x03, 0xdf,
				     0x03, 0xf5, 0x03, 0xf7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x45, 0x00, 0x65,
				     0x00, 0x81, 0x00, 0x99, 0x00, 0xae, 0x00, 0xc1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x00, 0xd2, 0x01, 0x0b, 0x01, 0x34, 0x01, 0x76,
				     0x01, 0xa3, 0x01, 0xef, 0x02, 0x27, 0x02, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba, 0x02, 0x5f, 0x02, 0x9e, 0x02, 0xc9, 0x03, 0x00,
				     0x03, 0x26, 0x03, 0x53, 0x03, 0x63, 0x03, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x03, 0x86, 0x03, 0x9a, 0x03, 0xaf, 0x03, 0xdf,
				     0x03, 0xf5, 0x03, 0xf7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_GAMMA_CURVE, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_LUT, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_PARTIAL_ROWS, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);

	mipi_dsi_dcs_set_pixel_format_multi(&dsi_ctx, 0x9a);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_3D_CONTROL, 0x42);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0x9a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x43);

	mipi_dsi_dcs_set_display_brightness_multi(&dsi_ctx, 18);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x82, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_CABC_MIN_BRIGHTNESS, 0x00, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x82);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7e, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7f, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x82, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				     0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
				     0x05, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x5f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8d, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8e, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_GAMMA_CURVE, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0xe0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_VSYNC_TIMING, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_GET_SCANLINE, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_CABC_MIN_BRIGHTNESS, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_READ_PPS_START, 0xf3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa5, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0xa1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0xf2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x51);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x65);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x7b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x7b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_PARTIAL_ROWS, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_PARTIAL_COLUMNS, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);

	mipi_dsi_dcs_set_pixel_format_multi(&dsi_ctx, 0xc3);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x78);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x82);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x4e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_READ_PPS_CONTINUE, 0x4c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x47);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x53);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_CABC_MIN_BRIGHTNESS, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x2f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_GAMMA_CURVE, 0xf8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_LUT, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xe0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);

	mipi_dsi_dcs_set_pixel_format_multi(&dsi_ctx, 0x08);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);

	mipi_dsi_dcs_set_pixel_format_multi(&dsi_ctx, 0x5d);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0x5d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_GAMMA_CURVE, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_PARTIAL_ROWS, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_PARTIAL_COLUMNS, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0x8d);

	mipi_dsi_dcs_set_pixel_format_multi(&dsi_ctx, 0x75);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x75);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x02);

	mipi_dsi_dcs_set_tear_on_multi(&dsi_ctx, MIPI_DSI_DCS_TEAR_MODE_VBLANK);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x03, 0x5f, 0x1a, 0x04, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_usleep_range(&dsi_ctx, 10000, 11000);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);

	mipi_dsi_dcs_set_display_brightness_multi(&dsi_ctx, 0);

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x05, 0x01);

	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 100);

	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 30);

	return dsi_ctx.accum_err;
}

/* Parsed 357 DSI commands */

static int gta4l_lide_hsd_init_sequence(struct panel_info *pinfo)
{
	struct mipi_dsi_device *dsi = pinfo->dsi[0];
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0xc9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x87);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x37);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x63);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0xa5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0xa2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
				0x00, 0x08, 0x00, 0x2a, 0x00, 0x5b, 0x00, 0x80,
				0x00, 0x9f, 0x00, 0xb9, 0x00, 0xd0, 0x00, 0xe4
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
				0x00, 0xf6, 0x01, 0x31, 0x01, 0x59, 0x01, 0x9a,
				0x01, 0xc5, 0x02, 0x0a, 0x02, 0x3a, 0x02, 0x3c
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				0x02, 0x6d, 0x02, 0xa9, 0x02, 0xd8, 0x03, 0x0b,
				0x03, 0x36, 0x03, 0x63, 0x03, 0x72, 0x03, 0x80
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
				0x03, 0x91, 0x03, 0xa3, 0x03, 0xb8, 0x03, 0xcc,
				0x03, 0xd9, 0x03, 0xdb
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
				0x00, 0x08, 0x00, 0x2a, 0x00, 0x5b, 0x00, 0x80,
				0x00, 0x9f, 0x00, 0xb9, 0x00, 0xd0, 0x00, 0xe4
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5,
				0x00, 0xf6, 0x01, 0x31, 0x01, 0x59, 0x01, 0x9a,
				0x01, 0xc5, 0x02, 0x0a, 0x02, 0x3a, 0x02, 0x3c
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x02, 0x6d, 0x02, 0xa9, 0x02, 0xd8, 0x03, 0x0b,
				0x03, 0x36, 0x03, 0x63, 0x03, 0x72, 0x03, 0x80
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
				0x03, 0x91, 0x03, 0xa3, 0x03, 0xb8, 0x03, 0xcc,
				0x03, 0xd9, 0x03, 0xdb
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
				0x00, 0x08, 0x00, 0x2a, 0x00, 0x5b, 0x00, 0x80,
				0x00, 0x9f, 0x00, 0xb9, 0x00, 0xd0, 0x00, 0xe4
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9,
				0x00, 0xf6, 0x01, 0x31, 0x01, 0x59, 0x01, 0x9a,
				0x01, 0xc5, 0x02, 0x0a, 0x02, 0x3a, 0x02, 0x3c
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
				0x02, 0x6d, 0x02, 0xa9, 0x02, 0xd8, 0x03, 0x0b,
				0x03, 0x36, 0x03, 0x63, 0x03, 0x72, 0x03, 0x80
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb,
				0x03, 0x91, 0x03, 0xa3, 0x03, 0xb8, 0x03, 0xcc,
				0x03, 0xd9, 0x03, 0xdb
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd5, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd9, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
				0x00, 0x00, 0x00, 0x22, 0x00, 0x53, 0x00, 0x78,
				0x00, 0x97, 0x00, 0xb1, 0x00, 0xc8, 0x00, 0xdc
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
				0x00, 0xee, 0x01, 0x29, 0x01, 0x51, 0x01, 0x92,
				0x01, 0xbd, 0x02, 0x02, 0x02, 0x32, 0x02, 0x34
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				0x02, 0x65, 0x02, 0xa1, 0x02, 0xd0, 0x03, 0x03,
				0x03, 0x2e, 0x03, 0x5b, 0x03, 0x6a, 0x03, 0x78
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
				0x03, 0x89, 0x03, 0x9b, 0x03, 0xb0, 0x03, 0xc4,
				0x03, 0xd1, 0x03, 0xd3
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
				0x00, 0x00, 0x00, 0x22, 0x00, 0x53, 0x00, 0x78,
				0x00, 0x97, 0x00, 0xb1, 0x00, 0xc8, 0x00, 0xdc
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5,
				0x00, 0xee, 0x01, 0x29, 0x01, 0x51, 0x01, 0x92,
				0x01, 0xbd, 0x02, 0x02, 0x02, 0x32, 0x02, 0x34
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x02, 0x65, 0x02, 0xa1, 0x02, 0xd0, 0x03, 0x03,
				0x03, 0x2e, 0x03, 0x5b, 0x03, 0x6a, 0x03, 0x78
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
				0x03, 0x89, 0x03, 0x9b, 0x03, 0xb0, 0x03, 0xc4,
				0x03, 0xd1, 0x03, 0xd3
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
				0x00, 0x00, 0x00, 0x22, 0x00, 0x53, 0x00, 0x78,
				0x00, 0x97, 0x00, 0xb1, 0x00, 0xc8, 0x00, 0xdc
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9,
				0x00, 0xee, 0x01, 0x29, 0x01, 0x51, 0x01, 0x92,
				0x01, 0xbd, 0x02, 0x02, 0x02, 0x32, 0x02, 0x34
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
				0x02, 0x65, 0x02, 0xa1, 0x02, 0xd0, 0x03, 0x03,
				0x03, 0x2e, 0x03, 0x5b, 0x03, 0x6a, 0x03, 0x78
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb,
				0x03, 0x89, 0x03, 0x9b, 0x03, 0xb0, 0x03, 0xc4,
				0x03, 0xd1, 0x03, 0xd3
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0xca);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x42);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0x52);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x42, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0xb0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x82);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7e, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7f, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x80, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x82, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0xc2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe9, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xea, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xeb, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xee, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x05, 0x05, 0x00, 0x00
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0xb0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0xb0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0xb0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0xa1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x6d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x93);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0xd2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0xd2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0xd2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0xd2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x41, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x42, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8b, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8d, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8f, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x98, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x99, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9c, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9e, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x4e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x79);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x85);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xac, 0x8b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xae, 0x97);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaf, 0x9d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0xa3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0xa9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0xaf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0xb5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0xc1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0xc7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8, 0xd3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x79, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7c, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x98, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x99, 0x95);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9c, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9e, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa2, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xd2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0xf8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0xee);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x03, 0x08, 0x1a, 0x04, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x02, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29);
	mipi_dsi_msleep(&dsi_ctx, 30);

	return dsi_ctx.accum_err;
}


/* Parsed 341 DSI commands, ported from t500 ov13 dsi_nt36523_hlt_auo_video */

static int gta4l_hlt_auo_init_sequence(struct panel_info *pinfo)
{
	struct mipi_dsi_device *dsi = pinfo->dsi[0];
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x37);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x1e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0xb1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x88, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf2, 0x51);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
			0x00, 0x08, 0x00, 0x2f, 0x00, 0x63, 0x00, 0x88,
			0x00, 0xa7, 0x00, 0xc0, 0x00, 0xd6, 0x00, 0xe8
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
			0x00, 0xfa, 0x01, 0x32, 0x01, 0x59, 0x01, 0x97,
			0x01, 0xc1, 0x02, 0x03, 0x02, 0x31, 0x02, 0x33
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
			0x02, 0x61, 0x02, 0x97, 0x02, 0xc0, 0x02, 0xf2,
			0x03, 0x1c, 0x03, 0x4a, 0x03, 0x5c, 0x03, 0x6d
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
			0x03, 0x82, 0x03, 0x99, 0x03, 0xb1, 0x03, 0xc3,
			0x03, 0xcc, 0x03, 0xcd
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
			0x00, 0x08, 0x00, 0x2f, 0x00, 0x63, 0x00, 0x88,
			0x00, 0xa7, 0x00, 0xc0, 0x00, 0xd6, 0x00, 0xe8
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5,
			0x00, 0xfa, 0x01, 0x32, 0x01, 0x59, 0x01, 0x97,
			0x01, 0xc1, 0x02, 0x03, 0x02, 0x31, 0x02, 0x33
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
			0x02, 0x61, 0x02, 0x97, 0x02, 0xc0, 0x02, 0xf2,
			0x03, 0x1c, 0x03, 0x4a, 0x03, 0x5c, 0x03, 0x6d
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
			0x03, 0x82, 0x03, 0x99, 0x03, 0xb1, 0x03, 0xc3,
			0x03, 0xcc, 0x03, 0xcd
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
			0x00, 0x08, 0x00, 0x2f, 0x00, 0x63, 0x00, 0x88,
			0x00, 0xa7, 0x00, 0xc0, 0x00, 0xd6, 0x00, 0xe8
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9,
			0x00, 0xfa, 0x01, 0x32, 0x01, 0x59, 0x01, 0x97,
			0x01, 0xc1, 0x02, 0x03, 0x02, 0x31, 0x02, 0x33
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
			0x02, 0x61, 0x02, 0x97, 0x02, 0xc0, 0x02, 0xf2,
			0x03, 0x1c, 0x03, 0x4a, 0x03, 0x5c, 0x03, 0x6d
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb,
			0x03, 0x82, 0x03, 0x99, 0x03, 0xb1, 0x03, 0xc3,
			0x03, 0xcc, 0x03, 0xcd
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
			0x00, 0x00, 0x00, 0x27, 0x00, 0x5b, 0x00, 0x80,
			0x00, 0x9f, 0x00, 0xb8, 0x00, 0xce, 0x00, 0xe0
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
			0x00, 0xf2, 0x01, 0x2a, 0x01, 0x51, 0x01, 0x8f,
			0x01, 0xb9, 0x01, 0xfb, 0x02, 0x29, 0x02, 0x2b
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
			0x02, 0x59, 0x02, 0x8f, 0x02, 0xb8, 0x02, 0xea,
			0x03, 0x14, 0x03, 0x42, 0x03, 0x54, 0x03, 0x65
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
			0x03, 0x7a, 0x03, 0x91, 0x03, 0xa9, 0x03, 0xbb,
			0x03, 0xc4, 0x03, 0xc5
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
			0x00, 0x00, 0x00, 0x27, 0x00, 0x5b, 0x00, 0x80,
			0x00, 0x9f, 0x00, 0xb8, 0x00, 0xce, 0x00, 0xe0
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
			0x00, 0xf2, 0x01, 0x2a, 0x01, 0x51, 0x01, 0x8f,
			0x01, 0xb9, 0x01, 0xfb, 0x02, 0x29, 0x02, 0x2b
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
			0x02, 0x59, 0x02, 0x8f, 0x02, 0xb8, 0x02, 0xea,
			0x03, 0x14, 0x03, 0x42, 0x03, 0x54, 0x03, 0x65
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
			0x03, 0x7a, 0x03, 0x91, 0x03, 0xa9, 0x03, 0xbb,
			0x03, 0xc4, 0x03, 0xc5
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
			0x00, 0x00, 0x00, 0x27, 0x00, 0x5b, 0x00, 0x80,
			0x00, 0x9f, 0x00, 0xb8, 0x00, 0xce, 0x00, 0xe0
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
			0x00, 0xf2, 0x01, 0x2a, 0x01, 0x51, 0x01, 0x8f,
			0x01, 0xb9, 0x01, 0xfb, 0x02, 0x29, 0x02, 0x2b
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
			0x02, 0x59, 0x02, 0x8f, 0x02, 0xb8, 0x02, 0xea,
			0x03, 0x14, 0x03, 0x42, 0x03, 0x54, 0x03, 0x65
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
			0x03, 0x7a, 0x03, 0x91, 0x03, 0xa9, 0x03, 0xbb,
			0x03, 0xc4, 0x03, 0xc5
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x42);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x42);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x42);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x42);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x54, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x84, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x00, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0xc2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
			0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x05, 0x05, 0x00, 0x00
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0xc6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc4, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd5, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x71);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6b, 0x62);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6c, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6d, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6e, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x71, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x72, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x73, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x79, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7a, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7d, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7e, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7f, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x86, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x89, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8a, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0xba);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0xa1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0xf3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x71);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x6f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0xec);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x46);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x79, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7c, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7f, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x88, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8b, 0xc9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x98, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x99, 0x95);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9c, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9e, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa2, 0x3f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa5, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0xee);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b,
			0x03, 0x08, 0x1a, 0x04, 0x04
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x02, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 100);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29);
	mipi_dsi_msleep(&dsi_ctx, 20);

	return dsi_ctx.accum_err;
}

/* Parsed 370 DSI commands */

static int gta4l_txd_inx_init_sequence(struct panel_info *pinfo)
{
	struct mipi_dsi_device *dsi = pinfo->dsi[0];
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0xb1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x4b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x69);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0xd7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9e, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x69, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0xa2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x77, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0xc6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x1d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x4c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x31);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x4c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0xb0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0x93);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xac, 0xb3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4f, 0x65);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x56);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55,
				0x82, 0x02
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e,
				0x00, 0x0c
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7e, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7f, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x80, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x81, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x82, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd8, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd9, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe9, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xea, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xeb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xee, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc5, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x38, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x41, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x42, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x54, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x1c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x69, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0xa1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0xf3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x32);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x71);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x6f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0xc7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x78);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x4e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x79);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x85);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xac, 0x8b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xae, 0x97);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaf, 0x9d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0xa3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0xa9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0xaf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x46);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x22);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x2f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0xf8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x65, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x66, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x70, 0x92);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x71, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x72, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa2, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x33);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0xee);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
				0x00, 0x08, 0x00, 0x1b, 0x00, 0x35, 0x00, 0x50,
				0x00, 0x69, 0x00, 0x81, 0x00, 0x94, 0x00, 0xa2
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
				0x00, 0xb4, 0x00, 0xec, 0x01, 0x17, 0x01, 0x5b,
				0x01, 0x8f, 0x01, 0xe3, 0x02, 0x26, 0x02, 0x27
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				0x02, 0x6b, 0x02, 0xb2, 0x02, 0xde, 0x03, 0x17,
				0x03, 0x3d, 0x03, 0x6b, 0x03, 0x7c, 0x03, 0x8d
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
				0x03, 0x9f, 0x03, 0xb3, 0x03, 0xbe, 0x03, 0xcf,
				0x03, 0xda, 0x03, 0xdc
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
				0x00, 0x08, 0x00, 0x1d, 0x00, 0x41, 0x00, 0x5d,
				0x00, 0x75, 0x00, 0x8a, 0x00, 0x9e, 0x00, 0xb1
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5,
				0x00, 0xc1, 0x00, 0xf8, 0x01, 0x24, 0x01, 0x67,
				0x01, 0x9b, 0x01, 0xed, 0x02, 0x2f, 0x02, 0x31
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x02, 0x71, 0x02, 0xb8, 0x02, 0xe6, 0x03, 0x1e,
				0x03, 0x42, 0x03, 0x70, 0x03, 0x7f, 0x03, 0x8f
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
				0x03, 0xa0, 0x03, 0xb2, 0x03, 0xc4, 0x03, 0xd3,
				0x03, 0xda, 0x03, 0xdc
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
				0x00, 0xa5, 0x00, 0xb0, 0x00, 0xbe, 0x00, 0xcd,
				0x00, 0xdd, 0x00, 0xeb, 0x00, 0xf6, 0x01, 0x00
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9,
				0x01, 0x0c, 0x01, 0x33, 0x01, 0x53, 0x01, 0x89,
				0x01, 0xb4, 0x01, 0xfd, 0x02, 0x3a, 0x02, 0x3b
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
				0x02, 0x7a, 0x02, 0xbf, 0x02, 0xec, 0x03, 0x25,
				0x03, 0x4c, 0x03, 0x74, 0x03, 0x7f, 0x03, 0x8e
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb,
				0x03, 0x9f, 0x03, 0xac, 0x03, 0xbd, 0x03, 0xce,
				0x03, 0xda, 0x03, 0xdc
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
				0x00, 0x00, 0x00, 0x13, 0x00, 0x2d, 0x00, 0x48,
				0x00, 0x61, 0x00, 0x79, 0x00, 0x8c, 0x00, 0x9a
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
				0x00, 0xac, 0x00, 0xe4, 0x01, 0x0f, 0x01, 0x53,
				0x01, 0x87, 0x01, 0xdb, 0x02, 0x1e, 0x02, 0x1f
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				0x02, 0x63, 0x02, 0xaa, 0x02, 0xd6, 0x03, 0x0f,
				0x03, 0x35, 0x03, 0x63, 0x03, 0x74, 0x03, 0x85
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
				0x03, 0x97, 0x03, 0xab, 0x03, 0xb6, 0x03, 0xc7,
				0x03, 0xd2, 0x03, 0xd4
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
				0x00, 0x00, 0x00, 0x15, 0x00, 0x39, 0x00, 0x55,
				0x00, 0x6d, 0x00, 0x82, 0x00, 0x96, 0x00, 0xa9
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5,
				0x00, 0xb9, 0x00, 0xf0, 0x01, 0x1c, 0x01, 0x5f,
				0x01, 0x93, 0x01, 0xe5, 0x02, 0x27, 0x02, 0x29
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x02, 0x69, 0x02, 0xb0, 0x02, 0xde, 0x03, 0x16,
				0x03, 0x3a, 0x03, 0x68, 0x03, 0x77, 0x03, 0x87
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
				0x03, 0x98, 0x03, 0xaa, 0x03, 0xbc, 0x03, 0xcb,
				0x03, 0xd2, 0x03, 0xd4
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
				0x00, 0x9d, 0x00, 0xa8, 0x00, 0xb6, 0x00, 0xc5,
				0x00, 0xd5, 0x00, 0xe3, 0x00, 0xee, 0x00, 0xf8
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9,
				0x01, 0x04, 0x01, 0x2b, 0x01, 0x4b, 0x01, 0x81,
				0x01, 0xac, 0x01, 0xf5, 0x02, 0x32, 0x02, 0x33
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
				0x02, 0x72, 0x02, 0xb7, 0x02, 0xe4, 0x03, 0x1d,
				0x03, 0x44, 0x03, 0x6c, 0x03, 0x77, 0x03, 0x86
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb,
				0x03, 0x97, 0x03, 0xa4, 0x03, 0xb5, 0x03, 0xc6,
				0x03, 0xd2, 0x03, 0xd4
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68,
				0x02, 0x01
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b,
				0x03, 0x08, 0x1a, 0x04, 0x04
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29);
	mipi_dsi_msleep(&dsi_ctx, 30);

	return dsi_ctx.accum_err;
}
	/* Parsed 405 DSI commands */

static int gta4l_lce_panda_init_sequence(struct panel_info *pinfo)
{
	struct mipi_dsi_device *dsi = pinfo->dsi[0];
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0xb1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0xc0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x4b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x89);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x69);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x88, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0xc3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x99);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x88);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x31, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x32, 0xc2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x75, 0xa2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x77, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
				0x00, 0x08, 0x00, 0x22, 0x00, 0x49, 0x00, 0x69,
				0x00, 0x84, 0x00, 0x9b, 0x00, 0xaf, 0x00, 0xc2
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
				0x00, 0xd2, 0x01, 0x0b, 0x01, 0x33, 0x01, 0x74,
				0x01, 0xa2, 0x01, 0xed, 0x02, 0x24, 0x02, 0x26
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				0x02, 0x5e, 0x02, 0x9b, 0x02, 0xca, 0x03, 0x03,
				0x03, 0x2d, 0x03, 0x5e, 0x03, 0x70, 0x03, 0x81
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
				0x03, 0x95, 0x03, 0xa8, 0x03, 0xbc, 0x03, 0xcc,
				0x03, 0xd5, 0x03, 0xd6
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
				0x00, 0x08, 0x00, 0x22, 0x00, 0x49, 0x00, 0x69,
				0x00, 0x84, 0x00, 0x9b, 0x00, 0xaf, 0x00, 0xc2
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5,
				0x00, 0xd2, 0x01, 0x0b, 0x01, 0x33, 0x01, 0x74,
				0x01, 0xa2, 0x01, 0xed, 0x02, 0x24, 0x02, 0x26
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x02, 0x5e, 0x02, 0x9b, 0x02, 0xca, 0x03, 0x03,
				0x03, 0x2d, 0x03, 0x5e, 0x03, 0x70, 0x03, 0x81
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
				0x03, 0x95, 0x03, 0xa8, 0x03, 0xbc, 0x03, 0xcc,
				0x03, 0xd5, 0x03, 0xd6
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
				0x00, 0x08, 0x00, 0x22, 0x00, 0x49, 0x00, 0x69,
				0x00, 0x84, 0x00, 0x9b, 0x00, 0xaf, 0x00, 0xc2
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9,
				0x00, 0xd2, 0x01, 0x0b, 0x01, 0x33, 0x01, 0x74,
				0x01, 0xa2, 0x01, 0xed, 0x02, 0x24, 0x02, 0x26
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
				0x02, 0x5e, 0x02, 0x9b, 0x02, 0xca, 0x03, 0x03,
				0x03, 0x2d, 0x03, 0x5e, 0x03, 0x70, 0x03, 0x81
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb,
				0x03, 0x95, 0x03, 0xa8, 0x03, 0xbc, 0x03, 0xcc,
				0x03, 0xd5, 0x03, 0xd6
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc6, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcc, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcd, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xce, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcf, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd5, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd6, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd9, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0,
				0x00, 0x00, 0x00, 0x1a, 0x00, 0x41, 0x00, 0x61,
				0x00, 0x7c, 0x00, 0x93, 0x00, 0xa7, 0x00, 0xba
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1,
				0x00, 0xca, 0x01, 0x03, 0x01, 0x2b, 0x01, 0x6c,
				0x01, 0x9a, 0x01, 0xe5, 0x02, 0x1c, 0x02, 0x1e
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2,
				0x02, 0x56, 0x02, 0x93, 0x02, 0xc2, 0x02, 0xfb,
				0x03, 0x25, 0x03, 0x56, 0x03, 0x68, 0x03, 0x79
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3,
				0x03, 0x8d, 0x03, 0xa0, 0x03, 0xb4, 0x03, 0xc4,
				0x03, 0xcd, 0x03, 0xce
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4,
				0x00, 0x00, 0x00, 0x1a, 0x00, 0x41, 0x00, 0x61,
				0x00, 0x7c, 0x00, 0x93, 0x00, 0xa7, 0x00, 0xba
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5,
				0x00, 0xca, 0x01, 0x03, 0x01, 0x2b, 0x01, 0x6c,
				0x01, 0x9a, 0x01, 0xe5, 0x02, 0x1c, 0x02, 0x1e
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x02, 0x56, 0x02, 0x93, 0x02, 0xc2, 0x02, 0xfb,
				0x03, 0x25, 0x03, 0x56, 0x03, 0x68, 0x03, 0x79
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7,
				0x03, 0x8d, 0x03, 0xa0, 0x03, 0xb4, 0x03, 0xc4,
				0x03, 0xcd, 0x03, 0xce
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8,
				0x00, 0x00, 0x00, 0x1a, 0x00, 0x41, 0x00, 0x61,
				0x00, 0x7c, 0x00, 0x93, 0x00, 0xa7, 0x00, 0xba
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9,
				0x00, 0xca, 0x01, 0x03, 0x01, 0x2b, 0x01, 0x6c,
				0x01, 0x9a, 0x01, 0xe5, 0x02, 0x1c, 0x02, 0x1e
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xba,
				0x02, 0x56, 0x02, 0x93, 0x02, 0xc2, 0x02, 0xfb,
				0x03, 0x25, 0x03, 0x56, 0x03, 0x68, 0x03, 0x79
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb,
				0x03, 0x8d, 0x03, 0xa0, 0x03, 0xb4, 0x03, 0xc4,
				0x03, 0xcd, 0x03, 0xce
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x24);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x96);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x63, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0xca);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x29);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x37, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3d, 0x81);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x5b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x5b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x68);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4c, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf7, 0x44);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x21);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4e, 0x43);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4f, 0x65);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x50, 0x87);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x78);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x52, 0x56);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x34);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x54, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x85, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x56, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x59, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5a, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5b, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x00, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x98, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd7, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd8, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd9, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xda, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdb, 0x72);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdc, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdd, 0x55);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xde, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xdf, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe0, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe1, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe2, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe3, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe4, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe5, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe6, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe7, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe9, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xea, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xeb, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xee, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xef, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6,
				0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x05, 0x05, 0x00, 0x00
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5d, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5f, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x60, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x61, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x62, 0x98);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6b, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6c, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6d, 0x0d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6e, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6f, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x79, 0xda);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x7b, 0x68);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x86, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x87, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x88, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc0, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x26);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0xa1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x30);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x09);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x6d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x93);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2f, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x30, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x33, 0x78);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x34, 0x16);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x39, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0xd4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x40, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x41, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x42, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x43, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x44, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x45, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x47, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x48, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x49, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4a, 0xc8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8b, 0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8c, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8d, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8f, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x91, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x92, 0x50);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x93, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x94, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x95, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x96, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0xa0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x98, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x99, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9b, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9c, 0xdc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0x0f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9e, 0x9c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9f, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa0, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc8, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc9, 0x12);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xca, 0x4e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xcb, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa9, 0x79);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaa, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xab, 0x85);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xac, 0x8b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xad, 0x91);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xae, 0x97);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xaf, 0x9d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0xa3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb1, 0xa9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0xaf);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb3, 0xb5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb4, 0xbb);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb5, 0xc1);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb6, 0xc7);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb7, 0xcd);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb8, 0xd3);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0xd9);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x27);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x58, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x78, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x2a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x64, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x67, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x6a, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x82, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x85, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x88, 0x86);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x89, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x97, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x98, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x99, 0x95);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9a, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9c, 0x0b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0x0a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9e, 0x90);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa2, 0x3f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa3, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa4, 0x3c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xa5, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0xd8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0xf8);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2a, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2b, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xe8, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xd0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x7f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0xee);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xf0);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3a, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x40);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb9, 0x02);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x25);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x11);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbb, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3b,
				0x03, 0x08, 0x1a, 0x04, 0x04
	);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x36, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x68, 0x02, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29);
	mipi_dsi_msleep(&dsi_ctx, 30);

	return dsi_ctx.accum_err;
}


static const struct drm_display_mode elish_boe_modes[] = {
	{
		.clock = (1600 + 60 + 8 + 60) * (2560 + 26 + 4 + 168) * 120 / 1000,
		.hdisplay = 1600,
		.hsync_start = 1600 + 60,
		.hsync_end = 1600 + 60 + 8,
		.htotal = 1600 + 60 + 8 + 60,
		.vdisplay = 2560,
		.vsync_start = 2560 + 26,
		.vsync_end = 2560 + 26 + 4,
		.vtotal = 2560 + 26 + 4 + 168,
	},
};

static const struct drm_display_mode elish_csot_modes[] = {
	{
		.clock = (1600 + 200 + 40 + 52) * (2560 + 26 + 4 + 168) * 120 / 1000,
		.hdisplay = 1600,
		.hsync_start = 1600 + 200,
		.hsync_end = 1600 + 200 + 40,
		.htotal = 1600 + 200 + 40 + 52,
		.vdisplay = 2560,
		.vsync_start = 2560 + 26,
		.vsync_end = 2560 + 26 + 4,
		.vtotal = 2560 + 26 + 4 + 168,
	},
};

static const struct drm_display_mode j606f_boe_modes[] = {
	{
		.clock = (1200 + 58 + 2 + 60) * (2000 + 26 + 2 + 93) * 60 / 1000,
		.hdisplay = 1200,
		.hsync_start = 1200 + 58,
		.hsync_end = 1200 + 58 + 2,
		.htotal = 1200 + 58 + 2 + 60,
		.vdisplay = 2000,
		.vsync_start = 2000 + 26,
		.vsync_end = 2000 + 26 + 2,
		.vtotal = 2000 + 26 + 2 + 93,
		.width_mm = 143,
		.height_mm = 235,
	},
};

static const struct panel_desc elish_boe_desc = {
	.modes = elish_boe_modes,
	.num_modes = ARRAY_SIZE(elish_boe_modes),
	.dsi_info = {
		.type = "BOE-elish",
		.channel = 0,
		.node = NULL,
	},
	.width_mm = 127,
	.height_mm = 203,
	.bpc = 8,
	.lanes = 3,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM,
	.init_sequence = elish_boe_init_sequence,
	.is_dual_dsi = true,
};

static const struct panel_desc elish_csot_desc = {
	.modes = elish_csot_modes,
	.num_modes = ARRAY_SIZE(elish_csot_modes),
	.dsi_info = {
		.type = "CSOT-elish",
		.channel = 0,
		.node = NULL,
	},
	.width_mm = 127,
	.height_mm = 203,
	.bpc = 8,
	.lanes = 3,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM,
	.init_sequence = elish_csot_init_sequence,
	.is_dual_dsi = true,
};

static const struct panel_desc j606f_boe_desc = {
	.modes = j606f_boe_modes,
	.num_modes = ARRAY_SIZE(j606f_boe_modes),
	.width_mm = 143,
	.height_mm = 235,
	.bpc = 8,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM,
	.init_sequence = j606f_boe_init_sequence,
	.has_dcs_backlight = true,
};

static void nt36523_reset(struct panel_info *pinfo)
{
	/* LOS seq <1 5 0 5 1>: release->assert->release, ends released.
	 * reset-gpios is active-high, so plain values: HIGH LOW HIGH. */
	gpiod_set_value_cansleep(pinfo->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(pinfo->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(pinfo->reset_gpio, 1);
	usleep_range(12000, 13000);
}

static int nt36523_prepare(struct drm_panel *panel)
{
	struct panel_info *pinfo = to_panel_info(panel);
	u8 buf[6];
	int len = 0;
	int ret;

	ret = regulator_enable(pinfo->vddio);
	if (ret) {
		dev_err(panel->dev, "failed to enable vddio regulator: %d\n", ret);
		return ret;
	}

	if (pinfo->enable_gpio)
		gpiod_set_value_cansleep(pinfo->enable_gpio, 1);

	/*
	 * Variant-safe power-up: check the panel ID (DSI regs 0x0a + 0xab)
	 * before running the init sequence. On gta4l the same NT36523 family
	 * ships in several hardware variants (lide-hsd / hlt-auo / lce-panda /
	 * txd-inx); 0x0a alone is ambiguous (0x9c is shared), so 0xab is read
	 * too. Initializing the wrong variant can push wrong voltages into the
	 * panel; if the ID does not match this desc, power down instead.
	 */
	if (pinfo->desc->panel_id_check) {
		ret = mipi_dsi_dcs_read(pinfo->dsi[0], 0x0a, buf, 1);
		if (ret == 1)
			len = 1;

		ret = mipi_dsi_dcs_read(pinfo->dsi[0], 0xab, buf + len, sizeof(buf) - len);
		if (ret > 0)
			len += ret;

		if (len > 0 && !pinfo->desc->panel_id_check(buf, len)) {
			dev_err(panel->dev, "panel ID does not match this variant, refusing power\n");
			goto unpower;
		}
	}

	nt36523_reset(pinfo);

	ret = pinfo->desc->init_sequence(pinfo);
	if (ret < 0) {
		dev_err(panel->dev, "failed to initialize panel: %d\n", ret);
		goto unpower;
	}

	return 0;

unpower:
	if (pinfo->enable_gpio)
		gpiod_set_value_cansleep(pinfo->enable_gpio, 0);
	regulator_disable(pinfo->vddio);
	return -ENODEV;
}

static int nt36523_disable(struct drm_panel *panel)
{
	struct panel_info *pinfo = to_panel_info(panel);
	int i;

	for (i = 0; i < DSI_NUM_MIN + pinfo->desc->is_dual_dsi; i++) {
		struct mipi_dsi_multi_context dsi_ctx = { .dsi = pinfo->dsi[i]};

		mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	}

	for (i = 0; i < DSI_NUM_MIN + pinfo->desc->is_dual_dsi; i++) {
		struct mipi_dsi_multi_context dsi_ctx = { .dsi = pinfo->dsi[i]};

		mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	}

	msleep(70);

	return 0;
}

static int nt36523_unprepare(struct drm_panel *panel)
{
	struct panel_info *pinfo = to_panel_info(panel);

	gpiod_set_value_cansleep(pinfo->reset_gpio, 1);
	if (pinfo->enable_gpio)
		gpiod_set_value_cansleep(pinfo->enable_gpio, 0);
	regulator_disable(pinfo->vddio);

	return 0;
}

static void nt36523_remove(struct mipi_dsi_device *dsi)
{
	struct panel_info *pinfo = mipi_dsi_get_drvdata(dsi);

	drm_panel_remove(&pinfo->panel);
}

static int nt36523_get_modes(struct drm_panel *panel,
			       struct drm_connector *connector)
{
	struct panel_info *pinfo = to_panel_info(panel);
	int i;

	for (i = 0; i < pinfo->desc->num_modes; i++) {
		const struct drm_display_mode *m = &pinfo->desc->modes[i];
		struct drm_display_mode *mode;

		mode = drm_mode_duplicate(connector->dev, m);
		if (!mode) {
			dev_err(panel->dev, "failed to add mode %ux%u@%u\n",
				m->hdisplay, m->vdisplay, drm_mode_vrefresh(m));
			return -ENOMEM;
		}

		mode->type = DRM_MODE_TYPE_DRIVER;
		if (i == 0)
			mode->type |= DRM_MODE_TYPE_PREFERRED;

		drm_mode_set_name(mode);
		drm_mode_probed_add(connector, mode);
	}

	connector->display_info.width_mm = pinfo->desc->width_mm;
	connector->display_info.height_mm = pinfo->desc->height_mm;
	connector->display_info.bpc = pinfo->desc->bpc;

	return pinfo->desc->num_modes;
}

static enum drm_panel_orientation nt36523_get_orientation(struct drm_panel *panel)
{
	struct panel_info *pinfo = to_panel_info(panel);

	return pinfo->orientation;
}

static const struct drm_panel_funcs nt36523_panel_funcs = {
	.disable = nt36523_disable,
	.prepare = nt36523_prepare,
	.unprepare = nt36523_unprepare,
	.get_modes = nt36523_get_modes,
	.get_orientation = nt36523_get_orientation,
};

static int nt36523_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	/* gta4l lide-hsd: qcom,mdss-dsi-bl-inverted-dbv — invert */
	brightness = 4095 - brightness;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

static int nt36523_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_get_display_brightness_large(dsi, &brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return brightness;
}

static const struct backlight_ops nt36523_bl_ops = {
	.update_status = nt36523_bl_update_status,
	.get_brightness = nt36523_bl_get_brightness,
};

static struct backlight_device *nt36523_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 3964,	/* 4095-131: downstream bl-default 0x83, inverted */
		.max_brightness = 4095,
		.scale = BACKLIGHT_SCALE_NON_LINEAR,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &nt36523_bl_ops, &props);
}

static int nt36523_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct device_node *dsi1;
	struct mipi_dsi_host *dsi1_host;
	struct panel_info *pinfo;
	const struct mipi_dsi_device_info *info;
	int i, ret;

	pinfo = devm_drm_panel_alloc(dev, struct panel_info, panel,
				     &nt36523_panel_funcs,
				     DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(pinfo))
		return PTR_ERR(pinfo);

	pinfo->vddio = devm_regulator_get(dev, "vddio");
	if (IS_ERR(pinfo->vddio))
		return dev_err_probe(dev, PTR_ERR(pinfo->vddio), "failed to get vddio regulator\n");

	pinfo->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(pinfo->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(pinfo->reset_gpio), "failed to get reset gpio\n");

	pinfo->enable_gpio = devm_gpiod_get_optional(dev, "enable", GPIOD_OUT_LOW);
	if (IS_ERR(pinfo->enable_gpio))
		return dev_err_probe(dev, PTR_ERR(pinfo->enable_gpio), "failed to get enable gpio\n");

	pinfo->desc = of_device_get_match_data(dev);
	if (!pinfo->desc)
		return -ENODEV;

	/* If the panel is dual dsi, register DSI1 */
	if (pinfo->desc->is_dual_dsi) {
		info = &pinfo->desc->dsi_info;

		dsi1 = of_graph_get_remote_node(dsi->dev.of_node, 1, -1);
		if (!dsi1) {
			dev_err(dev, "cannot get secondary DSI node.\n");
			return -ENODEV;
		}

		dsi1_host = of_find_mipi_dsi_host_by_node(dsi1);
		of_node_put(dsi1);
		if (!dsi1_host)
			return dev_err_probe(dev, -EPROBE_DEFER, "cannot get secondary DSI host\n");

		pinfo->dsi[1] = devm_mipi_dsi_device_register_full(dev, dsi1_host, info);
		if (IS_ERR(pinfo->dsi[1])) {
			dev_err(dev, "cannot get secondary DSI device\n");
			return PTR_ERR(pinfo->dsi[1]);
		}
	}

	pinfo->dsi[0] = dsi;
	mipi_dsi_set_drvdata(dsi, pinfo);

	ret = of_drm_get_panel_orientation(dev->of_node, &pinfo->orientation);
	if (ret < 0) {
		dev_err(dev, "%pOF: failed to get orientation %d\n", dev->of_node, ret);
		return ret;
	}

	pinfo->panel.prepare_prev_first = true;

	if (pinfo->desc->has_dcs_backlight) {
		pinfo->panel.backlight = nt36523_create_backlight(dsi);
		if (IS_ERR(pinfo->panel.backlight))
			return dev_err_probe(dev, PTR_ERR(pinfo->panel.backlight),
					     "Failed to create backlight\n");
	} else {
		ret = drm_panel_of_backlight(&pinfo->panel);
		if (ret)
			return dev_err_probe(dev, ret, "Failed to get backlight\n");
	}

	drm_panel_add(&pinfo->panel);

	for (i = 0; i < DSI_NUM_MIN + pinfo->desc->is_dual_dsi; i++) {
		pinfo->dsi[i]->lanes = pinfo->desc->lanes;
		pinfo->dsi[i]->format = pinfo->desc->format;
		pinfo->dsi[i]->mode_flags = pinfo->desc->mode_flags;

		ret = devm_mipi_dsi_attach(dev, pinfo->dsi[i]);
		if (ret < 0)
			return dev_err_probe(dev, ret, "cannot attach to DSI%d host.\n", i);
	}

	return 0;
}

static const struct drm_display_mode gta4l_lide_hsd_mode = {
	/* 1200x2000@60, nt36523 lide-hsd, from LOS P85946 timing@0
	 * hfp=0x34 hbp=0x33 hpw=1, vbp=7 vfp=0x1a vpw=1 */
	.clock = (1200 + 52 + 1 + 51) * (2000 + 26 + 1 + 7) * 60 / 1000,
	.hdisplay = 1200,
	.hsync_start = 1200 + 52,
	.hsync_end = 1200 + 52 + 1,
	.htotal = 1200 + 52 + 1 + 51,
	.vdisplay = 2000,
	.vsync_start = 2000 + 26,
	.vsync_end = 2000 + 26 + 1,
	.vtotal = 2000 + 26 + 1 + 7,
	.width_mm = 135,
	.height_mm = 226,
	.type = DRM_MODE_TYPE_DRIVER,
};

static bool gta4l_nt36523_family_id_check(u8 *buf, int len)
{
	/*
	 * Wrong-family fuse, not a variant discriminator: all four gta4l
	 * NT36523 variants (lide-hsd / hlt-auo / lce-panda / txd-inx) read
	 * 0x9c at status reg 0x0a (vendor ESD tables; 0x9e on live boot) and
	 * 0x00 0x00 at reg 0xab -- the vendor DT provides no byte-level
	 * variant signature. Selection is by DT compatible, exactly like the
	 * vendor dsi-display nodes. The family check only refuses a
	 * different-family panel behind a gta4l compatible (e.g. hx83102e,
	 * which reads 0x80 0x73...), preventing wrong-init overvoltage.
	 */
	return len >= 1 && (buf[0] == 0x9c || buf[0] == 0x9e);
}

static const struct panel_desc gta4l_lide_hsd_desc = {
	.width_mm = 135,
	.height_mm = 226,
	.bpc = 8,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM,
	.modes = &gta4l_lide_hsd_mode,
	.num_modes = 1,
	.init_sequence = gta4l_lide_hsd_init_sequence,
	.panel_id_check = gta4l_nt36523_family_id_check,
	.has_dcs_backlight = true,
};

static const struct drm_display_mode gta4l_hlt_auo_mode = {
	/* 1200x2000@60, nt36523 hlt-auo, ov13 timing@0: same geometry as lide-hsd */
	.clock = (1200 + 52 + 1 + 51) * (2000 + 26 + 1 + 7) * 60 / 1000,
	.hdisplay = 1200,
	.hsync_start = 1200 + 52,
	.hsync_end = 1200 + 52 + 1,
	.htotal = 1200 + 52 + 1 + 51,
	.vdisplay = 2000,
	.vsync_start = 2000 + 26,
	.vsync_end = 2000 + 26 + 1,
	.vtotal = 2000 + 26 + 1 + 7,
	.width_mm = 135,
	.height_mm = 226,
	.type = DRM_MODE_TYPE_DRIVER,
};

static const struct drm_display_mode gta4l_lce_panda_mode = {
	/* 1200x2000@60, nt36523 lce-panda, ov13 timing@0: same geometry as lide-hsd */
	.clock = (1200 + 52 + 1 + 51) * (2000 + 26 + 1 + 7) * 60 / 1000,
	.hdisplay = 1200,
	.hsync_start = 1200 + 52,
	.hsync_end = 1200 + 52 + 1,
	.htotal = 1200 + 52 + 1 + 51,
	.vdisplay = 2000,
	.vsync_start = 2000 + 26,
	.vsync_end = 2000 + 26 + 1,
	.vtotal = 2000 + 26 + 1 + 7,
	.width_mm = 135,
	.height_mm = 226,
	.type = DRM_MODE_TYPE_DRIVER,
};

static const struct panel_desc gta4l_hlt_auo_desc = {
	.width_mm = 135,
	.height_mm = 226,
	.bpc = 8,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM,
	.modes = &gta4l_hlt_auo_mode,
	.num_modes = 1,
	.init_sequence = gta4l_hlt_auo_init_sequence,
	.panel_id_check = gta4l_nt36523_family_id_check,
	.has_dcs_backlight = true,
};

static const struct panel_desc gta4l_txd_inx_desc = {
	.width_mm = 135,
	.height_mm = 226,
	.bpc = 8,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM,
	.modes = &gta4l_lide_hsd_mode,
	.num_modes = 1,
	.init_sequence = gta4l_txd_inx_init_sequence,
	.panel_id_check = gta4l_nt36523_family_id_check,
	.has_dcs_backlight = true,
};

static const struct panel_desc gta4l_lce_panda_desc = {
	.width_mm = 135,
	.height_mm = 226,
	.bpc = 8,
	.lanes = 4,
	.format = MIPI_DSI_FMT_RGB888,
	.mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_LPM,
	.modes = &gta4l_lce_panda_mode,
	.num_modes = 1,
	.init_sequence = gta4l_lce_panda_init_sequence,
	.panel_id_check = gta4l_nt36523_family_id_check,
	.has_dcs_backlight = true,
};

static const struct of_device_id nt36523_of_match[] = {
	{
		.compatible = "samsung,gta4l-nt36523-lide-hsd",
		.data = &gta4l_lide_hsd_desc,
	},	{
		.compatible = "samsung,gta4l-nt36523-hlt-auo",
		.data = &gta4l_hlt_auo_desc,
	},	{
		.compatible = "samsung,gta4l-nt36523-txd-inx",
		.data = &gta4l_txd_inx_desc,
	},	{
		.compatible = "samsung,gta4l-nt36523-lce-panda",
		.data = &gta4l_lce_panda_desc,
	},	{
		.compatible = "lenovo,j606f-boe-nt36523w",
		.data = &j606f_boe_desc,
	},
	{
		.compatible = "xiaomi,elish-boe-nt36523",
		.data = &elish_boe_desc,
	},
	{
		.compatible = "xiaomi,elish-csot-nt36523",
		.data = &elish_csot_desc,
	},
	{},
};
MODULE_DEVICE_TABLE(of, nt36523_of_match);

static struct mipi_dsi_driver nt36523_driver = {
	.probe = nt36523_probe,
	.remove = nt36523_remove,
	.driver = {
		.name = "panel-novatek-nt36523",
		.of_match_table = nt36523_of_match,
	},
};
module_mipi_dsi_driver(nt36523_driver);

MODULE_AUTHOR("Jianhua Lu <lujianhua000@gmail.com>");
MODULE_DESCRIPTION("DRM driver for Novatek NT36523 based MIPI DSI panels");
MODULE_LICENSE("GPL");
