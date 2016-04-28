// MyPligin.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "QIPHistory.h"
#include "Options.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

HMODULE h_Module = NULL;

PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"QIP 2005 History import",
	PLUGIN_MAKE_VERSION(0,0,0,3),
	"QIP 2005 History import plugin",
	"Robert Shekhe",
	"roger_h@mail.ru",
	"© 2009 Robert Shekhe",
	"",
	0,		//not transient
	0,		//doesn't replace anything built-in
	// Generate your own unique id for your plugin.
	// Do not use this UUID!
	// Use uuidgen.exe to generate the uuuid
	{0x9B6D47A1, 0xB7B7, 0x40af, { 0xAB, 0xA5, 0x27, 0xD0, 0xD4, 0xA4, 0x23, 0x99}}
};

struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;

	HANDLE HContactFromNumericID(char* pszProtoName, char* pszSetting, DWORD dwID)
	{
		char* szProto;
		HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact != NULL)
		{
			if (DBGetContactSettingDword(hContact, pszProtoName, pszSetting, 0) == dwID)
			{
				szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
				if (szProto != NULL && !strcmp(szProto, pszProtoName))
					return hContact;
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		return INVALID_HANDLE_VALUE;
	}

	static int PluginMenuCommand(WPARAM wParam,LPARAM lParam)
	{
		COptions cop;
		cop.DoModal();
		return 0;
	}

	static int PluginMenuCommandCD(WPARAM wParam,LPARAM lParam)
	{
		HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, (WPARAM)1, 0);

		while(hContact) 
		{
			HANDLE hDBEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0), hDBEventNext;
			while(hDBEvent) {
				hDBEventNext = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hDBEvent, 0);
				CallService(MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)hDBEvent);
				hDBEvent = hDBEventNext;
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		return 0;
	}

	__declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
	{
		return &pluginInfo;
	}

	static const MUUID interfaces[] = {MIID_TESTPLUGIN, MIID_IMPORT,MIID_LAST};
	__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
	{
		return interfaces;
	}

	int __declspec(dllexport) Load(PLUGINLINK *link)
	{
		pluginLink = link;
		mir_getMMI( &mmi );
		mir_getUTFI( &utfi );

		CLISTMENUITEM mi;

		pluginLink=link;
		CreateServiceFunction("QIP Import/MenuCommand",PluginMenuCommand);
		ZeroMemory(&mi,sizeof(mi));
		mi.cbSize=sizeof(mi);
		mi.position=-0x7FFFFFFF;
		mi.flags=0;
		mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		mi.pszName=LPGEN("&QIP Import...");
		mi.pszService="QIP Import/MenuCommand";
		CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

		//CreateServiceFunction("Clear Database/MenuCommand",PluginMenuCommandCD);
		//ZeroMemory(&mi,sizeof(mi));
		//mi.cbSize=sizeof(mi);
		//mi.position=-0x7FFFFFFF;
		//mi.flags=0;
		//mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
		//mi.pszName=LPGEN("&Clear Database...");
		//mi.pszService="Clear Database/MenuCommand";
		//CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
		return 0;
	}

	int __declspec(dllexport) Unload(void)
	{
		return 0;
	}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		h_Module = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		h_Module = NULL;
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif