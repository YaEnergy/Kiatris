// Kiatris.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

// TODO: Reference additional headers your program requires here.

const int DESIGN_WIDTH = 800;
const int DESIGN_HEIGHT = 480;

const std::string VERSION = "1.0.2b";

#ifdef PLATFORM_WEB
const std::string PLATFORM = "Web";
#elif PLATFORM_DESKTOP
const std::string PLATFORM = "Desktop";
#endif

#ifdef WIN32RELEASE
int main();
#endif