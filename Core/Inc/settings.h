/* 
 *  settings.h is part of the HB-RF-ETH firmware - https://github.com/alexreinert/HB-RF-ETH
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

#include <stdio.h>
#include <lwip/ip4_addr.h>

#define EE_ADDR_HOSTNAME     1
#define EE_LEN_HOSTNAME      33

#define EE_ADDR_ADMINPASS    34
#define EE_LEN_ADMINPASS     33

#define EE_ADDR_NTPSERVER    67
#define EE_LEN_NTPSERVER     65

#define EE_ADDR_LOCALIP		 132
#define EE_LEN_LOCALIP		 4
#define EE_ADDR_NETMASK		 136
#define EE_LEN_NETMASK		 4
#define EE_ADDR_GATEWAY		 140
#define EE_LEN_GATEWAY		 4

#define EE_ADDR_LEDBRIGHT	 144
#define EE_LEN_LEDBRIGHT     1

#define EE_ADDR_USEDHCP      145
#define EE_LEN_USEDHCP       1

class Settings
{
private:
  char _adminPassword[33] = {0};

  char _hostname[33] = {0};
  bool _useDHCP;
  ip4_addr_t _localIP;
  ip4_addr_t _netmask;
  ip4_addr_t _gateway;

  char _ntpServer[65] = {0};
  int _ledBrightness;

public:
  Settings();
  void load();
  void save();
  void clear();

  char *getAdminPassword();
  void setAdminPassword(char* password);

  char *getHostname();
  bool getUseDHCP();
  ip4_addr_t getLocalIP();
  ip4_addr_t getNetmask();
  ip4_addr_t getGateway();

  void setNetworkSettings(char *hostname, uint8_t useDHCP, ip4_addr_t localIP, ip4_addr_t netmask, ip4_addr_t gateway);

  char *getNtpServer();
  void setNtpServer(char *ntpServer);

  int getLEDBrightness();
  void setLEDBrightness(int brightness);
};
