#ifndef __UIBUTTONEX_H__
#define __UIBUTTONEX_H__

// -----------------------------------------------------------------------
// 功能描述 :	该控件继承CButtonUI，并且支持设置关键字样式
// -----------------------------------------------------------------------
#pragma once

namespace UiLib
{
	class UILIB_API CButtonExUI : public CButtonUI
	{
	public:
		CButtonExUI();
		virtual ~CButtonExUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetKeyWord(CDuiString strKeyWord);
		CDuiString GetKeyWord();

		void SetFocusedBkColor(DWORD dwColor);
		DWORD GetFocusedBkColor() const;

		//切分编辑框输入得到n个关键字
		void TextValueSplit(std::wstring& s);

		//HTML格式字符串
		void SetTextStyleValue();

		//构建格式串
		void BuildResultTextValue();

		//HTML解析绘制label文本
		virtual void PaintText(HDC hDC);
		void SetText(LPCTSTR pstrText);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		//构造格式串前缀
		void BuildKeyWordStylePre();
		//构造格式串后缀
		void BuildKeyWordStyleBack();
		//设置关键字显示格式
		void SetKeyWordStyle(DWORD dwKeyWordStyle);
		void SetKeyWordColor(DWORD dwKeyWordColor);
		//应用关键字显示格式
		void ApplyKeyWordStyle();
		// 设置text的html样式，目前只支持	KWS_BOLD,KWS_ITALIC,KWS_SELECTED,KWS_UNDERLINE
		void SetTextHtmlStyle(DWORD dwTextStyle);

		void PaintStatusImage(HDC hDC);
	private:
		CDuiString GetHtmlStyleTagBegin(DWORD dwTextStyle);
		CDuiString GetHtmlStyleTagEnd(DWORD dwTextStyle);

	private:
		int							m_iKeyWordNum;
		DWORD						m_dwKeyWordStyle;
		DWORD						m_dwKeyWordColor;
		DWORD						m_dwFocusedBkColor;
		wstring						m_strKeyWordStylePre;
		wstring						m_strKeyWordStyleBack;

		CDuiString					m_strKeyWord;
		CDuiString					m_strTextValue;
		wstring						m_strResultTextValue;
		bool*						m_bIsSetTheTextStyle;
		std::vector<std::wstring>	m_strKeyWordResult;
	};

} // namespace UiLib

#endif // __UIBUTTONEX_H__