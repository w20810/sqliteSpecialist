#ifndef __UIHORIZONTALLAYOUTFUNCTION_H__
#define __UIHORIZONTALLAYOUTFUNCTION_H__

// -----------------------------------------------------------------------
// 功能描述 : 该控件继承CHorizontalLayoutUI，用于稻壳搜搜应用中心窗口存放扩展功能按钮以及添加至工具栏按钮
// -----------------------------------------------------------------------
#pragma once

namespace UiLib
{
	class UILIB_API CHorizontalLayoutFuncUI : public CHorizontalLayoutUI
	{
	public:
		CHorizontalLayoutFuncUI();
		LPCTSTR GetClass() const;
		void DoEvent(TEventUI& event);
	};

} // namespace UiLib

#endif // __UIHORIZONTALLAYOUTFUNCTION_H__