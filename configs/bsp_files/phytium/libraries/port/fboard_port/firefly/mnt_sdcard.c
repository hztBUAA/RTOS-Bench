/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * 本文件由 RTOS-Bench 修改：原版硬编码 FS_SD_DEVICE_INDEX=0 / FS_SD_DEVICE_NAME="sd00"，
 * 在 RT-Thread 5.x 命名（<disk>p<partno>）与 SDIF0/1 归属不确定的情况下会挂载失败。
 * 现改为：遍历候选块设备名，挂载第一个能成功的 FAT 分区到 `/`。
 */
#include <rtthread.h>
#include <dfs_fs.h>

#define DBG_TAG "mnt.filesystem"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define FS_SD_MOUNT_POINT    "/"

/*
 * 候选设备名，按顺序尝试：
 *   - RT-Thread 5.x 分区命名 "<disk>p<partno>"（blk_dev.c:204），disk = "sd0"/"sd1"
 *     （drv_sdif.c:644-646）；无分区表时整盘以 partno 0 暴露为 "<disk>p0"。
 *   - 同时覆盖 SDIF0 / SDIF1 两种接法。
 * dfs_mount(..., "elm", ...) 只在真正的 FAT 分区上成功，故无需预先按类型筛选。
 */
static const char *const sd_candidates[] =
{
    "sd0p0", "sd0p1", "sd0p2", "sd0p3", "sd0",
    "sd1p0", "sd1p1", "sd1p2", "sd1p3", "sd1",
};

static char fs_mounted_dev[RT_NAME_MAX] = { 0 };

static int _sdcard_try_mount(void)
{
    rt_size_t i;

    for (i = 0; i < sizeof(sd_candidates) / sizeof(sd_candidates[0]); i++)
    {
        const char *name = sd_candidates[i];

        if (rt_device_find(name) == RT_NULL)
        {
            continue;
        }

        if (dfs_mount(name, FS_SD_MOUNT_POINT, "elm", 0, 0) == RT_EOK)
        {
            LOG_I("%s mount to '%s'", name, FS_SD_MOUNT_POINT);
            rt_strncpy(fs_mounted_dev, name, RT_NAME_MAX - 1);
            fs_mounted_dev[RT_NAME_MAX - 1] = '\0';
            return 1;
        }
    }

    return 0;
}

static void sd_auto_mount(void *parameter)
{
    RT_UNUSED(parameter);

    rt_thread_mdelay(200);

    while (RT_TRUE)
    {
        if (fs_mounted_dev[0] == '\0')
        {
            _sdcard_try_mount();
        }
        else if (rt_device_find(fs_mounted_dev) == RT_NULL)
        {
            rt_thread_mdelay(200);
            dfs_unmount(FS_SD_MOUNT_POINT);
            LOG_I("Unmount %s", FS_SD_MOUNT_POINT);
            fs_mounted_dev[0] = '\0';
        }

        rt_thread_mdelay(1000);
    }
}

static void sd_mount(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("sd_mount", sd_auto_mount, RT_NULL,
                           4096, RT_THREAD_PRIORITY_MAX - 2, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        LOG_E("create sd_mount thread err!");
    }
}

int filesystem_mount(void)
{
    sd_mount();
    return RT_EOK;
}
INIT_APP_EXPORT(filesystem_mount);
