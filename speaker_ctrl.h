#ifndef __SPEAKER_CTRL_H__
#define __SPEAKER_CTRL_H__

#include "main.h"     /* 声明 hadc1/htim1/htim3 等句柄 */

#ifdef __cplusplus
extern "C" {
#endif

void speaker_ctrl_start(void);   /* 一键启动所有外设 */

#ifdef __cplusplus
}
#endif
#endif /* __SPEAKER_CTRL_H__ */
