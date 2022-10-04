/*
 *  mdnsresponder.h is part of the HB-RF-HAP firmware - https://github.com/jp112sdl/HB-RF-HAP
 *
 *  Copyright jp112sdl 09/2022
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

#ifdef __cplusplus
extern "C" {
#endif

#define LWIP_IANA_PORT_MDNS 5353

#include <lwip/ip_addr.h>

#ifndef MDNS_RESPONDER_REPLY_SIZE
#define MDNS_RESPONDER_REPLY_SIZE      320
#endif

#ifndef MDNS_RESPONDER_DEBUGGING
#define MDNS_RESPONDER_DEBUGGING      0
#endif


void mdns_init(struct netif *station_netif);

typedef enum {
    mdns_TCP,
    mdns_UDP,
    mdns_Browsable        // see RFC6763:11 - adds a standard record that lets browsers find the service without needing to know its name
} mdns_flags;

void mdns_add_facility( const char* instanceName,
                        const char* serviceName,
                        const char* addText,
                        mdns_flags  flags,
                        u16_t       onPort,
                        u32_t       ttl
                      );


void mdns_add_PTR(const char* rKey, u32_t ttl, const char* nameStr);
void mdns_add_SRV(const char* rKey, u32_t ttl, u16_t rPort, const char* targname);
void mdns_add_TXT(const char* rKey, u32_t ttl, const char* txtStr);
void mdns_add_A  (const char* rKey, u32_t ttl, const ip4_addr_t *addr);


#ifdef __cplusplus
}
#endif
