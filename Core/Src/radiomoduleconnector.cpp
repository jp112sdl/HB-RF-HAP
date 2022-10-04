/* 
 *  radiomoduleconnector.cpp is part of the HB-RF-ETH firmware - https://github.com/alexreinert/HB-RF-ETH
 *  modified for HB-RF-HAP: jp112sdl 09/2022
 *  
 *  Copyright 2022 Alexander Reinert / jp112sdl
 *  
 *  The HB-RF-ETH firmware is licensed under a
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *  
 *  You should have received a copy of the license along with this
 *  work.  If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "radiomoduleconnector.h"
#include "hmframe.h"

UART_HandleTypeDef huart5;

const osThreadAttr_t serialQueueHandlerTask_attributes = {
  .name = "RaMoQueueHandler",
  .stack_size = 2304,
  .priority = (osPriority_t) osPriorityNormal
};
 osThreadId_t _serialQueueHandlerTaskHandle = NULL;


static void UART5_Init(void) {
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_MultiProcessor_Init(&huart5, 0, UART_WAKEUPMETHOD_IDLELINE) != HAL_OK) { Error_Handler(); }
}

StreamParser *_streamParser =NULL;


void serialQueueHandlerTask(void *parameter)
{
    ((RadioModuleConnector *)parameter)->_serialQueueHandler();
}

RadioModuleConnector::RadioModuleConnector(LED *redLED, LED *greenLed, LED *blueLed) : _redLED(redLED), _greenLED(greenLed), _blueLED(blueLed)
{
    using namespace std::placeholders;
    _streamParser = new StreamParser(false, std::bind(&RadioModuleConnector::_handleFrame, this, _1, _2));
}

void RadioModuleConnector::start()
{

	UART5_Init();

    //setLED(false, false, false);

    _serialQueueHandlerTaskHandle = osThreadNew(serialQueueHandlerTask, NULL, &serialQueueHandlerTask_attributes);

    resetModule();
}

void RadioModuleConnector::stop()
{

    resetModule();

    if (_serialQueueHandlerTaskHandle)
    {
    	osThreadTerminate(_serialQueueHandlerTaskHandle);
        vTaskDelete(_serialQueueHandlerTaskHandle);
        _serialQueueHandlerTaskHandle = NULL;
    }
}

void RadioModuleConnector::setFrameHandler(FrameHandler *frameHandler, bool decodeEscaped)
{

    atomic_store(&_frameHandler, frameHandler);
    _streamParser->setDecodeEscaped(decodeEscaped);

}

void RadioModuleConnector::setLED(bool red, bool green, bool blue)
{
    _redLED->setState(red ? LED_STATE_ON : LED_STATE_OFF);
    _greenLED->setState(green ? LED_STATE_ON : LED_STATE_OFF);
    _blueLED->setState(blue ? LED_STATE_ON : LED_STATE_OFF);

}

void RadioModuleConnector::resetModule()
{
  printf("reset radio module\n");
  HAL_GPIO_WritePin(EFM32_RESET_PORT, EFM32_RESET_PIN,GPIO_PIN_RESET);
  vTaskDelay(50 / portTICK_PERIOD_MS);
  HAL_GPIO_WritePin(EFM32_RESET_PORT, EFM32_RESET_PIN,GPIO_PIN_SET);
  vTaskDelay(50 / portTICK_PERIOD_MS);
}

void RadioModuleConnector::sendFrame(unsigned char *buffer, uint16_t len)
{
	HAL_UART_Transmit(&huart5,(uint8_t *)buffer, len, 0xFFFF);
}

void RadioModuleConnector::_serialQueueHandler()
{
    for (;;)
    {
    	uint8_t rcvbyte[1] = {0};
    	uint8_t UART_RcvBuffer[255]={0};
    	uint16_t len = 1;
    	while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE) == SET) {
    	    HAL_UART_Receive(&huart5, (uint8_t *)rcvbyte, 1, 10);
    	    if (rcvbyte[0] == 0xFD) {
    		  UART_RcvBuffer[0] = 0xFD;
    		  HAL_UART_Receive(&huart5, (uint8_t *)rcvbyte, 1, 10);
    		  uint8_t len1=rcvbyte[0] * 255;
    		  UART_RcvBuffer[1] = rcvbyte[0]; //00
    		  HAL_UART_Receive(&huart5, (uint8_t *)rcvbyte, 1, 10);
    		  len += (len1 << 8) + rcvbyte[0];
    		  UART_RcvBuffer[2] = rcvbyte[0]; // 0f
    		  HAL_UART_Receive(&huart5, (uint8_t *)rcvbyte, 1, 10);
    		  UART_RcvBuffer[3] = rcvbyte[0]; // fe

    		  for (uint16_t i = 0; i < len; i++) {
    			  HAL_UART_Receive(&huart5, (uint8_t *)rcvbyte, 1, 10);
    			  UART_RcvBuffer[4+i] = rcvbyte[0];
    		  }
      	     _streamParser->append(UART_RcvBuffer, len+4);

    		  memset(UART_RcvBuffer, 0, 255);
    		  memset(rcvbyte, 0, 1);
    	    } else {
    	      _streamParser->flush();
    	    }

    	}
  		_streamParser->flush();
    }
    vTaskDelete(NULL);
}

void RadioModuleConnector::_handleFrame(unsigned char *buffer, uint16_t len)
{
    FrameHandler *frameHandler = (FrameHandler *)atomic_load(&_frameHandler);

    if (frameHandler)
    {
        frameHandler->handleFrame(buffer, len);
    }
}
