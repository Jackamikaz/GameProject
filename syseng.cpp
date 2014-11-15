/*
 * syseng.cpp
 *
 *  Created on: 3 oct. 2014
 *      Author: camille
 */

#include "syseng.hpp"
#include <cstdio>
#include <cstdarg>
#include <SDL.h>

void SysEng::Log(SysEng::LogFlavour flavour ,const char* format, ...) {
	static char buffer[256];

	va_list argptr;
	va_start(argptr, format);
	//vfprintf(stdout, format, argptr);
	vsnprintf(buffer,256,format,argptr);
	va_end(argptr);

	fprintf(stdout,"%s\n",buffer);
	fflush(stdout);

	DataSingleton::Log l;
	l.message = std::string(buffer);
	l.flavour = flavour;
	l.timestamp = SDL_GetTicks() / 1000.0f;
	GetSingleton().logs.push_front(l);
}

void SysEng::Error(const char* format, ...) {
	static char buffer[1024];

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer,1024,format,argptr);
	va_end(argptr);

	MessageBoxA(GetSingleton().mainWindowHandle, buffer, "ERROR", MB_ICONERROR | MB_OK);
	Log(error,buffer);
}

void SysEng::DisplayLogs(BMFont& font, unsigned int typeMask, int maxOnScreen, float logLifetime, float logFadeDuration) {
	float curts = SDL_GetTicks() / 1000.0f;
	SysEngLog logs = GetSingleton().logs;
	SysEngLog::const_iterator it = logs.begin();
	float top = 0.0f;
	if (curts - (*it).timestamp < logFadeDuration) {
		top = -font.GetHeight() * (1.0f - (curts - (*it).timestamp)/logFadeDuration);
	}
	top += 2.0f;
	for(int i=0; i<maxOnScreen && it !=logs.end() && curts - (*it).timestamp < logLifetime; ++it) {
		float timeToDeath = logLifetime - (curts - (*it).timestamp);
		if (timeToDeath > logFadeDuration)
			timeToDeath = logFadeDuration;
		int r = 255;
		int g = 255;
		int b = 255;

		switch((*it).flavour) {
		case debug:
			b = 0;
			break;
		case error:
			g = 0;
			b = 0;
			break;
		default:break;
		}

		font.SetColor(r,g,b,int(timeToDeath*255.0f/logFadeDuration));
		font.Print(10.0f,top,(*it).message.c_str());
		top += font.GetHeight();
		++i;
	}
	font.SetColor(255,255,255,255);
	glColor4ub(255,255,255,255);
}

SysEng::DataSingleton* SysEng::DataSingleton::singleton = 0;

SysEng::DataSingleton::DataSingleton()
:	mainWindowHandle(0)
{
}

SysEng::DataSingleton& SysEng::DataSingleton::GetSingleton() {
	if (!singleton) {
		singleton = new DataSingleton();
	}
	return *singleton;
}

void SysEng::DataSingleton::DestroySingleton() {
	if (singleton) {
		delete singleton;
		singleton = 0;
	}
}
