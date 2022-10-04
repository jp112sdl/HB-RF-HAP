/* 
 *  led.cpp is part of the HB-RF-ETH firmware - https://github.com/alexreinert/HB-RF-ETH
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

#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"


const osThreadAttr_t ledTask_attributes = {
  .name = "LED_Switcher",
  .stack_size = 128,
  .priority = (osPriority_t) osPriorityNormal,
};

static uint8_t _blinkState = 0;
static LED *_leds[MAX_LED_COUNT] = {0};
static osThreadId_t _ledTaskHandle = NULL;
static TIM_HandleTypeDef *_htim;

static int _highDuty;

void ledSwitcherTask(void *parameter)
{
    for (;;)
    {
        _blinkState = (_blinkState + 1) % 24;

        for (uint8_t i = 0; i < MAX_LED_COUNT; i++)
        {
            if (_leds[i] == 0)
            {
                break;
            }
            _leds[i]->updatePinState();
        }
        vTaskDelay(125 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

void LED::start(Settings *settings, TIM_HandleTypeDef *htim)
{
	_htim = htim;
    _highDuty = settings->getLEDBrightness() * 255;

    if (!_ledTaskHandle)
    {
    	_ledTaskHandle = osThreadNew(ledSwitcherTask, NULL, &ledTask_attributes);
    }
}

void LED::stop()
{
    if (_ledTaskHandle)
    {
    	osThreadTerminate(_ledTaskHandle);
        vTaskDelete(_ledTaskHandle);
        _ledTaskHandle = NULL;
    }
}

LED::LED(uint16_t chn)
{
    for (uint8_t i = 0; i < MAX_LED_COUNT; i++)
    {
        if (_leds[i] == 0)
        {
        	_leds[i] = this;
           _chn = chn;
            break;
        }
    }
}

void LED::setState(led_state_t state)
{
    _state = state;
    updatePinState();
}

void LED::_setPinState(bool enabled) {
   __HAL_TIM_SET_COMPARE(_htim, _chn, enabled ? _highDuty : 0);
}

void LED::updatePinState()
{
    switch (_state)
    {
    case LED_STATE_OFF:
        _setPinState(false);
        break;
    case LED_STATE_ON:
        _setPinState(true);
        break;
    case LED_STATE_BLINK:
        _setPinState((_blinkState % 8) < 4);
        break;
    case LED_STATE_BLINK_INV:
        _setPinState((_blinkState % 8) >= 4);
        break;
    case LED_STATE_BLINK_FAST:
        _setPinState((_blinkState % 2) == 0);
        break;
    case LED_STATE_BLINK_SLOW:
        _setPinState(_blinkState < 12);
        break;
    }
}
