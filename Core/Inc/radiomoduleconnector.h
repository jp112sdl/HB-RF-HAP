/* 
 *  radiomoduleconnector.h is part of the HB-RF-ETH firmware - https://github.com/alexreinert/HB-RF-ETH
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

#pragma once

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "queue.h"
#include "led.h"
#include "streamparser.h"
#include <atomic>
#define _Atomic(X) std::atomic<X>

#define EFM32_RESET_PORT GPIOB
#define EFM32_RESET_PIN  GPIO_PIN_6

#define UART_RCV_BUFFER_SIZE 0x200

class FrameHandler
{
public:
    virtual void handleFrame(unsigned char *buffer, uint16_t len) = 0;
};

class RadioModuleConnector
{
private:
    LED *_redLED;
    LED *_greenLED;
    LED *_blueLED;

    void _handleFrame(unsigned char *buffer, uint16_t len);

public:
    RadioModuleConnector(LED *redLED, LED *greenLed, LED *blueLed);

    void start();
    void stop();

    void setLED(bool red, bool green, bool blue);

    void setFrameHandler(FrameHandler *handler, bool decodeEscaped);

    void resetModule();

    void sendFrame(unsigned char *buffer, uint16_t len);

    void _serialQueueHandler();
};
