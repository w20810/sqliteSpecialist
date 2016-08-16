#include "stdafx.h"
#include "uieditdisablermenu.h"

#include <regex>
#include <atlstr.h>

namespace UiLib
{

	class CEditDisableRMenuWnd : public CWindowWnd
	{
	public:
		CEditDisableRMenuWnd();

		void Init(CEditDisableRMenuUI* pOwner);
		RECT CalPos();

		LPCTSTR GetWindowClassName() const;
		LPCTSTR GetSuperClassName() const;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	protected:
		CEditDisableRMenuUI* m_pOwner;
		HBRUSH m_hBkBrush;
		bool m_bInit;
	};


	CEditDisableRMenuWnd::CEditDisableRMenuWnd() : m_pOwner(NULL), m_hBkBrush(NULL), m_bInit(false)
	{
	}

	void CEditDisableRMenuWnd::Init(CEditDisableRMenuUI* pOwner)
	{
		m_pOwner = pOwner;
		RECT rcPos = CalPos();
		UINT uStyle = 0;
		if(m_pOwner->GetManager()->IsBackgroundTransparent())
		{
			uStyle = WS_POPUP | ES_AUTOHSCROLL | WS_VISIBLE;
			RECT rcWnd={0};
			::GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWnd);
			rcPos.left += rcWnd.left;
			rcPos.right += rcWnd.left;
			rcPos.top += rcWnd.top;
			rcPos.bottom += rcWnd.top;
		}
		else
		{
			uStyle = WS_CHILD | ES_AUTOHSCROLL;
		}	
		if( m_pOwner->IsPasswordMode() ) uStyle |= ES_PASSWORD;
		Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
		SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->hFont, TRUE);
		Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
		if( m_pOwner->IsPasswordMode() ) Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());
		Edit_SetText(m_hWnd, m_pOwner->GetText());
		Edit_SetModify(m_hWnd, FALSE);
		SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
		Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
		Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);
		//Styls
		LONG styleValue = ::GetWindowLong(m_hWnd, GWL_STYLE);
		styleValue |= pOwner->GetWindowStyls();
		::SetWindowLong(GetHWND(), GWL_STYLE, styleValue);
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
		::SetFocus(m_hWnd);
		m_bInit = true;
	}

	RECT CEditDisableRMenuWnd::CalPos()
	{
		CDuiRect rcPos = m_pOwner->GetPos();
		RECT rcInset = m_pOwner->GetTextPadding();
		rcPos.left += rcInset.left;
		rcPos.top += rcInset.top;
		rcPos.right -= rcInset.right;
		rcPos.bottom -= rcInset.bottom;
		LONG lEditHeight = m_pOwner->GetManager()->GetFontInfo(m_pOwner->GetFont())->tm.tmHeight;
		if( lEditHeight < rcPos.GetHeight() ) {
			rcPos.top += (rcPos.GetHeight() - lEditHeight) / 2;
			rcPos.bottom = rcPos.top + lEditHeight;
		}
		return rcPos;
	}

	LPCTSTR CEditDisableRMenuWnd::GetWindowClassName() const
	{
		return _T("EditDisableRMenuWnd");
	}

	LPCTSTR CEditDisableRMenuWnd::GetSuperClassName() const
	{
		return WC_EDIT;
	}

	void CEditDisableRMenuWnd::OnFinalMessage(HWND /*hWnd*/)
	{
		m_pOwner->Invalidate();
		// Clear reference and die
		if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
		m_pOwner->m_pWindow = NULL;
		delete this;
	}

	LRESULT CEditDisableRMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;

		if(m_hWnd && uMsg == WM_SETFOCUS )
		{
			m_pOwner->m_RegluarSrcText = m_pOwner->GetText();
			m_pOwner->GetManager()->SetTimer(m_pOwner,1650,m_pOwner->GetTimerDelay());
		}

		if( uMsg == WM_KILLFOCUS )
		{
			m_pOwner->GetManager()->KillTimer(m_pOwner);

			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		}
		else if( uMsg == OCM_COMMAND ) {
			if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
			else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
				RECT rcClient;
				::GetClientRect(m_hWnd, &rcClient);
				::InvalidateRect(m_hWnd, &rcClient, FALSE);
			}
		}
		else if( uMsg == WM_KEYDOWN ) {
			TEventUI event = { 0 };
			event.Type = UIEVENT_KEYDOWN;
			event.chKey = (TCHAR)wParam;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			m_pOwner->Event(event);

			if (TCHAR(wParam) == VK_RETURN )
				m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_RETURN);

		}

		if( uMsg == OCM__BASE + WM_CTLCOLOREDIT  || uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) {
			if( m_pOwner->GetNativeEditBkColor() == 0xFFFFFFFF && !m_pOwner->IsReadOnly() ) return NULL;
			::SetBkMode((HDC)wParam, TRANSPARENT);

			DWORD dwTextColor;
			if (m_pOwner->GetNativeEditTextColor() != 0x000000)
				dwTextColor = m_pOwner->GetNativeEditTextColor();
			else
				dwTextColor = m_pOwner->GetTextColor();

			::SetTextColor((HDC)wParam, RGB(GetBValue(dwTextColor),GetGValue(dwTextColor),GetRValue(dwTextColor)));
			if( m_hBkBrush == NULL ) {
				DWORD clrColor = !m_pOwner->IsReadOnly()?m_pOwner->GetNativeEditBkColor():m_pOwner->GetDisabledBkColor();
				if(clrColor != 0)
					m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor)));
				else 
					return (LRESULT)::HBRUSH(GetStockObject(HOLLOW_BRUSH));
			}
			return (LRESULT)m_hBkBrush;
		}
		else if (uMsg == WM_CONTEXTMENU) // Ω˚”√”“º¸≤Àµ•
		{
			bHandled = TRUE;
		}
		else
		{
			bHandled = FALSE;
		}
		if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		return lRes;
	}

	LRESULT CEditDisableRMenuWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		//m_pOwner->m_pWindow = NULL;  //redrain–ﬁ∏¥bug
		PostMessage(WM_CLOSE);
		return lRes;
	}

	LRESULT CEditDisableRMenuWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if( !m_bInit ) return 0;
		if( m_pOwner == NULL ) return 0;
		// Copy text back
		int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
		LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
		ASSERT(pstr);
		if( pstr == NULL ) return 0;
		::GetWindowText(m_hWnd, pstr, cchLen);
		m_pOwner->m_sText = pstr;
		m_pOwner->GetManager()->SendNotify(m_pOwner, DUI_MSGTYPE_TEXTCHANGED);
		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	CEditDisableRMenuUI::CEditDisableRMenuUI() : m_pWindow(NULL), m_uMaxChar(-1), m_bReadOnly(false), 
		m_bPasswordMode(false), m_cPasswordChar(_T('*')), m_uButtonState(0), 
		m_dwEditbkColor(0xFFFFFFFF), m_dwEditTextColor(0x00000000), m_iWindowStyls(0),m_dwTipValueColor(0xFFBAC0C5)
	{
		SetBorderSize(1);
		SetBorderColor(0xFFBAC0C5);
		SetTextPadding(CDuiRect(4, 3, 4, 3));
		SetBkColor(0xFFFFFFFF);
	}

	LPCTSTR CEditDisableRMenuUI::GetClass() const
	{
		return _T("EditDisableRMenuUI");
	}

	LPVOID CEditDisableRMenuUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_EDITDISABLERMENU) == 0 ) return static_cast<CEditDisableRMenuUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CEditDisableRMenuUI::GetControlFlags() const
	{
		if( !IsEnabled() ) return CControlUI::GetControlFlags();

		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CEditDisableRMenuUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}
		if( event.Type == UIEVENT_TIMER && event.pSender == this && m_pWindow )
			return OnTimer(event.wParam );
		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			return;
		}
		if( event.Type == UIEVENT_WINDOWSIZE )
		{
			if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if( m_pWindow != NULL ) return;
		}
		if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
		{
			if( m_pWindow ) return;
			m_pWindow = new CEditDisableRMenuWnd();
			ASSERT(m_pWindow);
			m_pWindow->Init(this);
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
		{
			if(m_RegularCheckStr.GetLength() > 0)
			{
				if(!MatchRegular(true)){
					GetManager()->SendNotify(this,DUI_MSGTYPE_EDITREGEX,IDNO);
					event.Type = UIEVENT_SETFOCUS;
					DoEvent(event);
					return;
				}
				else
					GetManager()->SendNotify(this,DUI_MSGTYPE_EDITREGEX,IDYES);
			}
			SetInternVisible(false);
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
		{
			if( IsEnabled() ) {
				GetManager()->ReleaseCapture();
				if( IsFocused() && m_pWindow == NULL )
				{
					m_pWindow = new CEditDisableRMenuWnd();
					ASSERT(m_pWindow);
					m_pWindow->Init(this);

					if( PtInRect(&m_rcItem, event.ptMouse) )
					{
						int nSize = GetWindowTextLength(*m_pWindow);
						if( nSize == 0 )
							nSize = 1;

						Edit_SetSel(*m_pWindow, 0, nSize);
					}
				}
				else if( m_pWindow != NULL )
				{
#if 1
					int nSize = GetWindowTextLength(*m_pWindow);
					if( nSize == 0 )
						nSize = 1;

					Edit_SetSel(*m_pWindow, 0, nSize);
#else
					POINT pt = event.ptMouse;
					pt.x -= m_rcItem.left + m_rcTextPadding.left;
					pt.y -= m_rcItem.top + m_rcTextPadding.top;
					::SendMessage(*m_pWindow, WM_LBUTTONDOWN, event.wParam, MAKELPARAM(pt.x, pt.y));
#endif
				}
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CLabelUI::DoEvent(event);
	}

	void CEditDisableRMenuUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	void CEditDisableRMenuUI::SetText(LPCTSTR pstrText)
	{
		m_sText = pstrText;
		if( m_pWindow != NULL ) Edit_SetText(*m_pWindow, m_sText);
		Invalidate();
	}

	void CEditDisableRMenuUI::SetMaxChar(int uMax)
	{
		m_uMaxChar = uMax;
		if( m_pWindow != NULL ) Edit_LimitText(*m_pWindow, m_uMaxChar);
	}

	int CEditDisableRMenuUI::GetMaxChar()
	{
		return m_uMaxChar;
	}

	void CEditDisableRMenuUI::SetReadOnly(bool bReadOnly)
	{
		if( m_bReadOnly == bReadOnly ) return;

		m_bReadOnly = bReadOnly;
		if( m_pWindow != NULL ) Edit_SetReadOnly(*m_pWindow, m_bReadOnly);
		Invalidate();
	}

	bool CEditDisableRMenuUI::IsReadOnly() const
	{
		return m_bReadOnly;
	}

	void CEditDisableRMenuUI::SetNumberOnly(bool bNumberOnly)
	{
		if( bNumberOnly )
		{
			m_iWindowStyls |= ES_NUMBER;
		}
		else
		{
			m_iWindowStyls &= ~ES_NUMBER;
		}
	}

	bool CEditDisableRMenuUI::IsNumberOnly() const
	{
		return m_iWindowStyls&ES_NUMBER ? true:false;
	}

	int CEditDisableRMenuUI::GetWindowStyls() const 
	{
		return m_iWindowStyls;
	}

	void CEditDisableRMenuUI::SetPasswordMode(bool bPasswordMode)
	{
		if( m_bPasswordMode == bPasswordMode ) return;
		m_bPasswordMode = bPasswordMode;
		Invalidate();
	}

	bool CEditDisableRMenuUI::IsPasswordMode() const
	{
		return m_bPasswordMode;
	}

	void CEditDisableRMenuUI::SetPasswordChar(TCHAR cPasswordChar)
	{
		if( m_cPasswordChar == cPasswordChar ) return;
		m_cPasswordChar = cPasswordChar;
		if( m_pWindow != NULL ) Edit_SetPasswordChar(*m_pWindow, m_cPasswordChar);
		Invalidate();
	}

	TCHAR CEditDisableRMenuUI::GetPasswordChar() const
	{
		return m_cPasswordChar;
	}

	LPCTSTR CEditDisableRMenuUI::GetNormalImage()
	{
		return m_sNormalImage;
	}

	void CEditDisableRMenuUI::SetNormalImage(LPCTSTR pStrImage)
	{
		m_sNormalImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditDisableRMenuUI::GetHotImage()
	{
		return m_sHotImage;
	}

	void CEditDisableRMenuUI::SetHotImage(LPCTSTR pStrImage)
	{
		m_sHotImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditDisableRMenuUI::GetFocusedImage()
	{
		return m_sFocusedImage;
	}

	void CEditDisableRMenuUI::SetFocusedImage(LPCTSTR pStrImage)
	{
		m_sFocusedImage = pStrImage;
		Invalidate();
	}

	LPCTSTR CEditDisableRMenuUI::GetDisabledImage()
	{
		return m_sDisabledImage;
	}

	void CEditDisableRMenuUI::SetDisabledImage(LPCTSTR pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	void CEditDisableRMenuUI::SetNativeEditBkColor(DWORD dwBkColor)
	{
		m_dwEditbkColor = dwBkColor;
		Invalidate();
	}

	DWORD CEditDisableRMenuUI::GetNativeEditBkColor() const
	{
		return m_dwEditbkColor;
	}

	void CEditDisableRMenuUI::SetNativeEditTextColor( LPCTSTR pStrColor )
	{
		if( *pStrColor == _T('#')) pStrColor = ::CharNext(pStrColor);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pStrColor, &pstr, 16);

		m_dwEditTextColor = clrColor;
	}

	DWORD CEditDisableRMenuUI::GetNativeEditTextColor() const
	{
		return m_dwEditTextColor;
	}

	void CEditDisableRMenuUI::SetSel(long nStartChar, long nEndChar)
	{
		if( m_pWindow != NULL ) Edit_SetSel(*m_pWindow, nStartChar,nEndChar);
	}

	void CEditDisableRMenuUI::SetSelAll()
	{
		SetSel(0,-1);
	}

	void CEditDisableRMenuUI::SetReplaceSel(LPCTSTR lpszReplace)
	{
		if( m_pWindow != NULL ) Edit_ReplaceSel(*m_pWindow, lpszReplace);
	}

	bool CEditDisableRMenuUI::MatchRegular(bool isShowMsg/* = true*/)
	{
		if(!m_RegularCheckStr.GetLength())
			return true;

		try
		{
#ifndef _UNICODE
			wchar_t* mSrcRegularCheck = new wchar_t[_tclen(GetRegularCheck())*2+1]();
			wsprintfW(mSrcRegularCheck,L"%s",GetRegularCheck());
			std::tr1::wregex regExpress(mSrcRegularCheck);

			wchar_t* mSrcVal = new wchar_t[GetText().GetLength()*2+1]();
			wsprintfW(mSrcVal,L"%s",GetText().GetData());
			std::wstring mSrcText = mSrcVal;

			delete mSrcRegularCheck;
			mSrcRegularCheck = NULL;
			delete mSrcVal;
			mSrcVal = NULL;

			if(!regex_match(mSrcText,regExpress))
				goto MatchFailed;
			else
				return true;
#else
			char* mSrcRegularCheck = new char[strlen(CW2A(GetRegularCheck()))*2+1]();
			sprintf(mSrcRegularCheck,"%s",CW2A(GetRegularCheck()));
			std::tr1::regex regExpress(mSrcRegularCheck);

			char* mSrcVal = new char[GetText().GetLength()*2+1]();
			sprintf(mSrcVal,"%s",CW2A(GetText().GetData()));
			std::string mSrcText = mSrcVal;

			delete m_pWindow;
			mSrcRegularCheck = NULL;
			delete mSrcVal;
			mSrcVal = NULL;

			if(!regex_match(mSrcText,regExpress))
				goto MatchFailed;
			else
				return true;
#endif
		}
		catch(...)
		{
			goto MatchFailed;
		}

MatchFailed:
		{
			if(m_RegularTipStr.GetLength() > 0 && isShowMsg)
				MessageBox(GetManager()->GetPaintWindow(),m_RegularTipStr.GetData(),NULL,MB_OK);

			SetText(m_RegluarSrcText.GetData());
			return false;
		}
	}


	void CEditDisableRMenuUI::SetRegularCheck( LPCTSTR pRegularCheckStr )
	{
		m_RegularCheckStr = pRegularCheckStr;
		Invalidate();
	}

	LPCTSTR CEditDisableRMenuUI::GetRegularCheck()
	{
		return m_RegularCheckStr;
	}

	void CEditDisableRMenuUI::SetRegularTip( LPCTSTR pRegularTipStr )
	{
		m_RegularTipStr = pRegularTipStr;
		Invalidate();
	}

	LPCTSTR CEditDisableRMenuUI::GetRegularTip()
	{
		return m_RegularTipStr;
	}

	void CEditDisableRMenuUI::SetMatchCase( bool bMatchCase /*= false*/ )
	{
		m_bMatchCase = bMatchCase;
	}

	bool CEditDisableRMenuUI::GetMatchCase()
	{
		return m_bMatchCase;
	}

	void CEditDisableRMenuUI::SetTipValue( LPCTSTR pStrTipValue )
	{
		m_sTipValue	= pStrTipValue;
	}

	LPCTSTR CEditDisableRMenuUI::GetTipValue()
	{
		return m_sTipValue.GetData();
	}

	void CEditDisableRMenuUI::SetTipValueColor( LPCTSTR pStrColor )
	{
		if( *pStrColor == _T('#')) pStrColor = ::CharNext(pStrColor);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pStrColor, &pstr, 16);

		m_dwTipValueColor = clrColor;
	}

	DWORD CEditDisableRMenuUI::GetTipValueColor()
	{
		return m_dwTipValueColor;
	}

	void CEditDisableRMenuUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		if( m_pWindow != NULL ) {
			RECT rcPos = m_pWindow->CalPos();
			::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left, 
				rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE);        
		}
	}

	void CEditDisableRMenuUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	void CEditDisableRMenuUI::SetInternVisible(bool bVisible)
	{
		if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}

	SIZE CEditDisableRMenuUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 6);
		return CControlUI::EstimateSize(szAvailable);
	}

	void CEditDisableRMenuUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("readonly")) == 0 ) SetReadOnly(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("numberonly")) == 0 ) SetNumberOnly(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("password")) == 0 ) SetPasswordMode(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("maxchar")) == 0 ) SetMaxChar(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
		else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
		else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
		else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
		else if( _tcscmp(pstrName, _T("tipvalue")) == 0 ) SetTipValue(pstrValue);
		else if( _tcscmp(pstrName, _T("tipvaluecolor")) == 0 ) SetTipValueColor(pstrValue);
		else if( _tcscmp(pstrName, _T("enabletimer")) == 0) SetEnableTimer(_tcscmp(pstrValue,_T("true")) == 0);
		else if( _tcscmp(pstrName, _T("timerdelay")) == 0) SetTimerDelay(_tcstoul(pstrValue,NULL,10));
		else if( _tcscmp(pstrName, _T("regularcheck")) == 0) SetRegularCheck(pstrValue);
		else if( _tcscmp(pstrName, _T("regulartip")) == 0) SetRegularTip(pstrValue);
		else if( _tcscmp(pstrName, _T("nativetextcolor")) == 0 ) SetNativeEditTextColor(pstrValue);
		else if( _tcscmp(pstrName, _T("nativebkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetNativeEditBkColor(clrColor);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CEditDisableRMenuUI::PaintBkColor( HDC hDC )
	{
		if(!IsEnabled() || IsReadOnly())
			CRenderEngine::DrawColor(hDC, m_rcItem, GetAdjustColor(m_dwDisabledBkColor));
		else 
			CLabelUI::PaintBkColor(hDC);
	}

	void CEditDisableRMenuUI::PaintStatusImage(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusedImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHotImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
				else return;
			}
		}

		if( !m_sNormalImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
			else return;
		}
	}

	void CEditDisableRMenuUI::PaintText(HDC hDC)
	{
		DWORD mCurTextColor = m_dwTextColor;

		if( m_dwTextColor == 0 ) mCurTextColor = m_dwTextColor = m_pManager->GetDefaultFontColor();		
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		CDuiString sText;
		if(GetText() == m_sTipValue || GetText() == _T(""))	
		{
			mCurTextColor = m_dwTipValueColor;
			sText = m_sTipValue;			
		}
		else
		{
			sText = m_sText;

			if( m_bPasswordMode ) {
				sText.Empty();
				LPCTSTR p = m_sText.GetData();
				while( *p != _T('\0') ) {
					sText += m_cPasswordChar;
					p = ::CharNext(p);
				}
			}
		}

		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;
		if( IsEnabled() ) {
			CRenderEngine::DrawText(hDC, m_pManager, rc, sText, mCurTextColor, \
				m_iFont, DT_SINGLELINE | m_uTextStyle);
		}
		else {
			CRenderEngine::DrawText(hDC, m_pManager, rc, sText, m_dwDisabledTextColor, \
				m_iFont, DT_SINGLELINE | m_uTextStyle);
		}
	}

	void CEditDisableRMenuUI::SetEnableTimer( bool bEnableTime )
	{
		m_bEnableTime = bEnableTime;
	}

	bool CEditDisableRMenuUI::GetEnableTimer()
	{
		return m_bEnableTime;
	}

	void CEditDisableRMenuUI::SetTimerDelay( UINT nDelay )
	{
		m_uDelay = nDelay;

		if(!m_bEnableTime)
			return;

		GetManager()->KillTimer(this);
		GetManager()->SetTimer(this,1650,m_uDelay);
	}

	void CEditDisableRMenuUI::OnTimer( UINT iTimerID )
	{
		if(_tcscmp(m_sCheckVal.GetData(),GetText().GetData()) != 0)
		{
			m_sCheckVal = GetText();
			GetManager()->SendNotify(this,DUI_MSGTYPE_EDITIMTER);
		}
	}

	UINT CEditDisableRMenuUI::GetTimerDelay()
	{
		return m_uDelay;
	}
}
