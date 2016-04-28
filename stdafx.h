// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>


#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <atlbase.h>
#include <atlapp.h>

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

// TODO: reference additional headers your program requires here

#include <newpluginapi.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_database.h>

#include <m_database.h>
#include <m_clc.h>
#include <m_protocols.h>
#include <m_options.h>
#include <m_message.h>
#include <m_icolib.h>
#include <m_protomod.h>
#include <m_protosvc.h>

#include <vector>
#include <string>
namespace std {
#if defined _UNICODE || defined UNICODE
	typedef wstring tstring;
#else
	typedef string tstring;
#endif
}

using namespace std;

#include <time.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace boost; 

#define ICQOSCPROTONAME "ICQ"


