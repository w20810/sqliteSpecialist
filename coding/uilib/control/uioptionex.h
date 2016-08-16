#ifndef __UIOPTIONEX_H__
#define __UIOPTIONEX_H__

// -----------------------------------------------------------------------
// 功能描述 :	该控件继承COptionUI，并且支持单击切换状态
//            单击前状态         单击后状态
//				选中------------->未选中
//				未选中----------->选中
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