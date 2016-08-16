#include "stdafx.h"
#include "uioptionex.h"

namespace UiLib
{
	COptionExUI::COptionExUI() : m_bCanDrag(false), m_bDraged(false)
	{

	}

	COptionExUI::~COptionExUI()
	{

	}

	LPCTSTR COptionExUI::GetClass() const
	{
		return _T("OptionExUI");
	}

	LPVOID COptionExUI::GetInterface(LPCTSTR pstrName)
	{
		if( _tcscmp(pstrName, DUI_CTR_OPTIONEX) == 0 ) return static_cast<COptionExUI*>(this);
			return COptionUI::GetInterface(pstrName);
	}

	bool COptionExUI::Activate()
	{
		if( !CButtonUI::Activate() || m_bDraged)
		{
			return false;
		}

		if( !m_sGroupName.IsEmpty() && !m_bSelected)
		{
			Selected(true);
		}
		else
		{
			Selected(!m_bSelected);
		}

		return true;
	}

	void COptionExUI::Selected(bool bSelected)
	{
		if( m_bSelected == bSelected ) return;
		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.IsEmpty() ) {
				if( m_bSelected ) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this ) {
							pControl->Selected(false);
						}
					}
					Invalidate();
					m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
				}
			}
			else {
				Invalidate();
				m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
			}
		}

		Invalidate();

		if (!m_bSelected)
			m_pManager->SendNotify(this, DUI_MSGTYPE_CANCLESELECTED);

	}
}

void COptionExUI::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
	{
		m_ptLastCursor.x = event.ptMouse.x;
		m_ptLastCursor.y = event.ptMouse.y;

		if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() )
		{
			m_bCanDrag = true;
			m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_MOUSEMOVE )
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 )
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) )
			{
				if (m_bCanDrag && (m_ptLastCursor.x != event.ptMouse.x || m_ptLastCursor.y != event.ptMouse.y ))
					m_bDraged = true;
				m_uButtonState |= UISTATE_PUSHED;
			}
			else
			{
				m_uButtonState &= ~UISTATE_PUSHED;
			}
			Invalidate();
		}
		return;
	}
	if( event.Type == UIEVENT_BUTTONUP )
	{
		m_bCanDrag = false;

		if( (m_uButtonState & UISTATE_CAPTURED) != 0 )
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse))
			{
				Activate();
			}
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			Invalidate();
		}

		m_bDraged = false;
		return;
	}

	COptionUI::DoEvent(event);
}