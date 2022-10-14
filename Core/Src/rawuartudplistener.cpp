/* 
 *  rawuartudplistener.cpp is part of the HB-RF-ETH firmware - https://github.com/alexreinert/HB-RF-ETH
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

#include "rawuartudplistener.h"
#include "hmframe.h"
#include <string.h>
#include "udphelper.h"
#include "radiomoduleconnector.h"

static RadioModuleConnector *_radioModuleConnector;
std::atomic<uint> _remoteAddress;
std::atomic<ushort> _remotePort;
std::atomic<bool> _connectionStarted;
std::atomic<int> _counter;
std::atomic<int> _endpointConnectionIdentifier;
uint32_t _lastReceivedKeepAlive;
udp_pcb *_pcb;

const osThreadAttr_t udpQueueHandlerTask_attributes = {
  .name = "UDPQueueHandler",
  .stack_size = 2432,
  .priority = (osPriority_t) osPriorityAboveNormal
};
osThreadId_t _udpQueueHandlerTaskHandle = NULL;
QueueHandle_t _udp_queue;

void _raw_uart_udpQueueHandlerTask(void *parameter)
{
    ((RawUartUdpListener *)parameter)->_udpQueueHandler();
}

void _raw_uart_udpReceivePaket(void *arg, udp_pcb *pcb, pbuf *pb, const ip_addr_t *addr, uint16_t port)
{
    while (pb != NULL)
    {
        pbuf *this_pb = pb;
        pb = pb->next;
        this_pb->next = NULL;
        if (!((RawUartUdpListener *)arg)->_udpReceivePacket(this_pb, addr, port))
        {
            pbuf_free(this_pb);
        }
    }
}

RawUartUdpListener::RawUartUdpListener(RadioModuleConnector *radioModuleConnector)
{
	_radioModuleConnector = radioModuleConnector;
    atomic_init(&_connectionStarted, false);
    atomic_init(&_remotePort, (ushort)0);
    atomic_init(&_remoteAddress, 0u);
    atomic_init(&_counter, 0);
    atomic_init(&_endpointConnectionIdentifier, 1);
}

void RawUartUdpListener::handlePacket(pbuf *pb, ip4_addr_t addr, uint16_t port)
{
    size_t length = pb->len;
    unsigned char *data = (unsigned char *)(pb->payload);
    unsigned char response_buffer[3];

    if (length < 4)
    {
        printf("Received invalid raw-uart packet, length %d\n", length);
        return;
    }

    if (data[0] != 0 && (addr.addr != atomic_load(&_remoteAddress) || port != atomic_load(&_remotePort)))
    {
        printf("Received raw-uart packet from invalid address.\n");
        return;
    }

    if (*((uint16_t *)(data + length - 2)) != htons(HMFrame::crc(data, length - 2)))
    {
        printf("Received raw-uart packet with invalid crc.\n");
        return;
    }

    _lastReceivedKeepAlive = xTaskGetTickCount()*portTICK_RATE_MS;

    switch (data[0])
    {
    case 0: // connect
        if (length == 5 && data[2] == 1)
        { // protocol version 1
        	printf("connected with protocol version 1\n");
            atomic_fetch_add(&_endpointConnectionIdentifier, 2);
            atomic_store(&_remotePort, (ushort)0);
            atomic_store(&_connectionStarted, false);
            atomic_store(&_remoteAddress, addr.addr);
            atomic_store(&_remotePort, port);
            _radioModuleConnector->setLED(true, true, false);
            response_buffer[0] = 1;
            response_buffer[1] = data[1];
            sendMessage(0, response_buffer, 2);
        }
        else if (length == 6 && data[2] == 2) {
            int endpointConnectionIdentifier  = atomic_load(&_endpointConnectionIdentifier);

            if (data[3] == 0)
            {
                endpointConnectionIdentifier += 2;
                atomic_store(&_endpointConnectionIdentifier, endpointConnectionIdentifier);
                atomic_store(&_connectionStarted, false);
            }
            else if (data[3] != (endpointConnectionIdentifier & 0xff))
            {
                printf("Received raw-uart reconnect packet with invalid endpoint identifier %d, should be %d\n", data[3], endpointConnectionIdentifier);
                return;
            }
        	printf("connected with protocol version 2\n");

            atomic_store(&_remotePort, (ushort)0);
            atomic_store(&_remoteAddress, addr.addr);
            atomic_store(&_remotePort, port);
            _radioModuleConnector->setLED(true, true, false);
            response_buffer[0] = 2;
            response_buffer[1] = data[1];
            response_buffer[2] = endpointConnectionIdentifier;
            sendMessage(0, response_buffer, 3);
        }
        else {
            printf("Received invalid raw-uart connect packet, length %d\n", length);
            return;
        }
        break;

    case 1: // disconnect
        atomic_store(&_remotePort, (ushort)0);
        atomic_store(&_connectionStarted, false);
        atomic_store(&_remoteAddress, 0u);
        _radioModuleConnector->setLED(false, false, false);
        break;

    case 2: // keep alive
        break;

    case 3: // LED
        if (length != 5)
        {
            printf("Received invalid raw-uart LED packet, length %d\n", length);
            return;
        }

        _radioModuleConnector->setLED(data[2] & 1, data[2] & 2, data[2] & 4);
        break;

    case 4: // Reset
        if (length != 4)
        {
            printf("Received invalid raw-uart reset packet, length %d\n", length);
            return;
        }

        _radioModuleConnector->resetModule();
        break;

    case 5: // Start connection
        if (length != 4)
        {
            printf("Received invalid raw-uart startconn packet, length %d\n", length);
            return;
        }

        atomic_store(&_connectionStarted, true);
        break;

    case 6: // End connection
        if (length != 4)
        {
            printf("Received invalid raw-uart endconn packet, length %d\n", length);
            return;
        }

        atomic_store(&_connectionStarted, false);
        break;

    case 7: // Frame
        if (length < 5)
        {
            printf("Received invalid raw-uart frame packet, length %d\n", length);
            return;
        }

        //printf("RX[%02x]:",length); for ( uint8_t i = 2; i < length - 2; i++) { printf(" %02x",data[i]); } printf("\n");
        _radioModuleConnector->sendFrame(&data[2], length - 4);
        break;

    default:
        printf("Received invalid raw-uart packet with unknown type %d\n", data[0]);
        break;
    }
}

ip_addr_t RawUartUdpListener::getConnectedRemoteAddress()
{
    uint16_t port = atomic_load(&_remotePort);
    uint32_t address = atomic_load(&_remoteAddress);
    ip_addr_t temp;
    ip4_addr_set_any(&temp);
    if (port)
    {
        temp.addr = address;
    }
    return temp;
}

void RawUartUdpListener::sendMessage(unsigned char command, unsigned char *buffer, size_t len)
{
    uint16_t port = atomic_load(&_remotePort);
    uint32_t address = atomic_load(&_remoteAddress);

    pbuf *pb = pbuf_alloc(PBUF_TRANSPORT, len + 4, PBUF_RAM);
    unsigned char *sendBuffer = (unsigned char *)pb->payload;

    ip_addr_t addr;
    addr.addr = address;

    if (!port)
        return;

    sendBuffer[0] = command;
    sendBuffer[1] = (unsigned char)atomic_fetch_add(&_counter, 1);

    if (len)
        memcpy(sendBuffer + 2, buffer, len);

    *((uint16_t *)(sendBuffer + len + 2)) = htons(HMFrame::crc(sendBuffer, len + 2));

    _udp_sendto(_pcb, pb, &addr, port);
    pbuf_free(pb);

}

void RawUartUdpListener::handleFrame(unsigned char *buffer, uint16_t len)
{
    if (!atomic_load(&_connectionStarted))
        return;

    if (len > (1500 - 28 - 4))
    {
        printf("Received oversized frame from radio module, length %d\n", len);
        return;
    }

    //patch SGTIN to represent device type 16 (271) (HmIP-RFUSB) and not 15 (270) (HmIP-HAP)
    if (len == 0x15) {
    	if (buffer[5] == 0x05 && buffer[6] == 0x01) {
    		uint8_t partition = (uint8_t)(buffer[1+7] >> 2 & 7);
    		if (partition == 5) {

    		  printf("*");

    		  buffer[13] = 0x04;
    		  buffer[14] = 0x1D;

    	      uint16_t crc = HMFrame::crc(buffer, len-2);
    	      buffer[len-2] = (crc >> 8) & 0xff;
    	      buffer[len-1] = crc & 0xff;
    		}
    	}
    }
    //printf("TX[%02x]:",len); for ( uint8_t i = 0; i < len; i++) { printf(" %02x",buffer[i]); } printf("\n");
    sendMessage(7, buffer, len);
}

void RawUartUdpListener::start()
{
    _udp_queue = xQueueCreate(32, sizeof(udp_event_t *));

    _udpQueueHandlerTaskHandle = osThreadNew(_raw_uart_udpQueueHandlerTask, ( void * ) 1, &udpQueueHandlerTask_attributes);

    _pcb = udp_new();
    udp_recv(_pcb, &_raw_uart_udpReceivePaket, (void *)this);

    _udp_bind(_pcb, IP4_ADDR_ANY, 3008);

    _radioModuleConnector->setFrameHandler(this, false);
}

void RawUartUdpListener::stop()
{
    _udp_disconnect(_pcb);
    udp_recv(_pcb, NULL, NULL);
    _udp_remove(_pcb);
    _pcb = NULL;

    _radioModuleConnector->setFrameHandler(NULL, false);
    vTaskDelete(_udpQueueHandlerTaskHandle);
}

void RawUartUdpListener::_udpQueueHandler()
{
    udp_event_t *event = NULL;
    uint32_t nextKeepAliveSentOut =xTaskGetTickCount()*portTICK_RATE_MS;

    for (;;)
    {

        if (xQueueReceive(_udp_queue, &event, (portTickType)(100 / portTICK_PERIOD_MS)) == pdTRUE)
        {
            handlePacket(event->pb, event->addr, event->port);
            pbuf_free(event->pb);
            free(event);
        }

        if (atomic_load(&_remotePort) != 0)
        {
            uint32_t now = xTaskGetTickCount()*portTICK_RATE_MS;

            if (now > _lastReceivedKeepAlive + 5000)
            { // 5 sec
                atomic_store(&_remotePort, (ushort)0);
                atomic_store(&_remoteAddress, 0u);
                _radioModuleConnector->setLED(true, false, false);
                printf("Connection timed out\n");
            }

            if (now > nextKeepAliveSentOut)
            {
                nextKeepAliveSentOut = now + 1000; // 1sec
                sendMessage(2, NULL, 0);
            }
        }

    }

    vTaskDelete(NULL);
}

bool RawUartUdpListener::_udpReceivePacket(pbuf *pb, const ip_addr_t *addr, uint16_t port)
{
    udp_event_t *e = (udp_event_t *)malloc(sizeof(udp_event_t));
    if (!e)
    {
        return false;
    }

    e->pb = pb;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpointer-arith"

    ip_hdr *iphdr = reinterpret_cast<ip_hdr *>(pb->payload - UDP_HLEN - IP_HLEN);
    e->addr.addr = iphdr->src.addr;

    udp_hdr *udphdr = reinterpret_cast<udp_hdr *>(pb->payload - UDP_HLEN);
    e->port = ntohs(udphdr->src);

    #pragma GCC diagnostic pop

    if (xQueueSend(_udp_queue, &e, portMAX_DELAY) != pdPASS)
    {
        free((void *)(e));
        return false;
    }
    return true;
}

/*
Index 0 - Type: 0-Connect, 1-Disconnect, 2-KeepAlive, 3-LED, 4-StartConn, 5-StopConn, 6-Reset, 7-Frame
Index 1 - Counter
Index 2..n-2 - Payload
Index n-2,n-1 - CRC16

Payload:
  Keepalive: Empty
  Connect: 1 Byte: Protocol version
  LED: 1 Byte: Bit 0 R, Bit 1 G, Bit 2 B
  Reset: Empty
  Frame: Frame-Data
*/
