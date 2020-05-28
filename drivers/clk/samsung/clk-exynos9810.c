/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Common Clock Framework support for Exynos9810 SoC.
 */

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <soc/samsung/cal-if.h>
#include <dt-bindings/clock/exynos9810.h>

#include "../../soc/samsung/cal-if/exynos9810/cmucal-vclk.h"
#include "../../soc/samsung/cal-if/exynos9810/cmucal-node.h"
#include "../../soc/samsung/cal-if/exynos9810/cmucal-qch.h"
#include "../../soc/samsung/cal-if/exynos9810/clkout_exynos9810.h"
#include "composite.h"

static struct samsung_clk_provider *exynos9810_clk_provider;
/*
 * list of controller registers to be saved and restored during a
 * suspend/resume cycle.
 */
/* fixed rate clocks generated outside the soc */
struct samsung_fixed_rate exynos9810_fixed_rate_ext_clks[] __initdata = {
	FRATE(OSCCLK, "fin_pll", NULL, 0, 26000000),
};

/* HWACG VCLK */
struct init_vclk exynos9810_apm_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_DLL, MUX_DLL_USER, "UMUX_DLL", NULL, 0, VCLK_GATE, NULL),

	HWACG_VCLK(GATE_GREBE, GREBEINTEGRATION_QCH_GREBE, "GATE_GREBE", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_GREBE_DBG, GREBEINTEGRATION_QCH_DBG, "GATE_GREBE_DBG", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_INTMEM, INTMEM_QCH, "GATE_INTMEM", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_AP2CHUB, MAILBOX_AP2CHUB_QCH, "GATE_MAILBOX_AP2CHUB", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_AP2CP, MAILBOX_AP2CP_QCH, "GATE_MAILBOX_AP2CP", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_AP2CP_S, MAILBOX_AP2CP_S_QCH, "GATE_MAILBOX_AP2CP_S", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_AP2GNSS, MAILBOX_AP2GNSS_QCH, "GATE_MAILBOX_AP2GNSS", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_AP2VTS, MAILBOX_AP2VTS_QCH, "GATE_MAILBOX_AP2VTS", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_APM2AP, MAILBOX_APM2AP_QCH, "GATE_MAILBOX_APM2AP", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_APM2CHUB, MAILBOX_APM2CHUB_QCH, "GATE_MAILBOX_APM2CHUB", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_APM2CP, MAILBOX_APM2CP_QCH, "GATE_MAILBOX_APM2CP", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_APM2GNSS, MAILBOX_APM2GNSS_QCH, "GATE_MAILBOX_APM2GNSS", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_CHUB2CP, MAILBOX_CHUB2CP_QCH, "GATE_MAILBOX_CHUB2CP", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_GNSS2CHUB, MAILBOX_GNSS2CHUB_QCH, "GATE_MAILBOX_GNSS2CHUB", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_GNSS2CP, MAILBOX_GNSS2CP_QCH, "GATE_MAILBOX_GNSS2CP", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PEM, PEM_QCH, "GATE_PEM", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SPEEDY_APM, SPEEDY_APM_QCH, "GATE_SPEEDY_APM", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SPEEDY_SUB_APM, SPEEDY_SUB_APM_QCH, "GATE_SPEEDY_SUB_APM", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT_APM, WDT_APM_QCH, "GATE_WDT_APM", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_abox_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_ABOX_ACLK, ABOX_QCH_ACLK, "GATE_ABOX_ACLK", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_BCLK_DSIF, ABOX_QCH_BCLK_DSIF, "GATE_ABOX_BCLK_DSIF", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_BCLK0, ABOX_QCH_BCLK0, "GATE_ABOX_BCLK0", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_BCLK1, ABOX_QCH_BCLK1, "GATE_ABOX_BCLK1", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_BCLK2, ABOX_QCH_BCLK2, "GATE_ABOX_BCLK2", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_BCLK3, ABOX_QCH_BCLK3, "GATE_ABOX_BCLK3", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_DUMMY, ABOX_QCH_DUMMY, "GATE_ABOX_DUMMY", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ABOX_DMIC, DMIC_QCH, "GATE_ABOX_DMIC", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_ABOX, SYSMMU_AUD_QCH, "GATE_SYSMMU_AUD", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT_ABOXCPU, WDT_AUD_QCH, "GATE_WDT_AUD", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_bus1_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_TREX_P_BUS1, TREX_P_BUS1_QCH, "GATE_TREX_P_BUS1", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_busc_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_PDMA0, PDMA0_QCH, "GATE_PDMA0", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PPFW, PPFW_QCH, "GATE_PPFW", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SBIC, SBIC_QCH, "GATE_SBIC", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SIREX, SIREX_QCH, "GATE_SIREX", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SPDMA, SPDMA_QCH, "GATE_SPDMA", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_TREX_D_BUSC, TREX_D_BUSC_QCH, "GATE_TREX_D_BUSC", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_TREX_P_BUSC, TREX_P_BUSC_QCH, "GATE_TREX_P_BUSC", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_TREX_RB_BUSC, TREX_RB_BUSC_QCH, "GATE_TREX_RB_BUSC", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_chub_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_CM4_CHUB, CM4_CHUB_QCH, "GATE_CM4_CHUB", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CHUB00, I2C_CHUB00_QCH, "GATE_I2C_CHUB00", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CHUB01, I2C_CHUB01_QCH, "GATE_I2C_CHUB01", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PDMA_CHUB, PDMA_CHUB_QCH, "GATE_PDMA_CHUB", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PWM_CHUB, PWM_CHUB_QCH, "GATE_PWM_CHUB", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_TIMER_CHUB, TIMER_CHUB_QCH, "GATE_TIMER_CHUB", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CHUB00, USI_CHUB00_QCH, "GATE_USI_CHUB00", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CHUB01, USI_CHUB01_QCH, "GATE_USI_CHUB01", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT_CHUB, WDT_CHUB_QCH, "GATE_WDT_CHUB", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_cmgp_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_ADC_CMGP_S0, ADC_CMGP_QCH_S0, "GATE_ADC_CMGP_S0", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ADC_CMGP_S1, ADC_CMGP_QCH_S1, "GATE_ADC_CMGP_S1", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_ADC_CMGP, ADC_CMGP_QCH_ADC, "GATE_ADC_CMGP_ADC", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP00, I2C_CMGP00_QCH, "GATE_I2C_CMGP00", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP01, I2C_CMGP01_QCH, "GATE_I2C_CMGP01", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP02, I2C_CMGP02_QCH, "GATE_I2C_CMGP02", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CMGP03, I2C_CMGP03_QCH, "GATE_I2C_CMGP03", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP00, USI_CMGP00_QCH, "GATE_USI_CMGP00", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP01, USI_CMGP01_QCH, "GATE_USI_CMGP01", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP02, USI_CMGP02_QCH, "GATE_USI_CMGP02", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI_CMGP03, USI_CMGP03_QCH, "GATE_USI_CMGP03", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_cmu_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_CMU_CMUREF, CMU_CMU_CMUREF_QCH, "GATE_CMU_CMUREF", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DFTMUX_TOP_CIS_CLK0, DFTMUX_TOP_QCH_CIS_CLK0, "GATE_CIS_CLK0", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DFTMUX_TOP_CIS_CLK1, DFTMUX_TOP_QCH_CIS_CLK1, "GATE_CIS_CLK1", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DFTMUX_TOP_CIS_CLK2, DFTMUX_TOP_QCH_CIS_CLK2, "GATE_CIS_CLK2", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DFTMUX_TOP_CIS_CLK3, DFTMUX_TOP_QCH_CIS_CLK3, "GATE_CIS_CLK3", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_OTP, OTP_QCH, "GATE_OTP", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_core_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_TREX_D_CORE, TREX_D_CORE_QCH, "GATE_TREX_D_CORE", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_TREX_P0_CORE, TREX_P0_CORE_QCH, "GATE_TREX_P0_CORE", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_TREX_P1_CORE, TREX_P1_CORE_QCH, "GATE_TREX_P1_CORE", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_dcf_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_DCF_BUS, MUX_CLKCMU_DCF_BUS_USER, "UMUX_CLKCMU_DCF_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_IS_DCF_CIP, IS_DCF_QCH_CIP, "GATE_IS_DCF_CIP", "UMUX_CLKCMU_DCF_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_DCF_C2SYNC_2SLV, IS_DCF_QCH_C2SYNC_2SLV, "GATE_IS_DCF_C2SYNC_2SLV", "UMUX_CLKCMU_DCF_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_DCF_SYSMMU, IS_DCF_QCH_SYSMMU, "GATE_IS_DCF_SYSMMU", "UMUX_CLKCMU_DCF_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_dcpost_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_DCPOST_BUS, MUX_CLKCMU_DCPOST_BUS_USER, "UMUX_CLKCMU_DCPOST_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_IS_DCPOST_CIP2, IS_DCPOST_QCH_CIP2, "GATE_IS_DCPOST_CIP2", "UMUX_CLKCMU_DCPOST_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_DCPOST_C2SYNC_1SLV_CLK, IS_DCPOST_QCH_C2SYNC_1SLV_CLK, "GATE_IS_DCPOST_C2SYNC_1SLV_CLK", "UMUX_CLKCMU_DCPOST_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_dcrd_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_DCRD_BUS, MUX_CLKCMU_DCRD_BUS_USER, "UMUX_CLKCMU_DCRD_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_IS_DCRD_DCP, IS_DCRD_QCH_DCP, "GATE_IS_DCRD_DCP", "UMUX_CLKCMU_DCRD_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_DCRD_SYSMMU, IS_DCRD_QCH_SYSMMU, "GATE_IS_DCRD_SYSMMU", "UMUX_CLKCMU_DCRD_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_DCRD_DCP_C2C, IS_DCRD_QCH_DCP_C2C, "GATE_IS_DCRD_DCP_C2C", "UMUX_CLKCMU_DCRD_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_DCRD_DCP_DIV2, IS_DCRD_QCH_DCP_DIV2, "GATE_IS_DCRD_DCP_DIV2", "UMUX_CLKCMU_DCRD_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_dpu_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_DPU_BUS, MUX_CLKCMU_DPU_BUS_USER, "UMUX_CLKCMU_DPU_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_DPU, DPU_QCH_DPU, "GATE_DPU_DPU", "UMUX_CLKCMU_DPU_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DPU_DMA, DPU_QCH_DPU_DMA, "GATE_DPU_DMA", "UMUX_CLKCMU_DPU_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DPU_DPP, DPU_QCH_DPU_DPP, "GATE_DPU_DPP", "UMUX_CLKCMU_DPU_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DPU_WB_MUX, DPU_QCH_DPU_WB_MUX, "GATE_DPU_WB_MUX", "UMUX_CLKCMU_DPU_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SYSMMU_DPUD0, SYSMMU_DPUD0_QCH, "GATE_SYSMMU_DPUD0", "UMUX_CLKCMU_DPU_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SYSMMU_DPUD1, SYSMMU_DPUD1_QCH, "GATE_SYSMMU_DPUD1", "UMUX_CLKCMU_DPU_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SYSMMU_DPUD2, SYSMMU_DPUD2_QCH, "GATE_SYSMMU_DPUD2", "UMUX_CLKCMU_DPU_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_dspm_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_DSPM_BUS, MUX_CLKCMU_DSPM_BUS_USER, "UMUX_CLKCMU_DSPM_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_SCORE_MASTER, SCORE_MASTER_QCH, "GATE_SCORE_MASTER", "UMUX_CLKCMU_DSPM_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_DSPM0, SYSMMU_DSPM0_QCH, "GATE_SMMU_DSPM0", "UMUX_CLKCMU_DSPM_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_DSPM1, SYSMMU_DSPM1_QCH, "GATE_SMMU_DSPM1", "UMUX_CLKCMU_DSPM_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_dsps_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_DSPS_BUS, MUX_CLKCMU_DSPS_BUS_USER, "UMUX_CLKCMU_DSPS_BUS", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_DSPS_AUD, MUX_CLKCMU_DSPS_AUD_USER, "UMUX_CLKCMU_DSPS_AUD", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_SCORE_KNIGHT, SCORE_KNIGHT_QCH, "GATE_SCORE_KNIGHT", "UMUX_CLKCMU_DSPS_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_fsys0_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_FSYS0_BUS, MUX_CLKCMU_FSYS0_BUS_USER, "UMUX_CLKCMU_FSYS0_BUS", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_FSYS0_UFS_EMBD, MUX_CLKCMU_FSYS0_UFS_EMBD_USER, "UMUX_CLKCMU_FSYS0_UFS_EMBD", "UMUX_CLKCMU_FSYS0_BUS", 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_FSYS0_USB30DRD, MUX_CLKCMU_FSYS0_USB30DRD_USER, "UMUX_CLKCMU_FSYS0_USB30DRD", "UMUX_CLKCMU_FSYS0_BUS", 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_FSYS0_DPGTC, MUX_CLKCMU_FSYS0_DPGTC_USER, "UMUX_CLKCMU_FSYS0_DPGTC", "UMUX_CLKCMU_FSYS0_BUS", 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_FSYS0_USBDP_DEBUG, MUX_CLKCMU_FSYS0_USBDP_DEBUG_USER, "UMUX_CLKCMU_FSYS0_USBDP_DEBUG", "UMUX_CLKCMU_FSYS0_BUS", 0, 0, NULL),

	HWACG_VCLK(GATE_DP_LINK, DP_LINK_QCH, "GATE_DP_LINK", "UMUX_CLKCMU_FSYS0_DPGTC", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_DP_LINK_GTC, DP_LINK_QCH_GTC, "GATE_DP_LINK_GTC", "UMUX_CLKCMU_FSYS0_DPGTC", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_UFS_EMBD, UFS_EMBD_QCH, "GATE_UFS_EMBD", "UMUX_CLKCMU_FSYS0_UFS_EMBD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_UFS_EMBD_FMP, UFS_EMBD_QCH_FMP, "GATE_UFS_EMBD_FMP", "UMUX_CLKCMU_FSYS0_UFS_EMBD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USB30DRD_USB30DRD_LINK, USB30DRD_QCH_USB30DRD_LINK, "GATE_USB30DRD_USB30DRD_LINK", "UMUX_CLKCMU_FSYS0_USB30DRD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USB30DRD_USBPCS, USB30DRD_QCH_USBPCS, "GATE_USB30DRD_USBPCS", "UMUX_CLKCMU_FSYS0_USB30DRD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USB30DRD_USB30DRD_CTRL, USB30DRD_QCH_USB30DRD_CTRL, "GATE_USB30DRD_USB30DRD_CTRL", "UMUX_CLKCMU_FSYS0_USB30DRD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USB30DRD_USBDPPHY, USB30DRD_QCH_USBDPPHY, "GATE_USB30DRD_USBDPPHY", "UMUX_CLKCMU_FSYS0_USB30DRD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USB30DRD_SOC_PLL, USB30DRD_QCH_SOC_PLL, "GATE_USB30DRD_SOC_PLL", "UMUX_CLKCMU_FSYS0_USBDP_DEBUG", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_fsys1_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_FSYS1_BUS, MUX_CLKCMU_FSYS1_BUS_USER, "UMUX_CLKCMU_FSYS1_BUS", NULL, 0, VCLK_GATE, NULL),
	HWACG_VCLK(UMUX_CLKCMU_FSYS1_PCIE, MUX_CLKCMU_FSYS1_PCIE_USER, "UMUX_CLKCMU_FSYS1_PCIE", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_FSYS1_MMC_CARD, MUX_CLKCMU_FSYS1_MMC_CARD_USER, "UMUX_CLKCMU_FSYS1_MMC_CARD", "UMUX_CLKCMU_FSYS1_BUS", 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_FSYS1_UFS_CARD, MUX_CLKCMU_FSYS1_UFS_CARD_USER, "UMUX_CLKCMU_FSYS1_UFS_CARD", "UMUX_CLKCMU_FSYS1_BUS", 0, 0, NULL),

	HWACG_VCLK(GATE_MMC_CARD, MMC_CARD_QCH, "GATE_MMC_CARD", "UMUX_CLKCMU_FSYS1_MMC_CARD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN2_MSTR, PCIE_GEN2_QCH_MSTR, "GATE_PCIE_GEN2_MSTR", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN2_PCS, PCIE_GEN2_QCH_PCS, "GATE_PCIE_GEN2_PCS", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN2_PHY, PCIE_GEN2_QCH_PHY, "GATE_PCIE_GEN2_PHY", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN2_DBI, PCIE_GEN2_QCH_DBI, "GATE_PCIE_GEN2_DBI", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN2_APB, PCIE_GEN2_QCH_APB, "GATE_PCIE_GEN2_APB", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN2_SOCPLL, PCIE_GEN2_QCH_SOCPLL, "GATE_PCIE_GEN2_SOCPLL", "UMUX_CLKCMU_FSYS1_PCIE", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN3_MSTR, PCIE_GEN3_QCH_MSTR, "GATE_PCIE_GEN3_MSTR", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN3_PCS, PCIE_GEN3_QCH_PCS, "GATE_PCIE_GEN3_PCS", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN3_DBI, PCIE_GEN3_QCH_DBI, "GATE_PCIE_GEN3_DBI", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN3_APB, PCIE_GEN3_QCH_APB, "GATE_PCIE_GEN3_APB", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN3_SOCPLL, PCIE_GEN3_QCH_SOCPLL, "GATE_PCIE_GEN3_SOCPLL", "UMUX_CLKCMU_FSYS1_PCIE", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_GEN3_PHY, PCIE_GEN3_QCH_PHY, "GATE_PCIE_GEN3", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_IA_GEN2, PCIE_IA_GEN2_QCH, "GATE_PCIE_IA_GEN2", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_PCIE_IA_GEN3, PCIE_IA_GEN3_QCH, "GATE_PCIE_IA_GEN3", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_RTIC, RTIC_QCH, "GATE_RTIC", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SSS, SSS_QCH, "GATE_SSS", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SYSMMU_FSYS1, SYSMMU_FSYS1_QCH, "GATE_SYSMMU_FSYS1", "UMUX_CLKCMU_FSYS1_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_UFS_CARD, UFS_CARD_QCH, "GATE_UFS_CARD", "UMUX_CLKCMU_FSYS1_UFS_CARD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_UFS_CARD_FMP, UFS_CARD_QCH_FMP, "GATE_UFS_CARD_FMP", "UMUX_CLKCMU_FSYS1_UFS_CARD", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_g2d_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_G2D_MSCL, MUX_CLKCMU_G2D_MSCL_USER, "UMUX_CLKCMU_G2D_MSCL", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_G2D_G2D, MUX_CLKCMU_G2D_G2D_USER, "UMUX_CLKCMU_G2D_G2D", "UMUX_CLKCMU_G2D_MSCL", 0, 0, NULL),

	HWACG_VCLK(GATE_JSQZ, JSQZ_QCH, "GATE_JSQZ", "UMUX_CLKCMU_G2D_MSCL", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_G2D, G2D_QCH, "GATE_G2D", "UMUX_CLKCMU_G2D_G2D", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_JPEG, JPEG_QCH, "GATE_JPEG", "UMUX_CLKCMU_G2D_MSCL", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MSCL, MSCL_QCH, "GATE_MSCL", "UMUX_CLKCMU_G2D_MSCL", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_G2DD0, SYSMMU_G2DD0_QCH, "GATE_SMMU_G2DD0", "UMUX_CLKCMU_G2D_G2D", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_G2DD1, SYSMMU_G2DD1_QCH, "GATE_SMMU_G2DD1", "UMUX_CLKCMU_G2D_G2D", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_G2DD2, SYSMMU_G2DD2_QCH, "GATE_SMMU_G2DD2", "UMUX_CLKCMU_G2D_G2D", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_g3d_hwacg_vclks[] __initdata = {
	HWACG_VCLK(GATE_GPU, GPU_QCH, "GATE_GPU", NULL, 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_isphq_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_ISPHQ_BUS, MUX_CLKCMU_ISPHQ_BUS_USER, "UMUX_CLKCMU_ISPHQ_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_IS_ISPHQ, IS_ISPHQ_QCH_ISPHQ, "GATE_IS_ISPHQ", "UMUX_CLKCMU_ISPHQ_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPHQ_C2COM, IS_ISPHQ_QCH_ISPHQ_C2COM, "GATE_IS_ISPHQ_C2COM", "UMUX_CLKCMU_ISPHQ_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPHQ_SYSMMU, IS_ISPHQ_QCH_SYSMMU_ISPHQ, "GATE_IS_ISPHQ_SYSMMU", "UMUX_CLKCMU_ISPHQ_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_isplp_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_ISPLP_BUS, MUX_CLKCMU_ISPLP_BUS_USER, "UMUX_CLKCMU_ISPLP_BUS", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_ISPLP_VRA, MUX_CLKCMU_ISPLP_VRA_USER, "UMUX_CLKCMU_ISPLP_VRA", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_ISPLP_GDC, MUX_CLKCMU_ISPLP_GDC_USER, "UMUX_CLKCMU_ISPLP_GDC", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_IS_ISPLP_MC_SCALER, IS_ISPLP_QCH_MC_SCALER, "GATE_IS_ISPLP_MC_SCALER", "UMUX_CLKCMU_ISPLP_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPLP, IS_ISPLP_QCH_ISPLP, "GATE_IS_ISPLP_ISPLP", "UMUX_CLKCMU_ISPLP_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPLP_VRA, IS_ISPLP_QCH_VRA, "GATE_IS_ISPLP_VRA", "UMUX_CLKCMU_ISPLP_VRA", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPLP_GDC, IS_ISPLP_QCH_GDC, "GATE_IS_ISPLP_GDC", "UMUX_CLKCMU_ISPLP_GDC", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPLP_C2, IS_ISPLP_QCH_ISPLP_C2, "GATE_IS_ISPLP_ISPLP_C2", "UMUX_CLKCMU_ISPLP_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_SYSMMU_ISPLP0, IS_ISPLP_QCH_SYSMMU_ISPLP0, "GATE_IS_ISPLP_SYSMMU_ISPLP0", "UMUX_CLKCMU_ISPLP_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_SYSMMU_ISPLP1, IS_ISPLP_QCH_SYSMMU_ISPLP1, "GATE_IS_ISPLP_SYSMMU_ISPLP1", "UMUX_CLKCMU_ISPLP_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_isppre_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_ISPPRE_BUS, MUX_CLKCMU_ISPPRE_BUS_USER, "UMUX_CLKCMU_ISPPRE_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_IS_ISPPRE_CSIS0, IS_ISPPRE_QCH_CSIS0, "GATE_IS_ISPPRE_CSIS0", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_CSIS1, IS_ISPPRE_QCH_CSIS1, "GATE_IS_ISPPRE_CSIS1", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_CSIS2, IS_ISPPRE_QCH_CSIS2, "GATE_IS_ISPPRE_CSIS2", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_CSIS3, IS_ISPPRE_QCH_CSIS3, "GATE_IS_ISPPRE_CSIS3", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_PDP_DMA, IS_ISPPRE_QCH_PDP_DMA, "GATE_IS_ISPPRE_PDP_DMA", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_SYSMMU, IS_ISPPRE_QCH_SYSMMU_ISPPRE, "GATE_IS_ISPPRE_SYSMMU", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_3AA, IS_ISPPRE_QCH_3AA, "GATE_IS_ISPPRE_3AA", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_3AAM, IS_ISPPRE_QCH_3AAM, "GATE_IS_ISPPRE_3AAM", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_PDP_CORE0, IS_ISPPRE_QCH_PDP_CORE0, "GATE_IS_ISPPRE_PDP_CORE0", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IS_ISPPRE_PDP_CORE1, IS_ISPPRE_QCH_PDP_CORE1, "GATE_IS_ISPPRE_PDP_CORE1", "UMUX_CLKCMU_ISPPRE_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_iva_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_IVA_BUS, MUX_CLKCMU_IVA_BUS_USER, "UMUX_CLKCMU_IVA_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_IVA, IVA_QCH_IVA, "GATE_IVA", "UMUX_CLKCMU_IVA_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IVA_DEBUG, IVA_QCH_IVA_DEBUG, "GATE_IVA_DEBUG", "UMUX_CLKCMU_IVA_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_IVA_INTMEM, IVA_INTMEM_QCH, "GATE_IVA_INTMEM", "UMUX_CLKCMU_IVA_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_IVA, SYSMMU_IVA_QCH, "GATE_SMMU_IVA", "UMUX_CLKCMU_IVA_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_TREX_RB_IVA, TREX_RB_IVA_QCH, "GATE_TREX_RB_IVA", "UMUX_CLKCMU_IVA_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_mfc_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_MFC_BUS, MUX_CLKCMU_MFC_BUS_USER, "UMUX_CLKCMU_MFC_BUS", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_MFC_WFD, MUX_CLKCMU_MFC_WFD_USER, "UMUX_CLKCMU_MFC_WFD", "UMUX_CLKCMU_MFC_BUS", 0, 0, NULL),

	HWACG_VCLK(GATE_MFC, MFC_QCH, "GATE_MFC", "UMUX_CLKCMU_MFC_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WFD, WFD_QCH, "GATE_WFD", "UMUX_CLKCMU_MFC_WFD", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_MFCD0, SYSMMU_MFCD0_QCH, "GATE_SMMU_MFCD0", "UMUX_CLKCMU_MFC_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SMMU_MFCD1, SYSMMU_MFCD1_QCH, "GATE_SMMU_MFCD1", "UMUX_CLKCMU_MFC_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_peric0_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_PERIC0_BUS, MUX_CLKCMU_PERIC0_BUS_USER, "UMUX_CLKCMU_PERIC0_BUS", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_PERIC0_IP, MUX_CLKCMU_PERIC0_IP_USER, "UMUX_CLKCMU_PERIC0_IP", "UMUX_CLKCMU_PERIC0_BUS", 0, 0, NULL),

	HWACG_VCLK(GATE_PWM, PWM_QCH, "GATE_PWM", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_UART_DBG, UART_DBG_QCH, "GATE_UART_DBG", "UMUX_CLKCMU_PERIC0_BUS", 0, 0, "console-pclk0"),
	HWACG_VCLK(GATE_USI00_I2C, USI00_I2C_QCH, "GATE_USI00_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
#ifdef ENABLE_SENSORS_FPRINT_SECURE
	HWACG_VCLK(GATE_USI00, USI00_USI_QCH, "GATE_USI00_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, "fp-spi-pclk"),
#else
	HWACG_VCLK(GATE_USI00, USI00_USI_QCH, "GATE_USI00_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
#endif
	HWACG_VCLK(GATE_USI01_I2C, USI01_I2C_QCH, "GATE_USI01_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI01, USI01_USI_QCH, "GATE_USI01_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI02_I2C, USI02_I2C_QCH, "GATE_USI02_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI02, USI02_USI_QCH, "GATE_USI02_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI03_I2C, USI03_I2C_QCH, "GATE_USI03_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, "clk_peric_usi03"),
	HWACG_VCLK(GATE_USI03, USI03_USI_QCH, "GATE_USI03_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI04_I2C, USI04_I2C_QCH, "GATE_USI04_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI04, USI04_USI_QCH, "GATE_USI04_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI05_I2C, USI05_I2C_QCH, "GATE_USI05_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI05, USI05_USI_QCH, "GATE_USI05_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI12_I2C, USI12_I2C_QCH, "GATE_USI12_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI12, USI12_USI_QCH, "GATE_USI12_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI13_I2C, USI13_I2C_QCH, "GATE_USI13_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI13, USI13_USI_QCH, "GATE_USI13_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI14_I2C, USI14_I2C_QCH, "GATE_USI14_I2C", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI14, USI14_USI_QCH, "GATE_USI14_USI", "UMUX_CLKCMU_PERIC0_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_peric1_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_PERIC1_BUS, MUX_CLKCMU_PERIC1_BUS_USER, "UMUX_CLKCMU_PERIC1_BUS", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_PERIC1_IP, MUX_CLKCMU_PERIC1_IP_USER, "UMUX_CLKCMU_PERIC1_IP", "UMUX_CLKCMU_PERIC1_BUS", 0, 0, NULL),

	HWACG_VCLK(GATE_I2C_CAM0, I2C_CAM0_QCH, "GATE_I2C_CAM0", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CAM1, I2C_CAM1_QCH, "GATE_I2C_CAM1", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CAM2, I2C_CAM2_QCH, "GATE_I2C_CAM2", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_I2C_CAM3, I2C_CAM3_QCH, "GATE_I2C_CAM3", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_SPI_CAM0, SPI_CAM0_QCH, "GATE_SPI_CAM0", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_UART_BT, UART_BT_QCH, "GATE_UART_BT", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI06_I2C, USI06_I2C_QCH, "GATE_USI06_I2C", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI06, USI06_USI_QCH, "GATE_USI06", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI07_I2C, USI07_I2C_QCH, "GATE_USI07_I2C", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI07, USI07_USI_QCH, "GATE_USI07", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI08_I2C, USI08_I2C_QCH, "GATE_USI08_I2C", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI08, USI08_USI_QCH, "GATE_USI08", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI09_I2C, USI09_I2C_QCH, "GATE_USI09_I2C", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI09, USI09_USI_QCH, "GATE_USI09", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI10_I2C, USI10_I2C_QCH, "GATE_USI10_I2C", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI10, USI10_USI_QCH, "GATE_USI10", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI11_I2C, USI11_I2C_QCH, "GATE_USI11_I2C", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_USI11, USI11_USI_QCH, "GATE_USI11", "UMUX_CLKCMU_PERIC1_IP", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_peris_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_PERIS_BUS, MUX_CLKCMU_PERIS_BUS_USER, "UMUX_CLKCMU_PERIS_BUS", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_BUSIF_TMU, BUSIF_TMU_QCH, "GATE_BUSIF_TMU", "UMUX_CLKCMU_PERIS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_GIC, GIC_QCH, "GATE_GIC", "UMUX_CLKCMU_PERIS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MCT, MCT_QCH, "GATE_MCT", "UMUX_CLKCMU_PERIS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_OTP_CON_BIRA, OTP_CON_BIRA_QCH, "GATE_OTP_CON_BIRA", "UMUX_CLKCMU_PERIS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_OTP_CON_TOP, OTP_CON_TOP_QCH, "GATE_OTP_CON_TOP", "UMUX_CLKCMU_PERIS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT_CLUSTER0, WDT_CLUSTER0_QCH, "GATE_WDT_CLUSTER0", "UMUX_CLKCMU_PERIS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT_CLUSTER1, WDT_CLUSTER1_QCH, "GATE_WDT_CLUSTER1", "UMUX_CLKCMU_PERIS_BUS", 0, VCLK_GATE, NULL),
};

struct init_vclk exynos9810_vts_hwacg_vclks[] __initdata = {
	HWACG_VCLK(UMUX_CLKCMU_VTS_BUS, MUX_CLKCMU_VTS_BUS_USER, "UMUX_CLKCMU_VTS_BUS", NULL, 0, 0, NULL),
	HWACG_VCLK(UMUX_CLKCMU_VTS_DLL, MUX_CLKCMU_VTS_DLL_USER, "UMUX_CLKCMU_VTS_DLL", NULL, 0, 0, NULL),

	HWACG_VCLK(GATE_VTS_CPU, CORTEXM4INTEGRATION_QCH_CPU, "GATE_CORTEXM4_CPU", "UMUX_CLKCMU_VTS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_VTS_DMIC_IF, DMIC_IF_QCH_DMIC_CLK, "GATE_DMIC_IF", "UMUX_CLKCMU_VTS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_HWACG_SYS_DMIC0, HWACG_SYS_DMIC0_QCH, "GATE_HWACG_SYS_DMIC0", "UMUX_CLKCMU_VTS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_HWACG_SYS_DMIC1, HWACG_SYS_DMIC1_QCH, "GATE_HWACG_SYS_DMIC1", "UMUX_CLKCMU_VTS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_MAILBOX_VTS2CHUB, MAILBOX_VTS2CHUB_QCH, "GATE_MAILBOX_VTS2CHUB", "UMUX_CLKCMU_VTS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_WDT_VTS, WDT_VTS_QCH, "GATE_WDT_VTS", "UMUX_CLKCMU_VTS_BUS", 0, VCLK_GATE, NULL),
	HWACG_VCLK(GATE_U_DMIC_CLK_MUX, U_DMIC_CLK_MUX_QCH, "GATE_U_DMIC_CLK_MUX", "UMUX_CLKCMU_VTS_BUS", 0, VCLK_GATE, NULL),
};

/* Special VCLK */
struct init_vclk exynos9810_apm_vclks[] __initdata = {
	VCLK(APM_DLL_CMGP, CLKCMU_APM_DLL_CMGP, "APM_DLL_CMGP", 0, 0, NULL),
	VCLK(APM_DLL_VTS, CLKCMU_APM_DLL_VTS, "APM_DLL_VTS", 0, 0, NULL),
};

struct init_vclk exynos9810_abox_vclks[] __initdata = {
	VCLK(DOUT_CLK_ABOX_ACLK, DIV_CLK_AUD_BUS, "DOUT_CLK_ABOX_ACLK", 0, 0, NULL),
	VCLK(DOUT_CLK_ABOX_AUDIF, DIV_CLK_AUD_AUDIF, "DOUT_CLK_ABOX_AUDIF", 0, 0, NULL),
	VCLK(DOUT_CLK_ABOX_DSIF, DIV_CLK_AUD_DSIF, "DOUT_CLK_ABOX_DSIF", 0, 0, NULL),
	VCLK(DOUT_CLK_ABOX_DMIC, DIV_CLK_AUD_DMIC, "DOUT_CLK_ABOX_DMIC", 0, 0, NULL),
	VCLK(DOUT_CLK_ABOX_UAIF0, DIV_CLK_AUD_UAIF0, "DOUT_CLK_ABOX_UAIF0", 0, 0, NULL),
	VCLK(DOUT_CLK_ABOX_UAIF1, DIV_CLK_AUD_UAIF1, "DOUT_CLK_ABOX_UAIF1", 0, 0, NULL),
	VCLK(DOUT_CLK_ABOX_UAIF2, DIV_CLK_AUD_UAIF2, "DOUT_CLK_ABOX_UAIF2", 0, 0, NULL),
	VCLK(DOUT_CLK_ABOX_UAIF3, DIV_CLK_AUD_UAIF3, "DOUT_CLK_ABOX_UAIF3", 0, 0, NULL),
	VCLK(PLL_OUT_AUD, PLL_AUD, "PLL_OUT_AUD", 0, 0, NULL),
};

struct init_vclk exynos9810_chub_vclks[] __initdata = {
	VCLK(CHUB_USI00, DIV_CLK_CHUB_USI00, "CHUB_USI00", 0, 0, NULL),
	VCLK(CHUB_USI01, DIV_CLK_CHUB_USI01, "CHUB_USI01", 0, 0, NULL),
	VCLK(CHUB_USI_I2C, DIV_CLK_CHUB_I2C, "CHUB_USI_I2C", 0, 0, NULL),
	VCLK(CHUB_TIMER_FCLK, CLK_CHUB_TIMER_FCLK, "CHUB_TIMER_FCLK", 0, 0, NULL),
};

struct init_vclk exynos9810_cmgp_vclks[] __initdata = {
	VCLK(USI_CMGP00, DIV_CLK_USI_CMGP00, "USI_CMGP00", 0, 0, NULL),
	VCLK(USI_CMGP01, DIV_CLK_USI_CMGP01, "USI_CMGP01", 0, 0, NULL),
	VCLK(USI_CMGP02, DIV_CLK_USI_CMGP02, "USI_CMGP02", 0, 0, NULL),
	VCLK(USI_CMGP03, DIV_CLK_USI_CMGP03, "USI_CMGP03", 0, 0, NULL),
	VCLK(CMGP_USI_I2C, DIV_CLK_I2C_CMGP, "CMGP_USI_I2C", 0, 0, NULL),
	VCLK(CMGP_ADC, DIV_CLK_CMGP_ADC, "CMGP_ADC", 0, 0, NULL),
};

struct init_vclk exynos9810_fsys0_vclks[] __initdata = {
	VCLK(UFS_EMBD, VCLK_CLK_FSYS0_UFS_EMBD_BLK_CMU, "UFS_EMBD", 0, 0, NULL),
	VCLK(DPGTC, VCLK_CLK_FSYS0_DPGTC_BLK_CMU, "DPGTC", 0, 0, NULL),
	VCLK(USB30DRD, VCLK_CLK_FSYS0_USB30DRD_BLK_CMU, "USB30DRD", 0, 0, NULL),
	VCLK(USBDP_DEBUG_USER, VCLK_MUX_CLKCMU_FSYS0_USBDP_DEBUG_USER_BLK_CMU, "USBDP_DEBUG_USER", 0, 0, NULL),
};

struct init_vclk exynos9810_fsys1_vclks[] __initdata = {
	VCLK(MMC_CARD, CLKCMU_FSYS1_MMC_CARD, "MMC_CARD", 0, 0, NULL),
	VCLK(UFS_CARD, VCLK_CLK_FSYS1_UFS_CARD_BLK_CMU, "UFS_CARD", 0, 0, NULL),
};

struct init_vclk exynos9810_peric0_vclks[] __initdata = {
	VCLK(UART_DBG, DIV_CLK_PERIC0_UART_DBG, "UART_DBG", 0, 0, "console-sclk0"),
#ifdef ENABLE_SENSORS_FPRINT_SECURE
	VCLK(USI00, DIV_CLK_PERIC0_USI00_USI, "USI00_USI", 0, 0, "fp-spi-sclk"),
#else
	VCLK(USI00, DIV_CLK_PERIC0_USI00_USI, "USI00_USI", 0, 0, NULL),
#endif
	VCLK(USI01, DIV_CLK_PERIC0_USI01_USI, "USI01_USI", 0, 0, NULL),
	VCLK(USI02, DIV_CLK_PERIC0_USI02_USI, "USI02_USI", 0, 0, NULL),
	VCLK(USI03, DIV_CLK_PERIC0_USI03_USI, "USI03_USI", 0, 0, NULL),
	VCLK(USI04, DIV_CLK_PERIC0_USI04_USI, "USI04_USI", 0, 0, NULL),
	VCLK(USI05, DIV_CLK_PERIC0_USI05_USI, "USI05_USI", 0, 0, NULL),
	VCLK(USI12, DIV_CLK_PERIC0_USI12_USI, "USI12_USI", 0, 0, NULL),
	VCLK(USI13, DIV_CLK_PERIC0_USI13_USI, "USI13_USI", 0, 0, NULL),
	VCLK(USI14, DIV_CLK_PERIC0_USI14_USI, "USI14_USI", 0, 0, NULL),
	VCLK(PERIC0_USI_I2C, DIV_CLK_PERIC0_USI_I2C, "PERIC0_USI_I2C", 0, 0, NULL),
};

struct init_vclk exynos9810_peric1_vclks[] __initdata = {
	VCLK(UART_BT, DIV_CLK_PERIC1_UART_BT, "UART_BT", 0, 0, NULL),
	VCLK(I2C_CAM0, DIV_CLK_PERIC1_I2C_CAM0, "I2C_CAM0", 0, 0, NULL),
	VCLK(I2C_CAM1, DIV_CLK_PERIC1_I2C_CAM1, "I2C_CAM1", 0, 0, NULL),
	VCLK(I2C_CAM2, DIV_CLK_PERIC1_I2C_CAM2, "I2C_CAM2", 0, 0, NULL),
	VCLK(I2C_CAM3, DIV_CLK_PERIC1_I2C_CAM3, "I2C_CAM3", 0, 0, NULL),
	VCLK(SPI_CAM0, DIV_CLK_PERIC1_SPI_CAM0, "SPI_CAM0", 0, 0, NULL),
	VCLK(USI06, DIV_CLK_PERIC1_USI06_USI, "USI06", 0, 0, NULL),
	VCLK(USI07, DIV_CLK_PERIC1_USI07_USI, "USI07", 0, 0, NULL),
	VCLK(USI08, DIV_CLK_PERIC1_USI08_USI, "USI08", 0, 0, NULL),
	VCLK(USI09, DIV_CLK_PERIC1_USI09_USI, "USI09", 0, 0, NULL),
	VCLK(USI10, DIV_CLK_PERIC1_USI10_USI, "USI10", 0, 0, NULL),
	VCLK(USI11, DIV_CLK_PERIC1_USI11_USI, "USI11", 0, 0, NULL),
	VCLK(PERIC1_USI_I2C, DIV_CLK_PERIC1_USI_I2C, "PERIC1_USI_I2C", 0, 0, NULL),
};

struct init_vclk exynos9810_cmu_vclks[] __initdata = {
	VCLK(CIS_CLK0, CLKCMU_CIS_CLK0, "CIS_CLK0", 0, 0, NULL),
	VCLK(CIS_CLK1, CLKCMU_CIS_CLK1, "CIS_CLK1", 0, 0, NULL),
	VCLK(CIS_CLK2, CLKCMU_CIS_CLK2, "CIS_CLK2", 0, 0, NULL),
	VCLK(CIS_CLK3, CLKCMU_CIS_CLK3, "CIS_CLK3", 0, 0, NULL),
	VCLK(HPM, VCLK_CLKCMU_HPM_BLK_CMU, "HPM", 0, 0, NULL),
};

struct init_vclk exynos9810_vts_vclks[] __initdata = {
	VCLK(DOUT_CLK_VTS_DMICIF, DIV_CLK_VTS_DMIC_IF, "DOUT_CLK_VTS_DMICIF", 0, 0, NULL),
	VCLK(DOUT_CLK_VTS_DMIC, DIV_CLK_VTS_DMIC, "DOUT_CLK_VTS_DMIC", 0, 0, NULL),
	VCLK(DOUT_CLK_VTS_DMIC_DIV2, DIV_CLK_VTS_DMIC_DIV2, "DOUT_CLK_VTS_DMIC_DIV2", 0, 0, NULL),
};

static struct init_vclk exynos9810_clkout_vclks[] __initdata = {
	VCLK(OSC_NFC, VCLK_CLKOUT1, "OSC_NFC", 0, 0, NULL),
	VCLK(OSC_AUD, VCLK_CLKOUT0, "OSC_AUD", 0, 0, NULL),
};

static __initdata struct of_device_id ext_clk_match[] = {
	{.compatible = "samsung,exynos9810-oscclk", .data = (void *)0},
	{},
};

void exynos9810_vclk_init(void)
{
	/* Common clock init */
}

/* register exynos9810 clocks */
void __init exynos9810_clk_init(struct device_node *np)
{
	void __iomem *reg_base;
	int ret;

	if (np) {
		reg_base = of_iomap(np, 0);
		if (!reg_base)
			panic("%s: failed to map registers\n", __func__);
	} else {
		panic("%s: unable to determine soc\n", __func__);
	}

	ret = cal_if_init(np);
	if (ret)
		panic("%s: unable to initialize cal-if\n", __func__);

	exynos9810_clk_provider = samsung_clk_init(np, reg_base, CLK_NR_CLKS);
	if (!exynos9810_clk_provider)
		panic("%s: unable to allocate context.\n", __func__);

	samsung_register_of_fixed_ext(exynos9810_clk_provider, exynos9810_fixed_rate_ext_clks,
					  ARRAY_SIZE(exynos9810_fixed_rate_ext_clks),
					  ext_clk_match);
	/* register HWACG vclk */
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_apm_hwacg_vclks, ARRAY_SIZE(exynos9810_apm_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_abox_hwacg_vclks, ARRAY_SIZE(exynos9810_abox_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_bus1_hwacg_vclks, ARRAY_SIZE(exynos9810_bus1_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_busc_hwacg_vclks, ARRAY_SIZE(exynos9810_busc_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_chub_hwacg_vclks, ARRAY_SIZE(exynos9810_chub_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_cmgp_hwacg_vclks, ARRAY_SIZE(exynos9810_cmgp_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_cmu_hwacg_vclks, ARRAY_SIZE(exynos9810_cmu_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_core_hwacg_vclks, ARRAY_SIZE(exynos9810_core_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_dpu_hwacg_vclks, ARRAY_SIZE(exynos9810_dpu_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_dspm_hwacg_vclks, ARRAY_SIZE(exynos9810_dspm_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_dsps_hwacg_vclks, ARRAY_SIZE(exynos9810_dsps_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_fsys0_hwacg_vclks, ARRAY_SIZE(exynos9810_fsys0_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_fsys1_hwacg_vclks, ARRAY_SIZE(exynos9810_fsys1_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_g2d_hwacg_vclks, ARRAY_SIZE(exynos9810_g2d_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_g3d_hwacg_vclks, ARRAY_SIZE(exynos9810_g3d_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_isphq_hwacg_vclks, ARRAY_SIZE(exynos9810_isphq_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_isplp_hwacg_vclks, ARRAY_SIZE(exynos9810_isplp_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_isppre_hwacg_vclks, ARRAY_SIZE(exynos9810_isppre_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_iva_hwacg_vclks, ARRAY_SIZE(exynos9810_iva_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_mfc_hwacg_vclks, ARRAY_SIZE(exynos9810_mfc_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_peric0_hwacg_vclks, ARRAY_SIZE(exynos9810_peric0_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_peric1_hwacg_vclks, ARRAY_SIZE(exynos9810_peric1_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_peris_hwacg_vclks, ARRAY_SIZE(exynos9810_peris_hwacg_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_vts_hwacg_vclks, ARRAY_SIZE(exynos9810_vts_hwacg_vclks));

	/* register special vclk */
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_apm_vclks, ARRAY_SIZE(exynos9810_apm_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_abox_vclks, ARRAY_SIZE(exynos9810_abox_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_chub_vclks, ARRAY_SIZE(exynos9810_chub_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_cmgp_vclks, ARRAY_SIZE(exynos9810_cmgp_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_cmu_vclks, ARRAY_SIZE(exynos9810_cmu_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_fsys0_vclks, ARRAY_SIZE(exynos9810_fsys0_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_fsys1_vclks, ARRAY_SIZE(exynos9810_fsys1_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_peric0_vclks, ARRAY_SIZE(exynos9810_peric0_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_peric1_vclks, ARRAY_SIZE(exynos9810_peric1_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_vts_vclks, ARRAY_SIZE(exynos9810_vts_vclks));
	samsung_register_vclk(exynos9810_clk_provider, exynos9810_clkout_vclks, ARRAY_SIZE(exynos9810_clkout_vclks));

	clk_register_fixed_factor(NULL, "pwm-clock", "fin_pll", CLK_SET_RATE_PARENT, 1, 1);

	samsung_clk_of_add_provider(np, exynos9810_clk_provider);

	late_time_init = exynos9810_vclk_init;

	pr_info("EXYNOS9810: Clock setup completed\n");
}

CLK_OF_DECLARE(exynos9810_clk, "samsung,exynos9810-clock", exynos9810_clk_init);
