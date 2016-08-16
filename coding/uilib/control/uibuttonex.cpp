#include "stdafx.h"
#include "uibuttonex.h"

namespace UiLib
{
	CButtonExUI::CButtonExUI() : m_iKeyWordNum(0), m_dwKeyWordColor(RGB(1, 0, 0)), m_bIsSetTheTextStyle(NULL), m_dwKeyWordStyle(0), m_dwFocusedBkColor(0)
	{
		SetEnabledEffect(false);
		SetShowHtml(true);
	}

	CButtonExUI::~CButtonExUI()
	{

	}

	LPCTSTR CButtonExUI::GetClass() const
	{
		return _T("ButtonExUI");
	}

	LPVOID CButtonExUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_BUTTONEX) == 0 ) return static_cast<CButtonExUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void CButtonExUI::SetKeyWord(CDuiString strKeyWord)
	{
		m_strKeyWord = strKeyWord;
	}

	CDuiString CButtonExUI::GetKeyWord()
	{
		return m_strKeyWord;
	}

	void CButtonExUI::SetText( LPCTSTR pstrText )
	{
		m_strTextValue = pstrText;
		m_strResultTextValue = pstrText;
		ApplyKeyWordStyle();

		if (!GetEnabledEffect())
			return CControlUI::SetText(pstrText);

		m_TextValue = pstrText;
	}

	void CButtonExUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (wcscmp(pstrName, L"keywordcolor") == 0 )
		{
			while(*pstrValue > L'\0' && *pstrValue <= L' ') pstrValue = ::CharNext(pstrValue);
			if (*pstrValue == L'#') pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = wcstoul(pstrValue, &pstr, 16);
			SetKeyWordColor(clrColor);
		}
		else if (wcscmp(pstrName, L"textcolor") == 0)
		{
			while(*pstrValue > L'\0' && *pstrValue <= L' ') pstrValue = ::CharNext(pstrValue);
			if (*pstrValue == L'#') pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = wcstoul(pstrValue, &pstr, 16);
			m_dwTextColor = clrColor;
		}
		else if (wcscmp(pstrName, L"keywordstyle") == 0 )
		{
			if(_tcsstr(pstrValue, _T("bold")) != NULL)
			{
				SetKeyWordStyle(KWS_BOLD);
			}
			if(_tcsstr(pstrValue, _T("italic")) != NULL)
			{
				SetKeyWordStyle(KWS_ITALIC);
			}
			if(_tcsstr(pstrValue, _T("selected")) != NULL)
			{
				SetKeyWordStyle(KWS_SELECTED);
			}
			if(_tcsstr(pstrValue, _T("underline")) != NULL)
			{
				SetKeyWordStyle(KWS_UNDERLINE);
			}
			if(_tcsstr(pstrValue, _T("color")) != NULL)
			{
				SetKeyWordStyle(KWS_COLOR);
			}
		}
		else if(_tcscmp(pstrName, _T("keyword")) == 0)
		{
			SetKeyWord(pstrValue);
		}
		else if (wcscmp(pstrName, L"focusedbkcolor") == 0 )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetFocusedBkColor(clrColor);
		}
		else
			CButtonUI::SetAttribute(pstrName, pstrValue);
	}

	CDuiString CButtonExUI::GetHtmlStyleTagBegin(DWORD dwTextStyle)
	{
		CDuiString strHtmlText;
		if (dwTextStyle & KWS_UNDERLINE)
			strHtmlText.Append(L"<u>");
		if (dwTextStyle & KWS_SELECTED)
			strHtmlText.Append(L"<s>");
		if (dwTextStyle & KWS_ITALIC)
			strHtmlText.Append(L"<i>");
		if (dwTextStyle & KWS_BOLD)
			strHtmlText.Append(L"<b>");
		return strHtmlText;
	}

	CDuiString CButtonExUI::GetHtmlStyleTagEnd(DWORD dwTextStyle)
	{
		CDuiString strHtmlText;
		if (dwTextStyle & KWS_BOLD)
			strHtmlText.Append(L"</b>");
		if (dwTextStyle & KWS_ITALIC)
			strHtmlText.Append(L"</i>");
		if (dwTextStyle & KWS_SELECTED)
			strHtmlText.Append(L"</s>");
		if (dwTextStyle & KWS_UNDERLINE)
			strHtmlText.Append(L"</u>");
		return strHtmlText;
	}

	void CButtonExUI::SetTextHtmlStyle(DWORD dwTextStyle)
	{
		CDuiString strTagBegin = GetHtmlStyleTagBegin(dwTextStyle);
		CDuiString strTagEnd = GetHtmlStyleTagEnd(dwTextStyle);
		m_strResultTextValue = strTagBegin + CDuiString(m_strResultTextValue.c_str()) + strTagEnd;
	}

	void CButtonExUI::SetKeyWordStyle(DWORD dwKeyWordStyle)
	{
		m_dwKeyWordStyle |= dwKeyWordStyle;
		ApplyKeyWordStyle();
	}

	void CButtonExUI::SetKeyWordColor(DWORD dwKeyWordColor)
	{
		m_dwKeyWordColor= dwKeyWordColor;
		SetKeyWordStyle(m_dwKeyWordStyle | KWS_COLOR);
	}

	void CButtonExUI::ApplyKeyWordStyle()
	{
		//构造格式串前缀后缀
		BuildKeyWordStylePre();
		BuildKeyWordStyleBack();
		m_strResultTextValue = m_strTextValue;
		if (!m_strResultTextValue.empty())
			BuildResultTextValue();
	}

	void CButtonExUI::TextValueSplit(std::wstring& s)
	{
		size_t pos = 0;
		size_t len = s.length();
		int posPre = 0;
		int posBack = 0;
		int size = s.size();
		for (int i = 0; i < size;)
		{
			while (i < size && s[i] == L' ')
			{
				posPre = i;
				i++;
			}

			while (i < size && s[i] != L' ')
			{
				posBack = i;
				i++;
			}

			if ((posBack < size && posBack > posPre) || posPre == 0)
			{
				if (s.at(posPre) == L' ')
					m_strKeyWordResult.push_back(s.substr(posPre + 1, posBack - posPre));
				else
					m_strKeyWordResult.push_back(s.substr(posPre, posBack - posPre + 1));
			}
		}
	}

	void CButtonExUI::BuildResultTextValue()
	{
		CDuiString strDuiKeyWord(m_strKeyWord.GetData());
		strDuiKeyWord.MakeUpper();

		//切分编辑框输入，得到关键字
		TextValueSplit(strDuiKeyWord.GetStringW());
		m_iKeyWordNum = m_strKeyWordResult.size();

		//构造格式串
		SetTextStyleValue();
	}

	void CButtonExUI::SetTextStyleValue()
	{
		CDuiString strDuiTextValue(m_strTextValue.GetData());
		strDuiTextValue.MakeUpper();
		wstring wStrTextValue = strDuiTextValue.GetStringW();
		int nLenOfTextValue = m_strTextValue.GetLength();

		//设置格式串标识位
		m_bIsSetTheTextStyle = new bool[nLenOfTextValue];
		memset(m_bIsSetTheTextStyle, 0, sizeof(bool) * nLenOfTextValue);
		for (int i = 0; i < m_iKeyWordNum; i++)
		{
			wstring::size_type wpos(0);
			wstring strKeyWord = m_strKeyWordResult.at(i);
			int nLenKeyWord = strKeyWord.length();
			while ((wpos = wStrTextValue.find(strKeyWord, wpos)) != wstring::npos)
			{
				memset(m_bIsSetTheTextStyle + wpos, true, sizeof(bool) * nLenKeyWord);
				wpos += nLenKeyWord;
			}
		}

		//根据格式串标志位，插入格式串前缀后缀
		bool bHasStyle = false;
		wstring strResultTextValue;
		for (int i = 0; i < nLenOfTextValue; i++)
		{
			if (m_bIsSetTheTextStyle[i] && !bHasStyle)
			{
				strResultTextValue.append(m_strKeyWordStylePre);
				bHasStyle = true;
			}
			else if (!m_bIsSetTheTextStyle[i] && bHasStyle)
			{
				strResultTextValue.append(m_strKeyWordStyleBack);
				bHasStyle = false;
			}

			strResultTextValue.append(1, m_strResultTextValue[i]);
		}

		m_strResultTextValue = strResultTextValue;
	}

	void CButtonExUI::BuildKeyWordStylePre()
	{
		if (m_dwKeyWordStyle == 0)
		{
			m_strResultTextValue.clear();
			return;
		}

		m_strKeyWordStylePre.clear();

		//color
		if (m_dwKeyWordStyle & KWS_COLOR)
		{
			m_strKeyWordStylePre.append(L"<c ");
			TCHAR wstrdwColor[10] = {0};

			if (GetManager() != NULL && (GetManager()->IsBackgroundTransparent()) && m_dwKeyWordColor == 0xff000000)
				m_dwKeyWordColor = RGB(1, 0, 0);
			m_dwKeyWordColor = m_dwKeyWordColor & 0x00FFFFFF;

			swprintf(wstrdwColor, L"#%08x", m_dwKeyWordColor);
			m_strKeyWordStylePre.append(wstrdwColor);
			m_strKeyWordStylePre.append(L">");
		}

		m_strKeyWordStylePre.append(GetHtmlStyleTagBegin(m_dwKeyWordStyle));
	}

	void CButtonExUI::BuildKeyWordStyleBack()
	{
		//后缀构造与前缀顺序相反
		m_strKeyWordStyleBack.clear();

		m_strKeyWordStyleBack.append(GetHtmlStyleTagEnd(m_dwKeyWordStyle));

		//color
		if (m_dwKeyWordStyle & KWS_COLOR)
			m_strKeyWordStyleBack.append(L"</c>");
	}

	void CButtonExUI::PaintText(HDC hDC)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if( m_sText.IsEmpty() ) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
			clrColor = GetPushedTextColor();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
			clrColor = GetHotTextColor();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
			clrColor = GetFocusedTextColor();

		//使用CRenderEngine::DrawHtmlText解析格式串绘制文本
		if (m_bShowHtml)
		{
			//判断主窗口是否背景透明，避免字体透明；将RGB(0, 0, 0)设置为RGB(1, 0, 0)
			if ((GetManager()->IsBackgroundTransparent()) && m_dwTextColor  == 0xff000000)
				m_dwTextColor = RGB(1, 0, 0);

			SetTextRenderingHintAntiAlias(TextRenderingHintAntiAlias);
			if (!m_strResultTextValue.empty())
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_strResultTextValue.c_str(), clrColor, \
				NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
			else
				CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, clrColor, \
				NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
		}
		else
		{
			CButtonUI::PaintText(hDC);
		}
	}
}

void CButtonExUI::PaintStatusImage(HDC hDC)
{
	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
		if( !m_sDisabledImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
			else goto Label_ForeImage;
		}
	}
	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
		if( !m_sPushedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) ){
				m_sPushedImage.Empty();
			}
			if( !m_sPushedForeImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sPushedForeImage) )
					m_sPushedForeImage.Empty();
				return;
			}
			else goto Label_ForeImage;
		}
		else if(m_dwPushedBkColor != 0) {
			CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwPushedBkColor));
			return;
		}
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !m_sHotImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ){
				m_sHotImage.Empty();
			}
			if( !m_sHotForeImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sHotForeImage) )
					m_sHotForeImage.Empty();
				return;
			}
			else goto Label_ForeImage;
		}
		else if(m_dwHotBkColor != 0) {
			CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHotBkColor));
		}
	}
	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
		if( !m_sFocusedImage.IsEmpty() ) {
			if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
			else goto Label_ForeImage;
		}
		else if(m_dwFocusedBkColor != 0) {
			CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwFocusedBkColor));
		}
	}
	if( !m_sNormalImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
		else goto Label_ForeImage;
	}
	if(!m_sForeImage.IsEmpty() )
		goto Label_ForeImage;
	return;

Label_ForeImage:
	if(!m_sForeImage.IsEmpty() ) {
		if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
	}
}

void CButtonExUI::SetFocusedBkColor(DWORD dwColor)
{
	m_dwFocusedBkColor = dwColor;
}

DWORD CButtonExUI::GetFocusedBkColor() const
{
	return m_dwFocusedBkColor;
}