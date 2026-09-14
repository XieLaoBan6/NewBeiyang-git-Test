/*
* 文件名称：    AppProtocol.h
* 功能描述：    SC_Sealer 应用层协议（上位机 <-> 安全控制器）
* 当前版本号:   V2.0
*
* 帧格式（主机下发 / 设备回显 结构相同，负载经 Base64 编码后入帧）:
*   +--------+--------+-------+-------+------------------+--------+---------+
*   | 0x02   | VER    | CMD   | 0x80  | Base64 负载(可选) | 0x03   | XOR     |
*   +--------+--------+-------+-------+------------------+--------+---------+
*   XOR = VER ^ CMD ^ 0x80 ^ Base64字符... ^ 0x03    （若结果 < 10 则 +10）
*
* 设备事件（裸字节，无帧结构）:
*   0x06            ACK（应答成功）
*   0x07            Checksum Error（校验和错误）
*   0x08 <状态>      Command Processing Error（命令处理失败）
*
* @note 协议依据《SC_Sealer_指令分析报告.md》，当前实现命令:
*       START_SESSION / EXIT_SESSION / GET_CTRL_VERSION / GET_SENSOR_INFO
*       GET_INPUTS / CLOSE_SHUTTER / OPEN_SHUTTER
*/

#ifndef __APP_PROTOCOL_H__
#define __APP_PROTOCOL_H__

#include "main.h"

/* 帧控制字节 ---------------------------------------------------------------*/
#define SC_STX                          0x02U   /* 帧起始 */
#define SC_ETX                          0x03U   /* 帧结束（真实抓包帧存在，参与校验） */
#define SC_FLAGS                        0x80U   /* 固定标志 */

/* 事件字节 -----------------------------------------------------------------*/
#define SC_EVT_ACK                      0x06U   /* 应答成功 */
#define SC_EVT_CHECKSUM_ERR             0x07U   /* 校验和错误 */
#define SC_EVT_STATUS_ERR               0x08U   /* 命令处理错误，后跟状态码 */

/* 版本字节 -----------------------------------------------------------------*/
#define SC_VER_SESSION                  0x0AU   /* 会话/查询/配置类 */
#define SC_VER_CTRL                     0x0BU   /* 封口/闸门/下载类 */

/* 命令字节 -----------------------------------------------------------------*/
#define SC_CMD_SESSION                  0x0AU   /* START/EXIT_SESSION（靠负载区分） */
#define SC_CMD_GET_SENSOR_INFO          0x10U   /* 查询传感器信息 */
#define SC_CMD_GET_CTRL_VERSION         0x12U   /* 查询控制板版本 */
#define SC_CMD_GET_INPUTS               0x13U   /* 查询输入状态 */
#define SC_CMD_OPEN_SHUTTER             0x1BU   /* 打开闸门 */
#define SC_CMD_CLOSE_SHUTTER            0x1CU   /* 关闭闸门 */
#define SC_CMD_SHUTTER_REPORT           0xCDU   /* 设备上报：闸门动作完成 */

/* 会话负载首字节 -----------------------------------------------------------*/
#define SC_SESSION_START                0x31U   /* 31 00 建立会话 */
#define SC_SESSION_EXIT                 0x30U   /* 30 00 结束会话 */

/* 错误状态码（0x08 事件第二字节） ------------------------------------------*/
#define SC_STATUS_SEALER_ERR            0x08U   /* 封口器类操作失败 */
#define SC_STATUS_DOOR_CFG_ERR          0x09U   /* 门/配置类操作失败、无会话 */
#define SC_STATUS_SHUTTER_BUSY          0x09U   /* 闸门忙（上一动作未完成） */

/* 上报状态码 ---------------------------------------------------------------*/
#define SC_REPORT_OK                    0x00U   /* 动作成功 */
#define SC_REPORT_ERR                   0x07U   /* 动作失败（超时等） */

/* 负载长度 -----------------------------------------------------------------*/
#define SC_PAYLOAD_VERSION_LEN          13U     /* 版本：9字节0前缀 + 4字节版本号 */
#define SC_PAYLOAD_SENSOR_LEN           6U      /* 传感器位图 */
#define SC_PAYLOAD_INPUTS_LEN           6U      /* 输入位图 */
#define SC_PAYLOAD_REPORT_LEN           6U      /* 闸门完成上报 */

/* 固件版本（GET_CTRL_VERSION 返回） ----------------------------------------*/
#define SC_CTRL_VER_MAJOR               1U
#define SC_CTRL_VER_MINOR               0U
#define SC_CTRL_VER_BUILD               0U
#define SC_CTRL_VER_REV                 0U

/* 缓冲区 -------------------------------------------------------------------*/
#define SC_RX_BUF_SIZE                  128U
#define SC_TX_BUF_SIZE                  128U
#define SC_PAYLOAD_MAX                  64U

/* 对外接口 -----------------------------------------------------------------*/
void AppProtocolInit(void);
void AppProtocolPoll(void);                     /* 主循环周期调用 */

#endif /* __APP_PROTOCOL_H__ */
