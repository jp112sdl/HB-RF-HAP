/*
 * web.h
 *
 *  Created on: Sep 14, 2022
 *      Author: jp112sdl
 */

#pragma once
#include <stdint.h>
#include "settings.h"


class WebUI
{
public:
    WebUI(Settings *settings);
    void start();
};
