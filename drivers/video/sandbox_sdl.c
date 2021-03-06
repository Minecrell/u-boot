// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <video.h>
#include <asm/global_data.h>
#include <asm/sdl.h>
#include <asm/state.h>
#include <asm/u-boot-sandbox.h>
#include <dm/test.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	/* Default LCD size we support */
	LCD_MAX_WIDTH		= 1366,
	LCD_MAX_HEIGHT		= 768,
};

static int sandbox_sdl_probe(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct sandbox_sdl_plat *plat = dev_get_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct sandbox_state *state = state_get_current();
	int ret;

	ret = sandbox_sdl_init_display(plat->xres, plat->yres, plat->bpix,
				       state->double_lcd);
	if (ret) {
		puts("LCD init failed\n");
		return ret;
	}
	uc_priv->xsize = plat->xres;
	uc_priv->ysize = plat->yres;
	uc_priv->bpix = plat->bpix;
	uc_priv->rot = plat->rot;
	uc_priv->vidconsole_drv_name = plat->vidconsole_drv_name;
	uc_priv->font_size = plat->font_size;
	if (IS_ENABLED(CONFIG_VIDEO_COPY))
		uc_plat->copy_base = uc_plat->base - uc_plat->size / 2;

	return 0;
}

static int sandbox_sdl_bind(struct udevice *dev)
{
	struct video_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct sandbox_sdl_plat *plat = dev_get_plat(dev);
	int ret = 0;

	plat->xres = dev_read_u32_default(dev, "xres", LCD_MAX_WIDTH);
	plat->yres = dev_read_u32_default(dev, "yres", LCD_MAX_HEIGHT);
	plat->bpix = dev_read_u32_default(dev, "log2-depth", VIDEO_BPP16);
	plat->rot = dev_read_u32_default(dev, "rotate", 0);
	uc_plat->size = plat->xres * plat->yres * (1 << plat->bpix) / 8;

	/* Allow space for two buffers, the lower one being the copy buffer */
	log_debug("Frame buffer size %x\n", uc_plat->size);
	if (IS_ENABLED(CONFIG_VIDEO_COPY))
		uc_plat->size *= 2;

	return ret;
}

static const struct udevice_id sandbox_sdl_ids[] = {
	{ .compatible = "sandbox,lcd-sdl" },
	{ }
};

U_BOOT_DRIVER(sandbox_lcd_sdl) = {
	.name	= "sandbox_lcd_sdl",
	.id	= UCLASS_VIDEO,
	.of_match = sandbox_sdl_ids,
	.bind	= sandbox_sdl_bind,
	.probe	= sandbox_sdl_probe,
	.plat_auto	= sizeof(struct sandbox_sdl_plat),
};
