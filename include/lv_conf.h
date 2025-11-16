#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   GENERAL SETTINGS
 *====================*/

/* Color depth: 1 (1 byte per pixel), 8, 16, 32 (4 bytes per pixel)*/
#define LV_COLOR_DEPTH 16

/* Enable anti-aliasing by default. Can be turned off for specific objects.*/
#define LV_ANTIALIAS 0

/* Enable file system interface*/
#define LV_USE_FS_STDIO 0

/* Enable memory allocator and dynamic memory usage*/
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (32U * 1024U) /* 32KB for LVGL's internal memory heap */

/* Enable logging (recommended for debugging)*/
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/*====================
   HAL SETTINGS
 *====================*/

/* Driver for your display*/
#define LV_USE_DISP_DRV 1
#define LV_DISP_DEF_REFR_PERIOD 30 /* 30 ms refresh period (33 FPS) */

/* Driver for your input device*/
#define LV_USE_INDEV_DRV 1

/*====================
   FEATURE USAGE
 *====================*/

/* Widgets */
#define LV_USE_BTN 1
#define LV_USE_LABEL 1
#define LV_USE_BAR 1
#define LV_USE_CHART 1
#define LV_USE_TABLE 1
#define LV_USE_DROPDOWN 1
#define LV_USE_ROLLER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_MSGBOX 1
#define LV_USE_ARC 1
#define LV_USE_LINE 1
#define LV_USE_LED 1
#define LV_USE_IMG 1
#define LV_USE_QRCODE 1

/* Themes */
#define LV_USE_THEME_DEFAULT 1

/* Styles */
#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/* Animations */
#define LV_USE_ANIMATION 1

/* Groups (for keyboard/encoder navigation) */
#define LV_USE_GROUP 1

/*====================
   COMPILER SETTINGS
 *====================*/

/* C++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_CONF_H*/
