#ifndef __UIBUTTONEX_H__
#define __UIBUTTONEX_H__

// -----------------------------------------------------------------------
// �������� :	�ÿؼ��̳�CButtonUI������֧�����ùؼ�����ʽ
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

		//�зֱ༭������õ�n���ؼ���
		void TextValueSplit(std::wstring& s);

		//HTML��ʽ�ַ���
		void SetTextStyleValue();

		//������ʽ��
		void BuildResultTextValue();

		//HTML��������label�ı�
		virtual void PaintText(HDC hDC);
		void SetText(LPCTSTR pstrText);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		//�����ʽ��ǰ׺
		void BuildKeyWordStylePre();
		//�����ʽ����׺
		void BuildKeyWordStyleBack();
		//���ùؼ�����ʾ��ʽ
		void SetKeyWordStyle(DWORD dwKeyWordStyle);
		void SetKeyWordColor(DWORD dwKeyWordColor);
		//Ӧ�ùؼ�����ʾ��ʽ
		void ApplyKeyWordStyle();
		// ����text��html��ʽ��Ŀǰֻ֧��	KWS_BOLD,KWS_ITALIC,KWS_SELECTED,KWS_UNDERLINE
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