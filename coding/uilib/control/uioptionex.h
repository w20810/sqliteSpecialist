#ifndef __UIOPTIONEX_H__
#define __UIOPTIONEX_H__

// -----------------------------------------------------------------------
// �������� :	�ÿؼ��̳�COptionUI������֧�ֵ����л�״̬
//            ����ǰ״̬         ������״̬
//				ѡ��------------->δѡ��
//				δѡ��----------->ѡ��
// -----------------------------------------------------------------------
#pragma once

namespace UiLib
{
	class UILIB_API COptionExUI : public COptionUI
	{
	public:
		COptionExUI();
		virtual ~COptionExUI();

		LPCTSTR GetClass() const;
		LPVOID GetInterface(LPCTSTR pstrName);
		virtual bool Activate();
		void Selected(bool bSelected);
		void DoEvent(TEventUI& event);

	private:
		bool	m_bDraged;
		bool	m_bCanDrag;
		POINT	m_ptLastCursor;
	};

} // namespace UiLib

#endif // __UIOPTIONEX_H__