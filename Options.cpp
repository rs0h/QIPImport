#include "StdAfx.h"
#include "atlctrls.h"
#include "Options.h"
#include "QIPHistory.h"
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <map>

extern HMODULE h_Module;
extern BOOL IsDuplicateEvent(HANDLE hContact, DBEVENTINFO dbei);

#ifdef UNICODE
#define tregex wregex
#define tregex_iterator wsregex_iterator
#else if 
#define tregex regex
#define tregex_iterator sregex_iterator
#endif

struct MSGA
{
	char	*b,*e;
	bool	in;
	time_t	t;
	string	s;
};

HANDLE  GetMessageHandleFromDB(HANDLE hContact, DWORD timestamp)
{
	DBEVENTINFO dbei;
	HANDLE hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0);
	while( hEvent != NULL)
	{
		ZeroMemory(&dbei, sizeof(dbei));
		dbei.cbSize = sizeof(dbei);
		long blbsz = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent, (LPARAM)0);
		dbei.cbBlob = blbsz;
		dbei.pBlob = (PBYTE)malloc(blbsz);
		CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbei);
		//mmg.insert( t_ts(dbei.timestamp , (char*)dbei.pBlob));
		if (timestamp == dbei.timestamp)
			return hEvent;
		free((PBYTE)dbei.pBlob);
		hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hEvent, 0);
	}
	return NULL;
}

HANDLE GetContactFromDB(long UIN)
{
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, (WPARAM)1, 0);
	while(hContact) 
	{
		char * szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		int uin = DBGetContactSettingDword(hContact, szProto, "UIN", 0);
		if (uin == UIN)
			return hContact;
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	return INVALID_HANDLE_VALUE;
}


COptions::COptions(void)
{
}

COptions::~COptions(void)
{
}
tstring COptions::OpenFileDlg(LPCTSTR szFilter,LPCTSTR szCaption, LPCTSTR Dir, bool Ask)
{
	tstring outstr;
	OPENFILENAME of;
	TCHAR buffer[MAX_PATH]={_T('\0')};
	//TCHAR szInitialDir[MAX_PATH]={_T('\0')};

	TCHAR szCustomFilter[MAX_PATH];
	memset(szCustomFilter,0,sizeof(szCustomFilter));
	_tcscpy(szCustomFilter,szFilter);
	for(TCHAR* pch=szCustomFilter;*pch;pch++)
		if(*pch==_T('|')) *pch=_T('\0');

	of.lStructSize       = sizeof (OPENFILENAME);
	of.hwndOwner         = m_hWnd;
	of.hInstance         = NULL;
	of.lpstrFilter       = szCustomFilter;
	of.lpstrCustomFilter = NULL;
	of.nMaxCustFilter    = 0;
	of.nFilterIndex      = 0;
	of.lpstrFile         = buffer;
	of.nMaxFile          = MAX_PATH;
	of.lpstrFileTitle    = NULL;
	of.nMaxFileTitle     = 0;
	of.lpstrInitialDir   = Dir;
	of.lpstrTitle        = szCaption;
	of.Flags		     = (Ask == true)?OFN_OVERWRITEPROMPT:0;
	of.Flags             |= OFN_EXPLORER|OFN_HIDEREADONLY;
	of.nFileOffset       = 0;
	of.nFileExtension    = 0;
	of.lpstrDefExt       = _T("sql");
	of.lCustData         = 0;
	of.lpfnHook          = NULL;
	of.lpTemplateName    = NULL;
	if(!GetOpenFileName (&of))
		//if(!GetSaveFileName (&of))
		return outstr;

	outstr = buffer;
	return outstr; 
}

BOOL COptions::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL COptions::OnIdle()
{
	return FALSE;
}
LRESULT COptions::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_list.Attach(GetDlgItem(IDC_LIST2));
	ListView_SetExtendedListViewStyle(m_list.m_hWnd, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);
	m_list.AddColumn(_T("Name"), 1);
	m_list.AddColumn(_T("UIN"), 2);
	m_list.AddColumn(_T("QIP Msgs"), 3);
	m_list.AddColumn(_T("Miranda Msgs"), 4);
	m_status.Create( m_hWnd, NULL, _T(""), WS_CHILD|WS_VISIBLE);
	RECT rc;
	m_status.GetWindowRect(&rc);
	int partsw[2];
	partsw[0] = 350;
	partsw[1] = rc.right-rc.left;
	m_status.SetParts( 2, partsw);
	m_status.GetRect( 1, &rc);
	m_progress.Create( m_status.m_hWnd, rc, _T(""), WS_CHILD|WS_VISIBLE|PBS_SMOOTH);
	m_progress.SetRange32( 0, 100);
	m_progress.SetStep(1);

	TCHAR mfn[_MAX_PATH] = {0};
	GetModuleFileName(h_Module,mfn,_MAX_PATH);
	TCHAR tDrive[_MAX_DRIVE], tDir[_MAX_DIR], tFilename[_MAX_FNAME], tExt[_MAX_EXT];
	_tsplitpath( mfn,tDrive,tDir, tFilename, tExt);
	_tmakepath( mfn, tDrive, tDir, tFilename, _T("ini"));
	m_Options = mfn;

	GetPrivateProfileString( _T("QIPImport"), _T("QIPRootFolder"), NULL, mfn, _MAX_PATH, m_Options.c_str());
	m_InitialDir = mfn;
	return 0;
}

LRESULT COptions::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT COptions::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for (long item = 0; item < m_list.GetItemCount(); item++)
	{
		m_progress.SetPos(0);
		if (!m_list.GetCheckState(item))
			continue;
		TCHAR sUIN[256];
		m_list.GetItemText(item,1,sUIN, 256);
		long UIN = _ttoi(sUIN);
		bool bFound = false;
		long j = 0;
		for (j = 0; j < g_gun.size(); j++)
		{
			if (UIN == g_gun[j].UIN)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			continue;
		char * block = NULL;
		vector<MSGA> msgl;
		tstring s = m_InitialDir + _T("\\History\\") + g_gun[j].sUIN + _T(".txt");

		std::ifstream in(s.c_str(),ifstream::binary);
		long msgsq = 0;
		if (in)
		{
			in.seekg(0, std::ios::end);
			unsigned int len = in.tellg();      // read file pointer
			in.seekg(0, std::ios::beg);
			block = new char[len];
			in.read(block, len);
			regex reg("(-------------------------------------->-|--------------------------------------<-)");
			regex reg2("\\w+\\s+\\((\\d{1,2}):(\\d{1,2}):(\\d{1,2})\\s+(\\d{1,2})/(\\d{1,2})/((?:\\d{4}|\\d{2}))\\)");
			const char *from = block, *to = block+len;

			boost::cmatch m;
			boost::match_flag_type flags = boost::match_default;
			while( boost::regex_search(from, to, m, reg, flags) ) 
			{
				MSGA msg;
				const char * lastf = from;
				if (msgl.size())
				{
					msgl[msgl.size() - 1].e = (char*)m[0].first;
				}
				from = m[0].second, ++msgsq;
				string ms;
				ms.assign(m[1].first, m[1].second);
				if (ms == "-------------------------------------->-")
					msg.in = false;
				else if (ms == "--------------------------------------<-")
					msg.in = true;
				if (boost::regex_search(from, to, m, reg2, flags))
				{
					time_t t;
					from = m[0].second;
					struct tm  when = {0};
					when.tm_hour = atoi(string(m[1].first, m[1].second).c_str());
					when.tm_min = atoi(string(m[2].first, m[2].second).c_str());
					when.tm_sec = atoi(string(m[3].first, m[3].second).c_str());
					when.tm_mday = atoi(string(m[4].first, m[4].second).c_str());
					when.tm_mon = atoi(string(m[5].first, m[5].second).c_str());
					when.tm_year = atoi(string(m[6].first, m[6].second).c_str())-1900;
					when.tm_mon--;
					t = mktime(&when);
					char tmpbuf[128];
					_strtime_s( tmpbuf, 128 );
					ctime_s(tmpbuf, 26, &t);
					msg.t = t;
					msg.b = (char*)from;
				}
				msgl.push_back(msg);				
			}
			for (long i = 0; i < msgl.size(); i++)
			{

				if (i<msgl.size()-1)
					msgl[i].s.assign(msgl[i].b, msgl[i].e);
				else
					msgl[i].s.assign(msgl[i].b, to);
				//replace_all( msgl[i].s, "\n","\r\n");
			}
		}
		DBEVENTINFO dbei;
		long msg_cnt = 0;
		HANDLE hContact = GetContactFromDB(g_gun[j].UIN);
		if (hContact == INVALID_HANDLE_VALUE)
		{
			delete block;
			return 0;
		}

		map<time_t, string>	mmg;
		map<time_t, HANDLE>	mmets;
		typedef pair<time_t, string> t_ts;
		typedef pair<time_t, HANDLE> t_hdls;

		HANDLE hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST, (WPARAM)hContact, 0);
		while( hEvent != NULL)
		{
			ZeroMemory(&dbei, sizeof(dbei));
			dbei.cbSize = sizeof(dbei);

			long blbsz = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent, (LPARAM)0);
			dbei.cbBlob = blbsz;
			dbei.pBlob = (PBYTE)malloc(blbsz);
			CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbei);
			mmg.insert( t_ts(dbei.timestamp, (char*)dbei.pBlob));
			mmets.insert( t_hdls(dbei.timestamp, hEvent));
			free((PBYTE)dbei.pBlob);
			hEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT, (WPARAM)hEvent, 0);
		}

		long prc = 0;
		for (long i = 0; i < msgl.size(); i++)
		{
			ZeroMemory(&dbei, sizeof(dbei));
			dbei.cbSize = sizeof(dbei);
			dbei.eventType = EVENTTYPE_MESSAGE;
			dbei.flags =  msgl[i].in == 0 ? DBEF_SENT : DBEF_READ;
			dbei.szModule = ICQOSCPROTONAME;
			// Convert timestamp
			dbei.timestamp = msgl[i].t;
			dbei.cbBlob = msgl[i].s.size();
			dbei.pBlob = (PBYTE)malloc(msgl[i].s.size());
			CopyMemory(dbei.pBlob, msgl[i].s.c_str(), dbei.cbBlob);
			dbei.pBlob[dbei.cbBlob - 1] = 0;
			if (mmg.find(msgl[i].t) == mmg.end())
			{
				CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&dbei);
				msg_cnt++;
			}

			//else
			//{
			//	map<time_t, HANDLE>::iterator hh = mmets.find(msgl[i].t);
			//	HANDLE hdl = (*hh).second;
			//	//HANDLE hh = GetMessageHandleFromDB(hContact, )
			//	LRESULT r = CallService(MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)hdl);
			//	msg_cnt++;
			//}

			free((PBYTE)dbei.pBlob);
			if (prc < (long)i*100/msgl.size()+1)
			{
				prc = (long)i*100/msgl.size()+1;
				m_progress.SetPos(prc);
				tstring stext = g_gun[j].name + _T(": ") + boost::lexical_cast<tstring>(msg_cnt) + _T(" of ") + boost::lexical_cast<tstring>((long)msgl.size()) + _T(" messages imported");
				m_status.SetText(0, stext.c_str());
				UpdateWindow();	
			}
		}

		delete block;
		long msgsm = CallService(MS_DB_EVENT_GETCOUNT, (WPARAM)hContact, (LPARAM)0);
		s = boost::lexical_cast<tstring>(msgsm);
		m_list.AddItem(item, 3,s.c_str());
		m_list.SelectItem(item);
		m_list.SetCheckState(item,FALSE);
		tstring stext = g_gun[j].name + _T(": ") + boost::lexical_cast<tstring>(msg_cnt) + _T(" of ") + boost::lexical_cast<tstring>((long)msgl.size()) + _T(" messages imported");
		m_status.SetText(0, stext.c_str());
		UpdateWindow();	
	}
	m_progress.SetPos(0);
	if (m_list.GetItemCount())
		m_list.SelectItem(0);
	UpdateWindow();	
	return 0;
}
LRESULT COptions::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
tstring tfgets(FILE * f)
{
	tstring s;
	char bb[1024];
	fgets(bb, 1024, f);
#ifdef UNICODE
	TCHAR bb2[2048];
	long sz = MultiByteToWideChar(CP_ACP, 0, bb, -1, 0, 0);
	if (sz && sz < 1024)
		MultiByteToWideChar(CP_ACP, 0, bb, -1, bb2, sz);
	s = bb2;
#else
	s = bb;
#endif
	return s;
}

LRESULT COptions::OnBnClickedOFN(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	tstring path = OpenFileDlg(_T("*.cl|*.cl|"), _T("open QIP Contact list"),m_InitialDir.c_str());
	TCHAR mfn[_MAX_PATH] = {0};
	_tcscpy(mfn, path.c_str());
	//GetModuleFileName(h_Module,mfn,_MAX_PATH);
	TCHAR tDrive[_MAX_DRIVE], tDir[_MAX_DIR], tFilename[_MAX_FNAME], tExt[_MAX_EXT];
	_tsplitpath( mfn,tDrive,tDir, tFilename, tExt);
	_tmakepath( mfn, tDrive, tDir, _T(""), _T(""));
	m_InitialDir = mfn;
	WritePrivateProfileString( _T("QIPImport"), _T("QIPRootFolder"), mfn, m_Options.c_str());

	TCHAR fn[1024];//, path[1024];
	//strcpy(path, "C:/Users/Robert/AppData/Local/VirtualStore/Program Files (x86)/QIP/Users/123795137/");
	_tcscpy(fn, path.c_str());
	//strcat( fn,"123795137.cl");
	FILE * f = _tfopen(fn,_T("r"));
	if (!f) 
		return 0;
	g_gun.clear();
	while(!feof(f))
	{
		tstring s = tfgets(f);
		typedef vector< iterator_range<tstring::iterator> > find_vector_type;
		find_vector_type fv;
		split( fv, s, is_any_of(";") );
		if (fv.size()>2)
		{
			GUN g;
			g.group = copy_range<tstring>(fv[0]);
			g.name = copy_range<tstring>(fv[2]);
			g.sUIN = copy_range<tstring>(fv[1]);
			g.UIN = _tstoi(g.sUIN.c_str());
			g_gun.push_back(g);
		}
	}
	fclose(f);

	m_list.DeleteAllItems();
	tstring ts;

	for (long i = 0; i < g_gun.size(); i++)
	{
		long item = m_list.GetItemCount();
		ts.clear();ts.assign(g_gun[i].name.begin(), g_gun[i].name.end()); 
		m_list.AddItem(item, 0,ts.c_str());
		ts.clear();ts.assign(g_gun[i].sUIN.begin(), g_gun[i].sUIN.end()); 
		m_list.AddItem(item, 1,ts.c_str());
		UpdateWindow();
		tstring s = m_InitialDir + _T("\\History\\") + g_gun[i].sUIN + _T(".txt");

		std::ifstream in(s.c_str());
		long msgsq = 0;
		if (in)
		{
			in.seekg(0, std::ios::end);
			unsigned int len = in.tellg();      // read file pointer
			in.seekg(0, std::ios::beg);
			char * block = new char[len];
			in.read(block, len);
			regex reg("-------------------------------------->-|--------------------------------------<-");
			//regex reg("[^\\(\\n]\\s\\((\\d{1,2}):(\\d{1,2}):(\\d{1,2})\\s+(\\d{1,2})/(\\d{1,2})/((?:\\d{4}|\\d{2}))\\)");
			const char *from = block, *to = block+len;

			boost::cmatch m;
			boost::match_flag_type flags = boost::match_default;
			while( boost::regex_search(from, to, m, reg, flags) ) {
				from = m[0].second, ++msgsq;
			}
			delete block;
		}

		HANDLE hContact = GetContactFromDB(g_gun[i].UIN);

		if (hContact == INVALID_HANDLE_VALUE)
		{
			MessageBox(_T("No contacts found You must login at least once to acquire Your contact list from ICQ Server"));
			return 0;
		}
		long msgsm = CallService(MS_DB_EVENT_GETCOUNT, (WPARAM)hContact, (LPARAM)0);
		s = boost::lexical_cast<tstring>(msgsm);
		m_list.AddItem(item, 3,s.c_str());


		s = boost::lexical_cast<tstring>(msgsq);
		m_list.AddItem(item, 2,s.c_str());
		m_progress.SetPos((long)i*100/g_gun.size()+1);
		UpdateWindow();	
		m_list.SetColumnWidth(0,LVSCW_AUTOSIZE);
		m_list.SetColumnWidth(1,LVSCW_AUTOSIZE);
		m_list.SetColumnWidth(2,LVSCW_AUTOSIZE_USEHEADER);
		m_list.SetColumnWidth(3,LVSCW_AUTOSIZE_USEHEADER);
		m_list.SelectItem(item);
		UpdateWindow();	
	}
	m_progress.SetPos(0);
	m_status.SetText(0,_T("Ready"),0);
	if (m_list.GetItemCount())
		m_list.SelectItem(0);

	UpdateWindow();	
	return 0;
}


LRESULT COptions::OnAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for (long i = 0; i < m_list.GetItemCount(); i++)
	{
		m_list.SetCheckState(i, TRUE);
	}
	return 0;
}

LRESULT COptions::OnNone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	for (long i = 0; i < m_list.GetItemCount(); i++)
	{
		m_list.SetCheckState(i, FALSE);
	}
	return 0;
}
LRESULT COptions::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	return 0;
}

LRESULT COptions::OnBnClickedMtoq(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: Add your control notification handler code here

	return 0;
}