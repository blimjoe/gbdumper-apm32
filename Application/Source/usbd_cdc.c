/*!
 * @file        usbd_cdc.c
 *
 * @brief       CDC source file
 *
 * @version     V1.0.1
 *
 * @date        2022-12-01
 *
 * @attention
 *
 *  Copyright (C) 2020-2022 Geehy Semiconductor
 *
 *  You may not use this file except in compliance with the
 *  GEEHY COPYRIGHT NOTICE (GEEHY SOFTWARE PACKAGE LICENSE).
 *
 *  The program is only for reference, which is distributed in the hope
 *  that it will be useful and instructional for customers to develop
 *  their software. Unless required by applicable law or agreed to in
 *  writing, the program is distributed on an "AS IS" BASIS, WITHOUT
 *  ANY WARRANTY OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the GEEHY SOFTWARE PACKAGE LICENSE for the governing permissions
 *  and limitations under the License.
 */

/* Includes */
#include "usbd_cdc.h"
#include "usbd_descriptor.h"
#include "usbd_class_cdc.h"
#include <string.h>
#include <main.h>

/** @addtogroup Examples
  @{
*/

/** @addtogroup USB_CDC_VirtualCOMPort
  @{
*/

/** @defgroup USB_CDC_VirtualCOMPort_Variables Variables
  @{
*/

static uint8_t dataBuf[256] = {0};

static void USBD_VCP_SetConfigCallBack(void);

int SIZETYPE = 0;

/* Standard Request handler */
USBD_StdReqCallback_T stdReqCallback =
{
    NULL,      /*!< getConfigurationHandler; */
    NULL,      /*!< getDescriptorHandler; */
    NULL,      /*!< getInterfaceHandler; */
    NULL,      /*!< getStatusHandler; */
    NULL,      /*!< setAddressHandler; */
    USBD_VCP_SetConfigCallBack, /*!< setConfigurationHandler; */
    NULL,      /*!< setDescriptorHandler; */
    NULL,      /*!< setFeatureHandler; */
    NULL,      /*!< setInterfaceHandler; */
    NULL       /*!< clearFeatureHandler; */
};

/**@} end of group USB_CDC_VirtualCOMPort_Variables */

/** @defgroup USB_CDC_VirtualCOMPort_Functions Functions
  @{
  */

/*!
 * @brief       Reset
 *
 * @param       None
 *
 * @retval      None
 */
void VCP_Reset(void)
{
    USBD_EPConfig_T epConfig;

    /* Endpoint 1 IN */
    epConfig.epNum = USBD_EP_1;
    epConfig.epType = USBD_EP_TYPE_BULK;
    epConfig.epBufAddr = USB_EP1_TX_ADDR;
    epConfig.maxPackSize = 64;
    epConfig.epStatus = USBD_EP_STATUS_NAK;
    USBD_OpenInEP(&epConfig);

    epConfig.epBufAddr = USB_EP1_RX_ADDR;
    USBD_OpenOutEP(&epConfig);

    epConfig.epNum = USBD_EP_2;
    epConfig.epType = USBD_EP_TYPE_INTERRUPT;
    epConfig.epBufAddr = USB_EP2_TX_ADDR;
    epConfig.maxPackSize = 8;
    USBD_OpenInEP(&epConfig);
}

/*!
 * @brief       Standard request set configuration call back
 *
 * @param       None
 *
 * @retval      None
 */
static void USBD_VCP_SetConfigCallBack(void)
{
    USBD_RxData(USBD_EP_1, dataBuf, g_usbDev.outBuf[USBD_EP_1].maxPackSize);
}

/*!
 * @brief       OUT endpoint transfer done handler(except EP0)
 *
 * @param       ep : OUT endpoint
 *
 * @retval      None
 */

void USBD_VCP_OutEpCallback(uint8_t ep)
{
    uint32_t dataCnt;
    char message[256];
		extern void Delay();

    if (ep == USBD_EP_1)
    {
        dataCnt = g_usbDev.outBuf[USBD_EP_1].xferCnt;

        if (memcmp(dataBuf, "1", 1) == 0) {
					extern void readHeader();
					readHeader();
        }
        else if (memcmp(dataBuf, "2", 1) == 0) {
					extern void dumpRom(void);
					dumpRom();
        }
				else if (memcmp(dataBuf, "3", 1) == 0) {
					extern void readram(void);
					readram();
				}
				else if (memcmp(dataBuf, "0", 1) == 0) {
					// sending gba cart size
					// buf[1] = 1 = 1M, =2=4M, =3=8M, =4=16M, =5=32M;
					if (memcmp(dataBuf, "01", 2) == 0) { // 1M
						SIZETYPE = 1;
						strcpy(message, "success set size 01M");
					}
					else if (memcmp(dataBuf, "02", 2) == 0) { // 4M
						SIZETYPE = 2;
						strcpy(message, "success set size 04M");
					}
					else if (memcmp(dataBuf, "03", 2) == 0) { //8M
						SIZETYPE = 3;
						strcpy(message, "success set size 8M");
					}
					else if (memcmp(dataBuf, "04", 2) == 0) { //16M
						SIZETYPE = 4;
						strcpy(message, "success set size 16M");
					}
					else if (memcmp(dataBuf, "05", 2) == 0) { //32M
						SIZETYPE = 5;
						strcpy(message, "success set size 32M");
					}
					else {
						SIZETYPE = 0;
						strcpy(message, "failed");
					}
					USBD_TxData(USBD_EP_1, (uint8_t*)message, strlen(message)+1);
					memset(message, 0, sizeof(message));
				}
				else if (memcmp(dataBuf, "4", 1) == 0) {
					extern void readHeader_GBA(int time);
					readHeader_GBA(1);
        }
				else if (memcmp(dataBuf, "5", 1) == 0) {
					if(SIZETYPE != 0){
						extern void dump_GBA(int size_type);
						dump_GBA(SIZETYPE);
					}
					
        }
				else {
					strcpy(message, "unknown");
					USBD_TxData(USBD_EP_1, (uint8_t*)message, strlen(message)+1);
					memset(message, 0, sizeof(message));
				}
				memset(dataBuf, 0, sizeof(dataBuf));
			}
}


/*!
 * @brief       IN endpoint transfer done handler(except EP0)
 *
 * @param       ep : IN endpoint
 *
 * @retval      None
 */
void USBD_VCP_InEpCallback(uint8_t ep)
{
    if (ep == USBD_EP_1)
    {
			memset(dataBuf, 0, sizeof(dataBuf));
      USBD_RxData(USBD_EP_1, dataBuf, g_usbDev.outBuf[USBD_EP_1].maxPackSize);
			//USBD_RxData(USBD_EP_1, dataBuf, 1);
    }
}

/*!
 * @brief       USB cdc init
 *
 * @param       None
 *
 * @retval      None
 */
void CDC_Init(void)
{
    USBD_InitParam_T usbParam;

    USBD_InitParamStructInit(&usbParam);

    usbParam.classReqHandler = USBD_ClassHandler;
    usbParam.resetHandler = VCP_Reset;
    usbParam.inEpHandler = USBD_VCP_InEpCallback;
    usbParam.outEpHandler = USBD_VCP_OutEpCallback;
    usbParam.pDeviceDesc = (USBD_Descriptor_T *)&g_deviceDescriptor;
    usbParam.pConfigurationDesc = (USBD_Descriptor_T *)&g_configDescriptor;
    usbParam.pStringDesc = (USBD_Descriptor_T *)g_stringDescriptor;
    usbParam.pStdReqCallback = &stdReqCallback;

    USBD_Init(&usbParam);
}

/**@} end of group USB_CDC_VirtualCOMPort_Functions */
/**@} end of group USB_CDC_VirtualCOMPort */
/**@} end of group Examples */
