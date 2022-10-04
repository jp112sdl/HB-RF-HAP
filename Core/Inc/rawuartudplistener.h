/* 
 *  rawuartudplistener.h is part of the HB-RF-ETH firmware - https://github.com/alexreinert/HB-RF-ETH
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

#include "lwip/opt.h"
#include "lwip/inet.h"
#include "lwip/udp.h"
#include "lwip/priv/tcpip_priv.h"
#include <atomic>
#define _Atomic(X) std::atomic<X>
#include "radiomoduleconnector.h"

class RawUartUdpListener : FrameHandler
{
private:
    void handlePacket(pbuf *pb, ip4_addr_t addr, uint16_t port);
    void sendMessage(unsigned char command, unsigned char *buffer, size_t len);

public:
    RawUartUdpListener(RadioModuleConnector *radioModuleConnector);

    void handleFrame(unsigned char *buffer, uint16_t len);
    void handleEvent();

    ip4_addr_t getConnectedRemoteAddress();

    void start();
    void stop();

    void _udpQueueHandler();
    bool _udpReceivePacket(pbuf *pb, const ip_addr_t *addr, uint16_t port);
};
