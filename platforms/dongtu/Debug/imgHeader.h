/***************************************************************************
*				科东(广州)软件科技有限公司 版权所有
* 	 Copyright (C) 2000-2026 CoreTek Systems Inc. All Rights Reserved.
***************************************************************************/
/**
* @file: imgHeader.h
* @brief:
*        <li>映像头定义<li>
* @implements:
*/

#ifndef _IMHEADER_H
#define _IMHEADER_H

/************************头 文 件******************************/

#include <imImageInfo.h>

/************************宏 定 义******************************/

/************************类型定义******************************/

/************************外部变量声明******************************/


extern T_CHAR _text_start[];
extern T_CHAR store__text_size[];
extern T_CHAR _text_size[];
extern T_CHAR load__text_start[];

extern T_CHAR _sidata_start[];
extern T_CHAR store__sidata_size[];
extern T_CHAR _sidata_size[];
extern T_CHAR load__sidata_start[];

extern T_CHAR _iplt_start[];
extern T_CHAR store__iplt_size[];
extern T_CHAR _iplt_size[];
extern T_CHAR load__iplt_start[];

extern T_CHAR _data_start[];
extern T_CHAR store__sidata_size[];
extern T_CHAR _sidata_size[];
extern T_CHAR load__data_start[];

extern T_CHAR _bss_start[];
extern T_CHAR store__bss_size[];
extern T_CHAR _bss_size[];
extern T_CHAR load__bss_start[];

extern T_CHAR init_stack_start[];
extern T_CHAR store_init_stack_size[];
extern T_CHAR init_stack_size[];
extern T_CHAR load_init_stack_start[];

/************************全局变量******************************/

/*映像头定义*/
T_TA_ImageInfo g_imageHeader __attribute__((__section__(".image_info"))) = 
{
	"rtos_bench",/*映像名*/  
	"Version1",/*映像版本*/  
	IMAGERUNTYPE_RAM,/*运行位置*/  
	1,/*映像类型*/  
	6,  /*段总数*/  
	0x40000000,  /*运行地址*/  
	IM_MAGIC_HEAD_NUMBER,  /*魔数*/  
	/*段属性*/  
	{           
		/*段名称  是否加载到内存  加载到内存中的地址(物理地址)  运行地址(逻辑地址)  存储段大小(bin文件中占的空间)*/  
		{".text", TRUE,(T_ULONG)load__text_start, (T_ULONG)_text_start,(T_ULONG)store__text_size},
		{".sidata", FALSE,(T_ULONG)load__sidata_start, (T_ULONG)_sidata_start,(T_ULONG)store__sidata_size},
		{".iplt", FALSE,(T_ULONG)load__iplt_start, (T_ULONG)_iplt_start,(T_ULONG)store__iplt_size},
		{".data", TRUE,(T_ULONG)load__data_start, (T_ULONG)_data_start,(T_ULONG)store__sidata_size},
		{".bss", FALSE,(T_ULONG)load__bss_start, (T_ULONG)_bss_start,(T_ULONG)store__bss_size},
		{"init_stack", FALSE,(T_ULONG)load_init_stack_start, (T_ULONG)init_stack_start,(T_ULONG)init_stack_size}
	}
};

#endif
