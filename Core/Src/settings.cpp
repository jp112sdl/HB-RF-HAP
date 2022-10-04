/* 
 *  settings.cpp is part of the HB-RF-ETH firmware - https://github.com/alexreinert/HB-RF-ETH
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

#include "settings.h"
#include "eeprom.h"
#include <string.h>

template <class T>
inline void DPRINT(T str);
template <class T>
inline void DPRINTLN(T str);
template<typename TYPE>
inline void DDECLN(TYPE b);

Settings::Settings()
{
  load();
}

void Settings::load()
{

  if (EEPROM.read(0) != 0x9e) {
	  printf("FIRST EEPROM INIT!\n");
	  clear();
	  return;
  }
/*

   size_t ntpServerLength = sizeof(_ntpServer);
  if (nvs_get_str(handle, "ntpServer", _ntpServer, &ntpServerLength) != ESP_OK)
  {
    strncpy(_ntpServer, "pool.ntp.org", sizeof(_ntpServer) - 1);
  }


  size_t adminPasswordLength = sizeof(_adminPassword);
  if (nvs_get_str(handle, "adminPassword", _adminPassword, &adminPasswordLength) != ESP_OK)
  {
    strncpy(_adminPassword, "admin", sizeof(_adminPassword) - 1);
  }
*/

  /*
  uint32_t *uid = (uint32_t*)(UID_BASE+0x04UL );
  uint8_t baseMac[6] = {
		  0x02,
		  0xF3,
		  (uint8_t)((*uid >> 24) & 0xff),
		  (uint8_t)((*uid >> 16) & 0xff),
		  (uint8_t)((*uid >> 8 ) & 0xff),
		  (uint8_t)((*uid      ) & 0xff)
  };
  snprintf(_hostname, sizeof(_hostname) - 1, "HB-RF-HAP-%02X%02X%02X%02X%02X%02X",baseMac[0],baseMac[1],baseMac[2],baseMac[3],baseMac[4],baseMac[5]);
  */

  _useDHCP = EEPROM.read(EE_ADDR_USEDHCP);
  _ledBrightness = EEPROM.read(EE_ADDR_LEDBRIGHT);

  EEPROM.get(EE_ADDR_HOSTNAME, _hostname);
  EEPROM.get(EE_ADDR_LOCALIP, _localIP);
  EEPROM.get(EE_ADDR_NETMASK, _netmask);
  EEPROM.get(EE_ADDR_GATEWAY, _gateway);
}

void Settings::save()
{
	EEPROM.write(EE_ADDR_USEDHCP, _useDHCP);
	EEPROM.write(EE_ADDR_LEDBRIGHT, _ledBrightness);
	EEPROM.put(EE_ADDR_LOCALIP, _localIP);
	EEPROM.put(EE_ADDR_NETMASK, _netmask);
	EEPROM.put(EE_ADDR_GATEWAY, _gateway);
	EEPROM.put(EE_ADDR_HOSTNAME, _hostname);

//  SET_STR(handle, "adminPassword", _adminPassword);
//  SET_STR(handle, "ntpServer", _ntpServer);
}

void Settings::clear()
{
  EEPROM.write(0, 0x9e);
  _useDHCP = 1;
  _ledBrightness = 255;
  _localIP.addr=0;
  _netmask.addr=0;
  _gateway.addr=0;
  snprintf(_hostname, sizeof(_hostname) - 1, "HB-RF-HAP");

  save();
  load();
}

char *Settings::getAdminPassword()
{
  return _adminPassword;
}

void Settings::setAdminPassword(char *adminPassword)
{
  strncpy(_adminPassword, adminPassword, sizeof(_adminPassword) - 1);
}

char *Settings::getHostname()
{
  return _hostname;
}

bool Settings::getUseDHCP()
{
  return _useDHCP;
}

ip4_addr_t Settings::getLocalIP()
{
  return _localIP;
}

ip4_addr_t Settings::getNetmask()
{
  return _netmask;
}

ip4_addr_t Settings::getGateway()
{
  return _gateway;
}

void Settings::setNetworkSettings(char *hostname, uint8_t useDHCP, ip4_addr_t localIP, ip4_addr_t netmask, ip4_addr_t gateway)
{
  strncpy(_hostname, hostname, sizeof(_hostname) - 1);
  _useDHCP = useDHCP;
  _localIP = localIP;
  _netmask = netmask;
  _gateway = gateway;
}

char *Settings::getNtpServer()
{
  return _ntpServer;
}

void Settings::setNtpServer(char *ntpServer)
{
  strncpy(_ntpServer, ntpServer, sizeof(_ntpServer) - 1);
}

int Settings::getLEDBrightness()
{
  return _ledBrightness;
}

void Settings::setLEDBrightness(int ledBrightness)
{
  _ledBrightness = ledBrightness;
}
