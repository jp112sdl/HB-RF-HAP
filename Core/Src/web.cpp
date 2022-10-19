/*
 * web.cpp
 *
 *  Created on: Sep 14, 2022
 *      Author: jp112sdl
 */

#include "lwip/opt.h"
#include "lwip/apps/httpd.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"

#include <stdio.h>
#include <string.h>
#include "httpd.h"
#include "web.h"
#include "settings.h"

static Settings *_settings;

const uint8_t * _rfw;
const char * _sgtin;
char  _sgtine[30] = {0};

tSSIHandler SETTINGS_Page_SSI_Handler;

const char * ssi_tags[] = {
  "hname",
  "ipaddr",
  "snetmsk",
  "gateway",
  "usedhcp",
  "ledb",
  "mac",
  "cip",
  "sgtine",
  "sgtinr",
  "rfw"
};

const char * SETTINGS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

const tCGI SETTINGS_CGI={"/settings.cgi", SETTINGS_CGI_Handler};

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[1];

u16_t SETTINGS_SSI_Handler(int iIndex, char *pcInsert, int iInsertLen) {
	size_t printed;
	  uint8_t useDHCP = _settings->getUseDHCP();
      uint8_t ledbrightness = _settings->getLEDBrightness();

      char *hname=_settings->getHostname();

	  ip4_addr ip;
	  switch (iIndex) {
	    case 0: /* "hname" */
		  printed =  sprintf(pcInsert, "%s", hname);
		  break;
	    case 1: /* "ipaddr" */
	      ip = _settings->getLocalIP();
	      printed =  sprintf(pcInsert, "%s", ip4addr_ntoa(&ip));
	      break;
	    case 2: /* "snetmask" */
		  ip = _settings->getNetmask();
	      printed =  sprintf(pcInsert, "%s", ip4addr_ntoa(&ip));
	      break;
	    case 3: /* "gateway" */
		  ip = _settings->getGateway();
	      printed =  sprintf(pcInsert, "%s", ip4addr_ntoa(&ip));
	      break;
	    case 4: /* "use dhcp" */
	      printed =  sprintf(pcInsert, "%s", (useDHCP == 0) ? "" : "checked" );
	      break;
	    case 5: /* "ledb" */
	      printed = sprintf(pcInsert, "%d", ledbrightness);
	    break;
	    case 6: /* "mac" */
	      printed = sprintf(pcInsert, "%02X:%02X:%02X:%02x:%02x:%02x", netif_default->hwaddr[0], netif_default->hwaddr[1], netif_default->hwaddr[2], netif_default->hwaddr[3], netif_default->hwaddr[4], netif_default->hwaddr[5]);
	    break;
	    case 7: /* "cib" */
	      printed = sprintf(pcInsert, "%s", ip4addr_ntoa(netif_ip4_addr(netif_default)));
	    break;
	    case 8: /* "sgtine" */
	      memcpy(_sgtine, _sgtin, 30);
	      _sgtine[15] = '0';
	      _sgtine[16] = '4';
	      _sgtine[17] = '1';
	      _sgtine[18] = 'D';
  		  printed = sprintf(pcInsert, "%s", _sgtine);
	    break;
	    case 9: /* "sgtinr" */
	      printed = sprintf(pcInsert, "%s", _sgtin);
	    break;
	    case 10: /* "rfw" */
	      printed = sprintf(pcInsert, "%d.%d.%d",  *_rfw, *(_rfw + 1), *(_rfw + 2));
	    break;
	    default:
	      printed = 0;
	      break;
	  }

  return (u16_t)printed;
}
const char * SETTINGS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
  if (iNumParams > 0) {
	  ip4_addr ip;
	  ip4_addr nm;
	  ip4_addr gw;
	  ip.addr = 0;
	  nm.addr = 0;
	  gw.addr = 0;
	  uint8_t useDHCP=0;
	  uint8_t ledBrightness=255;
	  char * hostname = {0};
    if (iIndex==0) {
    for (int i=0; i<iNumParams; i++) {
        if (strcmp(pcParam[i] , "hname")==0)   { hostname = pcValue[i]; }
        if (strcmp(pcParam[i] , "ipaddr")==0)  { ip.addr = ipaddr_addr(pcValue[i]); }
        if (strcmp(pcParam[i] , "snetmsk")==0) { nm.addr = ipaddr_addr(pcValue[i]); }
        if (strcmp(pcParam[i] , "gateway")==0) { gw.addr = ipaddr_addr(pcValue[i]); }
        if (strcmp(pcParam[i] , "usedhcp")==0) { if(strcmp(pcValue[i], "on") ==0) useDHCP=1 ; }
        if (strcmp(pcParam[i] , "ledb")==0) { ledBrightness =  atoi(pcValue[i]); }
    }
    _settings->setLEDBrightness(ledBrightness);
	_settings->setNetworkSettings(hostname, useDHCP, ip, nm, gw);
	_settings->save();
  }

  }

  return "/index.shtml";
}

WebUI::WebUI(Settings *settings) {
    _settings = settings;
}

void WebUI::start(const char * sgtin, const uint8_t * rfw){
	_rfw= rfw;
	_sgtin = sgtin;
  httpd_init();
  http_set_ssi_handler(SETTINGS_SSI_Handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
  CGI_TAB[0] = SETTINGS_CGI;
  http_set_cgi_handlers(CGI_TAB, 1);
}
