/*
 * syseng.hpp
 *
 *  Created on: 26 sept. 2014
 *      Author: camille
 */

#ifndef SYSENG_HPP_
#define SYSENG_HPP_

#include <windows.h>
#include <list>
#include <string>
#include <cassert>
#include "bmfont.h"

namespace SysEng {

	enum LogFlavour {
		log		= 0x01,
		debug	= 0x02,
		error	= 0x04,
	};

	void Log(LogFlavour flavour, const char* format, ...);
	void Error(const char* format, ...);
	void DisplayLogs(BMFont& font, unsigned int typeMask = 0xFFFF, int maxOnScreen = 8, float logLifetime = 5.0f, float logFadeDuration = 0.3f);

	class DataSingleton {
	private:
		static DataSingleton* singleton;
		DataSingleton();
	public:
		static DataSingleton& GetSingleton();
		static void DestroySingleton();

		HWND mainWindowHandle;
		struct Log {
			std::string message;
			LogFlavour flavour;
			float timestamp;
		};
		std::list<Log> logs;
	};
	inline DataSingleton& GetSingleton() {
		return DataSingleton::GetSingleton();
	}
	inline void DestroySingleton() {
		DataSingleton::DestroySingleton();
	}
};

typedef std::list<SysEng::DataSingleton::Log> SysEngLog;

#define SEDEBUGLOG(log, ...) SysEng::Log(SysEng::debug, log, ##__VA_ARGS__)

#define MACROSTR2(x) #x
#define MACROSTR(x) MACROSTR2(x)
#define SEASSERT(TEST) if (!(TEST)) { \
	SysEng::Error("Assert \""#TEST"\" failed in file " MACROSTR(__FILE__) " at line " MACROSTR(__LINE__)); \
	assert(0); \
}


#endif /* SYSENG_HPP_ */
