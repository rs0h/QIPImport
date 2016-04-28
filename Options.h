#pragma once
#include "resource.h"


struct GUN
{
	tstring group;
	tstring name;
	tstring sUIN;
	long	UIN;
};

typedef vector< iterator_range<string::iterator> > find_vector_type;

class COptions  : public CDialogImpl<COptions>, public CUpdateUI<COptions>,
	public CMessageFilter, public CIdleHandler
{
public:
	COptions(void);
	~COptions(void);

	enum { IDD = IDD_OPTIONS };

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(COptions)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(COptions)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_ALL, OnAll)
		COMMAND_ID_HANDLER(IDC_NONE, OnNone)
		COMMAND_HANDLER(IDC_BUTTON2, BN_CLICKED, OnBnClickedOFN)
		COMMAND_HANDLER(IDC_BUTTON1, BN_CLICKED, OnBnClickedButton1)
		COMMAND_HANDLER(IDC_MTOQ, BN_CLICKED, OnBnClickedMtoq)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CListViewCtrl				m_list;
	CStatusBarCtrl				m_status;
	CProgressBarCtrl			m_progress;
	tstring						m_InitialDir;
	tstring						m_Options;
	vector<GUN>					g_gun;	
	tstring OpenFileDlg(LPCTSTR szFilter,LPCTSTR szCaption, LPCTSTR Dir, bool Ask = false);
	LRESULT OnBnClickedOFN(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNone(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMtoq(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
