#ifndef __UIHORIZONTALLAYOUTFUNCTION_H__
#define __UIHORIZONTALLAYOUTFUNCTION_H__

// -----------------------------------------------------------------------
// �������� : �ÿؼ��̳�CHorizontalLayoutUI�����ڵ�������Ӧ�����Ĵ��ڴ����չ���ܰ�ť�Լ��������������ť
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