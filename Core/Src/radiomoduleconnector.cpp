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
  .stack_size = 2432,
  .priority = (osPriority_t) osPriorityNormal
};
 osThreadId_t _serialQueueHandlerTaskHandle = NULL;

 std::atomic<FrameHandler *> _frameHandler = ATOMIC_VAR_INIT(0);

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
  //printf("reset radio module\n");
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
	volatile uint8_t rcvbyte[255] = {0};
	volatile uint16_t len = 1;
    unsigned char UART_RcvBuffer[255]={0};
	uint16_t rxlen;

    for (;;)
    {
    	taskENTER_CRITICAL();
    	while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE) == SET) {
    	    HAL_UART_Receive(&huart5, (uint8_t *)rcvbyte, 1, 0);
    	    if (rcvbyte[0] == 0xFD) {
    	    	HAL_UARTEx_ReceiveToIdle(&huart5, (uint8_t *)rcvbyte, 255, &rxlen, 5);
	    		UART_RcvBuffer[0] = 0xFD;

    	    	for (uint16_t i = 0; i < rxlen; i++) {
    	    		UART_RcvBuffer[len] = rcvbyte[i];
    	    		if (rcvbyte[i] == 0xFD) {
    	    			_streamParser->append(UART_RcvBuffer, len);
    	    			len = 1;
      	    			UART_RcvBuffer[0] = 0xFD;
    	    		}
    	    		len = len + 1;
    	    	}
    	    	_streamParser->append(UART_RcvBuffer, len);
    	    	len = 1;
    	    }
    	}

  		taskEXIT_CRITICAL();
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
