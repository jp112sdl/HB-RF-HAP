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

template <class T>
inline void DPRINT(T str);
template <class T>
inline void DPRINTLN(T str);
template<typename TYPE>
inline void DDECLN(TYPE b);

tSSIHandler SETTINGS_Page_SSI_Handler;

const char * ssi_tags[] = {
  "hname",
  "ipaddr",
  "snetmsk",
  "gateway",
  "usedhcp",
  "ledb"
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
		  printed =  sprintf(pcInsert, "<input value='%s' name='hname'>", hname);
		  break;
	    case 1: /* "ipaddr" */
	      ip = _settings->getLocalIP();
	      printed =  sprintf(pcInsert, "<input value='%s' name='ipaddr'>", ip4addr_ntoa(&ip));
	      break;
	    case 2: /* "snetmask" */
		  ip = _settings->getNetmask();
	      printed =  sprintf(pcInsert, "<input value='%s' name='snetmsk'>", ip4addr_ntoa(&ip));
	      break;
	    case 3: /* "gateway" */
		  ip = _settings->getGateway();
	      printed =  sprintf(pcInsert, "<input value='%s' name='gateway'>", ip4addr_ntoa(&ip));
	      break;
	    case 4: /* "use dhcp" */
	      printed =  sprintf(pcInsert, "<input type='checkbox' name='usedhcp' %s>", (useDHCP == 0) ? "" : "checked" );
	      break;
	    case 5: /* "ledb" */
	      printed = sprintf(pcInsert, "<input type='number' min=0 max=255 value='%d' name='ledb'>", ledbrightness);
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

void WebUI::start(){
  httpd_init();
  http_set_ssi_handler(SETTINGS_SSI_Handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
  CGI_TAB[0] = SETTINGS_CGI;
  http_set_cgi_handlers(CGI_TAB, 1);
}
