/*
 *  main.cpp is part of the HB-RF-HAP firmware - https://github.com/jp112sdl/HB-RF-HAP
 *
 *  Copyright 09/2022 jp112sdl
 *
 *  The HB-RF-HAP firmware is licensed under a
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
#include "main.h"
#include "retarget.h"

#include "cmsis_os.h"
#include "lwip.h"
#include <stdio.h>

#include "init.h"
#include "web.h"
#include "settings.h"
#include "led.h"
#include "mdnsresponder.h"
#include "linereader.h"
#include "radiomoduleconnector.h"
#include "radiomoduledetector.h"
#include "rawuartudplistener.h"

void start(RadioModuleConnector *rm);
osThreadId_t mainTaskHandle;
const osThreadAttr_t mainTask_attributes = {
  .name = "mainTask",
  .stack_size = 512,
  .priority = (osPriority_t) osPriorityNormal,
};

void startMainTask(void *argument) {
  printf("Starting MAIN task\n");

  Settings settings;
  WebUI webui(&settings);
  LED blueLED(HM_BLUE_CHN);
  LED redLED(HM_RED_CHN);
  LED greenLED(HM_GREEN_CHN);
  LED::start(&settings, &htim3);

  blueLED.setState(LED_STATE_OFF);
  redLED.setState(LED_STATE_OFF);
  greenLED.setState(LED_STATE_OFF);
  factoryReset(&settings, &blueLED, &redLED, &greenLED);
  redLED.setState(LED_STATE_BLINK);

  printf("Starting LWIP\n");
  setPHY();
  MX_LWIP_Init(&settings);
  if (netif_is_link_up(&gnetif) == false) {
	  printf("WARNING: ETH LINK DOWN\n");
  }
  printf("Hostname: %s\n", settings.getHostname());

  printf("Starting RadioModuleConnector\n");

  RadioModuleConnector radioModuleConnector(&redLED, &greenLED, &blueLED);
  radioModuleConnector.start();

  printf("Starting MDNS\n");
  mdns_init(netif_default);
  mdns_add_facility(settings.getHostname(), "_http", NULL, (mdns_flags)(mdns_TCP + mdns_Browsable), 80, 600);
  mdns_add_facility(settings.getHostname(), "_raw-uart", NULL, (mdns_flags)(mdns_UDP + mdns_Browsable), 3008, 600);
  mdns_add_facility(settings.getHostname(), "_ntp", NULL, (mdns_flags)(mdns_UDP + mdns_Browsable), 123, 600);

  printf("Starting RadioModuleDetector\n");

  RadioModuleDetector radioModuleDetector;
  radioModuleDetector.setLogFrameUART(&huart1);
  radioModuleDetector.detectRadioModule(&radioModuleConnector);

  radio_module_type_t radioModuleType = radioModuleDetector.getRadioModuleType();
  if (radioModuleType != RADIO_MODULE_NONE)
  {
      switch (radioModuleType)
      {
      case RADIO_MODULE_HM_MOD_RPI_PCB:
          printf("Detected HM-MOD-RPI-PCB:\n");
          break;
      case RADIO_MODULE_RPI_RF_MOD:
    	  printf("Detected RPI-RF-MOD:\n");
          break;
      case RADIO_MODULE_HMIP_RFUSB:
    	  printf("Detected HMIP-RFUSB:\n");
          break;
      default:
    	  printf("Detected unknown radio module:\n");
          break;
      }

      printf("  Serial: %s\n", radioModuleDetector.getSerial());
      printf("  SGTIN: %s\n", radioModuleDetector.getSGTIN());
      printf("  BidCos Radio MAC: 0x%06X\n", radioModuleDetector.getBidCosRadioMAC());
      printf("  HmIP Radio MAC: 0x%06X\n", radioModuleDetector.getHmIPRadioMAC());

      const uint8_t *firmwareVersion = radioModuleDetector.getFirmwareVersion();
      printf("  Firmware Version: %d.%d.%d\n", *firmwareVersion, *(firmwareVersion + 1), *(firmwareVersion + 2));
  }
  else
  {
	  printf("Radio module could not be detected.");
  }


  radioModuleConnector.resetModule();

  printf("Starting RawUartUdpListener\n");
  RawUartUdpListener rawUartUdpLister(&radioModuleConnector);
  rawUartUdpLister.start();

  printf("Starting WebUI\n");
  webui.start(radioModuleDetector.getSGTIN(), radioModuleDetector.getFirmwareVersion());

  if (settings.getUseDHCP() == false) {
	  printf("Static IP: %s\n",ip4addr_ntoa(netif_ip4_addr(&gnetif)));
  }
  redLED.setState(LED_STATE_OFF);
  greenLED.setState(LED_STATE_BLINK);
  printf("Initialization done.\n");
  int lastLEDBrightness = -1;
  uint32_t regvalue;

  /* for debugging only */
  //char *current_ipaddr = static_cast<char *>(malloc(sizeof(ip4_addr_t)));
  //static char * last_ipaddr= {0};

  for(;;)  {
	if (lastLEDBrightness != settings.getLEDBrightness()) {
		lastLEDBrightness = settings.getLEDBrightness();
		LED::start(&settings, &htim3);
	}
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if (HAL_ETH_ReadPHYRegister(&heth, PHY_BSR, &regvalue) == HAL_OK) {
      if((regvalue & PHY_LINKED_STATUS)== (uint16_t)RESET) {
         if (netif_is_link_up(&gnetif)) {
      	   if (settings.getUseDHCP() == true ) { dhcp_stop(&gnetif); }
           netif_set_down(&gnetif);
           netif_set_link_down(&gnetif);
          }
      } else {
        if (!netif_is_link_up(&gnetif)) {
          //if (settings.getUseDHCP() == true && dhcp_supplied_address(&gnetif) == 0) last_ipaddr = {0};
  	      netif_set_link_up(&gnetif);
  	      netif_set_up(&gnetif);
  	      if (settings.getUseDHCP() == true ) { dhcp_start(&gnetif); }
        }
      }
    }

    /* for debugging only */
    //if ((last_ipaddr != current_ipaddr) && (settings.getUseDHCP() == true) && (dhcp_supplied_address(&gnetif)>0)) {
    //    current_ipaddr = ip4addr_ntoa(netif_ip4_addr(&gnetif));
    //    if (current_ipaddr != last_ipaddr) {
    //      printf("DHCP IP: %s\n", current_ipaddr);
    //      last_ipaddr = current_ipaddr;
    //    }
    //}
  }
  vTaskSuspend(NULL);
}

void Error_Handler(void) {
  __disable_irq();
  while (1) { }
}

int main(void) {
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  RetargetInit(&huart1);

  MX_TIM3_Init();

  __HAL_TIM_SET_COMPARE(&htim3, HM_RED_CHN, 0xFFFF);

  osKernelInitialize();
  mainTaskHandle = osThreadNew(startMainTask, NULL, &mainTask_attributes );
  osKernelStart();
  while (1){}
}

