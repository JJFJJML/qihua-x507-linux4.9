/*
 * Allwinner Sun50iw3 clock register definitions
 *
 * (C) Copyright 2017  <weidonghui@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <sunxi_board.h>
#include <fdt_support.h>
#include <sys_config.h>
#include <sunxi_power/axp.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SUNXI_OVERLAY
int sunxi_overlay_apply_merged(void *dtb_base, void *dtbo_base)
{
	int nodeoffset = 0;
	//char axp_name[16] = {0};
	//u8 axp_chipid = 0;
	char axp_path[] = "pmu0";
	//char *node_compatible;

	nodeoffset = fdt_path_offset(dtb_base, axp_path);
	if (nodeoffset < 0) {
		pr_err("error: %s : %s\n", __func__, fdt_strerror(nodeoffset));
		return -1;
	}
	/*
	fdt_getprop_string(dtb_base, nodeoffset, "compatible", &node_compatible);
	pmu_get_info(axp_name, &axp_chipid);
	if (strstr(node_compatible, axp_name) == NULL) {
		return fdt_check_header(dtbo_base);
	}
	*/
	return 0;

}
#endif



