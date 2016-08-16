// -----------------------------------------------------------------------
// �������� :	�ÿؼ��̳�CHorizontalLayoutUI�����ڵ�������Ӧ�����Ĵ��ڴ����չ���ܰ�ť�Լ��������������ť
// -----------------------------------------------------------------------
#include "stdafx.h"
#include "uihorizontallayoutfunction.h"

CHorizontalLayoutFuncUI::CHorizontalLayoutFuncUI()
{

}

LPCTSTR CHorizontalLayoutFuncUI::GetClass() const
{
	return _T("HorizontalLayoutFuncUI");
}

void CHorizontalLayoutFuncUI::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_MOUSEENTER)
	{
		m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER, event.wParam, event.lParam);
	}
	if (event.Type == UIEVENT_MOUSELEAVE)
	{
		m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE, event.wParam, event.lParam);
	}

	CHorizontalLayoutUI::DoEvent(event);
}