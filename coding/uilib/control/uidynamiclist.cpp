#include "stdafx.h"
#include "uidynamiclist.h"

namespace UiLib {

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDynamicListUI::CDynamicListUI() 
		: m_pCallback(NULL), m_bScrollSelect(false)/*, m_iCurSel(-1)*/, m_iExpandedItem(-1), m_bSingleSel(false)
	{
		m_pList = new CDynamicListBodyUI(this);
		m_pHeader = new CListHeaderUI;

		Add(m_pHeader);
		CVerticalLayoutUI::Add(m_pList);

		m_ListInfo.nColumns = 0;
		m_ListInfo.nFont = -1;
		m_ListInfo.uTextStyle = DT_VCENTER; // m_uTextStyle(DT_VCENTER | DT_END_ELLIPSIS)
		m_ListInfo.dwTextColor = 0xFF000000;
		m_ListInfo.dwBkColor = 0;
		m_ListInfo.bAlternateBk = false;
		m_ListInfo.dwSelectedTextColor = 0xFF000000;
		m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
		m_ListInfo.dwHotTextColor = 0xFF000000;
		m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
		m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
		m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
		m_ListInfo.dwLineColor = 0;
		m_ListInfo.bShowHtml = false;
		m_ListInfo.bMultiExpandable = false;
		m_ListInfo.bShowHLine = true;
		m_ListInfo.bShowVLine = true;
		::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
		::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));
		::ZeroMemory(&m_ListInfo.szCheckImg, sizeof(m_ListInfo.szCheckImg));
		::ZeroMemory(&m_ListInfo.szIconImg, sizeof(m_ListInfo.szIconImg));

		m_bSelRange = false;
		m_nRangeBegin = -1;
		m_nRangeEnd = -1;
		m_nLastSelect = -1;
	}

	CDynamicListUI::~CDynamicListUI()
	{

	}

	LPCTSTR CDynamicListUI::GetClass() const
	{
		return _T("ListUI");
	}

	UINT CDynamicListUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	LPVOID CDynamicListUI::GetInterface(LPCTSTR pstrName)
	{
		if(m_pHeader)
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);

		if( _tcscmp(pstrName, DUI_CTR_DYNAMICLIST) == 0 ) return static_cast<CDynamicListUI*>(this);
		if( _tcscmp(pstrName, _T("IList")) == 0 ) return static_cast<IListUI*>(this);
		if( _tcscmp(pstrName, _T("IListOwner")) == 0 ) return static_cast<IListOwnerUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	CControlUI* CDynamicListUI::GetItemAt(int iIndex) const
	{
		return m_pList->GetItemAt(iIndex);
	}

	CControlUI* CDynamicListUI::GetItemInDList(int iIndex) const
	{
		IDynamicListCallbackUI* pCallback = GetItemCallback();
		CControlUI* pControl = NULL;
		if (pCallback)
		{
			pControl = pCallback->GetItem(iIndex);
		}

		return pControl;
	}

	int CDynamicListUI::GetItemIndex(CControlUI* pControl) const
	{
		if( pControl->GetInterface(DUI_CTR_LISTHEADER) != NULL ) return CVerticalLayoutUI::GetItemIndex(pControl);
		// We also need to recognize header sub-items
		if( _tcsstr(pControl->GetClass(), DUI_CTR_LISTHEADERITEM) != NULL ) return m_pHeader->GetItemIndex(pControl);

		return m_pList->GetItemIndex(pControl);
	}

	bool CDynamicListUI::SetItemIndex(CControlUI* pControl, int iIndex)
	{
		if( pControl->GetInterface(DUI_CTR_LISTHEADER) != NULL ) return CVerticalLayoutUI::SetItemIndex(pControl, iIndex);
		// We also need to recognize header sub-items
		if( _tcsstr(pControl->GetClass(), DUI_CTR_LISTHEADERITEM) != NULL ) return m_pHeader->SetItemIndex(pControl, iIndex);

		int iOrginIndex = m_pList->GetItemIndex(pControl);
		if( iOrginIndex == -1 ) return false;
		if( iOrginIndex == iIndex ) return true;

		IListItemUI* pSelectedListItem = NULL;
		if( !m_pList->SetItemIndex(pControl, iIndex) ) return false;
		int iMinIndex = min(iOrginIndex, iIndex);
		int iMaxIndex = max(iOrginIndex, iIndex);
		for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if( pListItem != NULL ) {
				pListItem->SetIndex(i);
			}
		}
		UnSelectAllItems();
		return true;
	}

	void CDynamicListUI::SetDynamicAttr(int totalSize, int perHeight, bool bOnlyUpdate)
	{
		m_pList->SetDynamicAttr(totalSize, perHeight, bOnlyUpdate);
	}

	//涉及item的移动
	void CDynamicListUI::UpdateItem(int form /* = 0 */)
	{
		m_pList->Update(form);
	}

	//纯刷新
	void CDynamicListUI::UpdateItemOnly()
	{
		CControlUI::Invalidate();
		m_pList->Update(m_pList->GetCurFirstItem());
	}

	int CDynamicListUI::GetCount() const
	{
		return m_pList->GetCount();
	}

	int CDynamicListUI::GetTotalCount() const
	{
		return m_pList->GetTotalCount();
	}

	int CDynamicListUI::GetCurFirstItem() const
	{
		return m_pList->GetCurFirstItem();
	}

	bool CDynamicListUI::Add(CControlUI* pControl)
	{
		// Override the Add() method so we can add items specifically to
		// the intended widgets. Headers are assumed to be
		// answer the correct interface so we can add multiple list headers.
		if( pControl->GetInterface(DUI_CTR_LISTHEADER) != NULL ) {
			if( m_pHeader != pControl && m_pHeader->GetCount() == 0 ) {
				CVerticalLayoutUI::Remove(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return CVerticalLayoutUI::AddAt(pControl, 0);
		}
		// We also need to recognize header sub-items
		if( _tcsstr(pControl->GetClass(), DUI_CTR_LISTHEADERITEM) != NULL ) {
			bool ret = m_pHeader->Add(pControl);
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return ret;
		}
		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(DUI_CTR_LISTITEM));
		if( pListItem != NULL ) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(GetCount());
		}
		return m_pList->Add(pControl);
	}

	bool CDynamicListUI::AddAt(CControlUI* pControl, int iIndex)
	{
		// Override the AddAt() method so we can add items specifically to
		// the intended widgets. Headers and are assumed to be
		// answer the correct interface so we can add multiple list headers.
		if( pControl->GetInterface(DUI_CTR_LISTHEADER) != NULL ) {
			if( m_pHeader != pControl && m_pHeader->GetCount() == 0 ) {
				CVerticalLayoutUI::Remove(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return CVerticalLayoutUI::AddAt(pControl, 0);
		}
		// We also need to recognize header sub-items
		if( _tcsstr(pControl->GetClass(), DUI_CTR_LISTHEADERITEM) != NULL ) {
			bool ret = m_pHeader->AddAt(pControl, iIndex);
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return ret;
		}
		if (!m_pList->AddAt(pControl, iIndex)) return false;

		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(DUI_CTR_LISTITEM));
		if( pListItem != NULL ) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(iIndex);
		}

		for(int i = iIndex + 1; i < m_pList->GetCount(); ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if( pListItem != NULL ) {
				pListItem->SetIndex(i);
			}
		}
		UnSelectAllItems();
		return true;
	}

	bool CDynamicListUI::Remove(CControlUI* pControl)
	{
		if( pControl->GetInterface(DUI_CTR_LISTHEADER) != NULL ) return CVerticalLayoutUI::Remove(pControl);
		// We also need to recognize header sub-items
		if( _tcsstr(pControl->GetClass(), DUI_CTR_LISTHEADERITEM) != NULL ) return m_pHeader->Remove(pControl);

		int iIndex = m_pList->GetItemIndex(pControl);
		if (iIndex == -1) return false;

		if (!m_pList->RemoveAt(iIndex)) return false;

		for(int i = iIndex; i < m_pList->GetCount(); ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if( pListItem != NULL ) {
				pListItem->SetIndex(i);
			}
		}

		m_aSelItems.Remove(m_aSelItems.Find((LPVOID)iIndex));
		return true;
	}

	bool CDynamicListUI::RemoveAt(int iIndex)
	{
		if (!m_pList->RemoveAt(iIndex)) return false;

		for(int i = iIndex; i < m_pList->GetCount(); ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(DUI_CTR_LISTITEM));
			if( pListItem != NULL ) pListItem->SetIndex(i);
		}

		m_aSelItems.Remove(m_aSelItems.Find((LPVOID)iIndex));
		return true;
	}

	void CDynamicListUI::RemoveAll()
	{
		//m_iCurSel = -1;
		m_iExpandedItem = -1;
		m_pList->RemoveAll();
	}

	void CDynamicListUI::SetPos(RECT rc)
	{
		CVerticalLayoutUI::SetPos(rc);
		if( m_pHeader == NULL ) return;
		// Determine general list information and the size of header columns
		m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
		// The header/columns may or may not be visible at runtime. In either case
		// we should determine the correct dimensions...

		if( !m_pHeader->IsVisible() ) {
			for( int it = 0; it < m_pHeader->GetCount(); it++ ) {
				static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
			}
			m_pHeader->SetPos(CDuiRect(rc.left, 0, rc.right, 0));
		}
		int iOffset = m_pList->GetScrollPos().cx;
		for( int i = 0; i < m_ListInfo.nColumns; i++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pHeader->GetItemAt(i));
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;

			RECT rcPos = pControl->GetPos();
			if( iOffset > 0 ) {
				rcPos.left -= iOffset;
				rcPos.right -= iOffset;
				pControl->SetPos(rcPos);
			}
			m_ListInfo.rcColumn[i] = pControl->GetPos();
		}
		if( !m_pHeader->IsVisible() ) {
			for( int it = 0; it < m_pHeader->GetCount(); it++ ) {
				static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
			}
		}

		m_pList->Update(m_pList->GetCurFirstItem());
		m_pList->SetPos(m_pList->GetPos());
	}

	int CDynamicListUI::GetMinSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int min = (int)m_aSelItems.GetAt(0);
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = (int)m_aSelItems.GetAt(i);
			if (min > index)
				min = index;
		}
		return min;
	}

	int CDynamicListUI::GetMaxSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int max = (int)m_aSelItems.GetAt(0);
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = (int)m_aSelItems.GetAt(i);
			if (max < index)
				max = index;
		}
		return max;
	}

	void CDynamicListUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CVerticalLayoutUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			m_bFocused = true;
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			//UnSelectAllItems();
			m_bFocused = false;
			return;
		}

		switch( event.Type ) {
		case UIEVENT_KEYDOWN:
			switch( event.chKey ) {
			case VK_UP:
				{
					if (m_aSelItems.GetSize() > 0)
					{
						//DLOG(INFO)<<m_aSelItems.GetAt(0);
						//DLOG(INFO)<<m_aSelItems.GetAt(1);
						int index = GetMinSelItemIndex() - 1;
						
						if (!(::GetKeyState(VK_SHIFT) < 0))
							UnSelectAllItems();

						if (index < 0)
							EnsureUpdateVisible(index);
						index > 0 ? SelectItem(index, true, false) : SelectItem(0, true, false);
					}
					else if (m_pList->GetCount() > 0)
					{
						m_pList->Update(m_pList->GetTotalCount() - m_pList->GetTotalHeight() + 1);
						SelectItem(m_pList->GetCount() - 1, true, false);
					}
				}			
				return;
			case VK_DOWN:
				{
					if (m_aSelItems.GetSize() > 0)
					{					
						int index = GetMaxSelItemIndex() + 1;
						if (!(::GetKeyState(VK_SHIFT) < 0))
							UnSelectAllItems();

						if (index >= m_pList->GetCount() - 1)
						{
							EnsureUpdateVisible(index);
						}
						index + 1 > m_pList->GetCount() ? SelectItem(m_pList->GetCount() - 1, true, false) : SelectItem(index, true, false);
					}
					else if (m_pList->GetCount() > 0)
					{
						m_pList->Update(0);
						SelectItem(0, true, false);
					}
				}
				return;
			case VK_PRIOR:
				PageUp();
				return;
			case VK_NEXT:
				PageDown();
				return;
			case VK_HOME:
				{
					if (m_pList->GetCount() > 0)
					{
						m_pList->Update(0);
						SelectItem(0, true);
					}
				}
				return;
			case VK_END:
				{
					if (m_pList->GetCount() > 0)
					{
						m_pList->Update(m_pList->GetTotalCount() - m_pList->GetTotalHeight() + 1);
						SelectItem(m_pList->GetCount() - 1, true);
					}
				}
				return;
			case 0x41: // ctrl+A
				{
					if (!m_bSingleSel && (GetKeyState(VK_CONTROL) & 0x8000))
					{
						SelectItemsRange(0, GetTotalCount() - 1);
					}
				}
				return;
			}
				break;
			case UIEVENT_SCROLLWHEEL:
				{
					switch( LOWORD(event.wParam) )
					{
					case SB_LINEUP:
						if( m_bScrollSelect && m_bSingleSel)
							SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) - 1, false), true);
						else LineUp();
						return;
					case SB_LINEDOWN:
						if( m_bScrollSelect && m_bSingleSel) 
							SelectItem(FindSelectable((int)m_aSelItems.GetAt(0) + 1, true), true);
						else LineDown();
						return;
					}
				}
				break;

		}
		CVerticalLayoutUI::DoEvent(event);
	}

	CListHeaderUI* CDynamicListUI::GetHeader() const
	{
		return m_pHeader;
	}

	CContainerUI* CDynamicListUI::GetList() const
	{
		return m_pList;
	}

	bool CDynamicListUI::GetScrollSelect()
	{
		return m_bScrollSelect;
	}

	void CDynamicListUI::SetScrollSelect(bool bScrollSelect)
	{
		m_bScrollSelect = bScrollSelect;
	}

	int CDynamicListUI::GetCurSel() const
	{
		if (m_aSelItems.GetSize() <= 0)
		{
			return -1;
		}
		else
		{
			return (int)m_aSelItems.GetAt(0);
		}

		return -1;
	}

	int CDynamicListUI::GetLastSel() const
	{
		return m_nLastSelect;
	}

	bool CDynamicListUI::SelectItem(int iIndex, bool bTakeFocus, bool bUnSelectOther, bool bSelected)
	{
		int nFirstItem = m_pList->GetCurFirstItem();
		if( iIndex < 0 ) return false;
		//IDynamicListCallbackUI * pCallback = GetItemCallback();
		//CControlUI* pControl = pCallback->GetItem(iIndex);
		CControlUI* pControl = GetItemAt(iIndex);
		if( pControl == NULL ) return false;
		if( !pControl->IsVisible() ) return false;
		if( !pControl->IsEnabled() ) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL ) return false;

		if (::GetKeyState(VK_SHIFT) < 0)
		{
			// 当前选中情况
			// 1、不连续
			// 最后一次的选中 到index
			// 2、连续选中[begin,end]
				// begin < end
						// index <= begin		[index, begin]
						// index > begin		[begin, index]
				// begin > end
						// [begin , index]
			if (m_bSelRange == true)
			{
				m_nRangeEnd = iIndex + nFirstItem;
			}
			else
			{
				UnSelectAllItems();
				m_bSelRange = true;
				m_nRangeBegin = m_nLastSelect;
				m_nRangeEnd = iIndex + nFirstItem;
			}

			SelectItemsRange(m_nRangeBegin, m_nRangeEnd);
		}

		else if (::GetKeyState(VK_CONTROL) < 0)
		{
			m_bSelRange = false;
		}

		m_nLastSelect = nFirstItem + iIndex;

		if(m_bSingleSel && m_aSelItems.GetSize() > 0) {
			CControlUI* pControl = GetItemAt((int)m_aSelItems.GetAt(0));
			if( pControl != NULL) {
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if( pListItem != NULL ) pListItem->Select(false);
			}
		}
		if( !pListItem->Select(true, false) ) {
			return false;
		}

		if (!bSelected)
		{
			int pos = m_aSelItemIndex.Find(LPVOID(nFirstItem + iIndex));

			if (pos == -1)
				m_aSelItemIndex.Add(LPVOID(nFirstItem + iIndex));
		}
		m_aSelItems.Add((LPVOID)iIndex);

		//EnsureVisible(iIndex);
		if( bTakeFocus ) pControl->SetFocus();
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex);
		}

		return true;
	}

	TListInfoUI* CDynamicListUI::GetListInfo()
	{
		return &m_ListInfo;
	}

	bool CDynamicListUI::IsDelayedDestroy() const
	{
		return m_pList->IsDelayedDestroy();
	}

	void CDynamicListUI::SetDelayedDestroy(bool bDelayed)
	{
		m_pList->SetDelayedDestroy(bDelayed);
	}

	int CDynamicListUI::GetChildPadding() const
	{
		return m_pList->GetChildPadding();
	}

	void CDynamicListUI::SetChildPadding(int iPadding)
	{
		m_pList->SetChildPadding(iPadding);
	}

	void CDynamicListUI::SetItemFont(int index)
	{
		m_ListInfo.nFont = index;
		NeedUpdate();
	}

	void CDynamicListUI::SetItemTextStyle(UINT uStyle)
	{
		m_ListInfo.uTextStyle = uStyle;
		NeedUpdate();
	}

	void CDynamicListUI::SetItemTextPadding(RECT rc)
	{
		m_ListInfo.rcTextPadding = rc;
		NeedUpdate();
	}

	RECT CDynamicListUI::GetItemTextPadding() const
	{
		return m_ListInfo.rcTextPadding;
	}

	void CDynamicListUI::SetItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwTextColor = dwTextColor;
		Invalidate();
	}

	void CDynamicListUI::SetItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwBkColor = dwBkColor;
		Invalidate();
	}

	void CDynamicListUI::SetItemBkImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sBkImage = pStrImage;
		Invalidate();
	}

	void CDynamicListUI::SetAlternateBk(bool bAlternateBk)
	{
		m_ListInfo.bAlternateBk = bAlternateBk;
		Invalidate();
	}

	DWORD CDynamicListUI::GetItemTextColor() const
	{
		return m_ListInfo.dwTextColor;
	}

	DWORD CDynamicListUI::GetItemBkColor() const
	{
		return m_ListInfo.dwBkColor;
	}

	LPCTSTR CDynamicListUI::GetItemBkImage() const
	{
		return m_ListInfo.sBkImage;
	}

	bool CDynamicListUI::IsAlternateBk() const
	{
		return m_ListInfo.bAlternateBk;
	}

	void CDynamicListUI::SetSelectedItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwSelectedTextColor = dwTextColor;
		Invalidate();
	}

	void CDynamicListUI::SetSelectedItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwSelectedBkColor = dwBkColor;
		Invalidate();
	}

	void CDynamicListUI::SetSelectedItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sSelectedImage = pStrImage;
		Invalidate();
	}

	DWORD CDynamicListUI::GetSelectedItemTextColor() const
	{
		return m_ListInfo.dwSelectedTextColor;
	}

	DWORD CDynamicListUI::GetSelectedItemBkColor() const
	{
		return m_ListInfo.dwSelectedBkColor;
	}

	LPCTSTR CDynamicListUI::GetSelectedItemImage() const
	{
		return m_ListInfo.sSelectedImage;
	}

	void CDynamicListUI::SetHotItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwHotTextColor = dwTextColor;
		Invalidate();
	}

	void CDynamicListUI::SetHotItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwHotBkColor = dwBkColor;
		Invalidate();
	}

	void CDynamicListUI::SetHotItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sHotImage = pStrImage;
		Invalidate();
	}

	DWORD CDynamicListUI::GetHotItemTextColor() const
	{
		return m_ListInfo.dwHotTextColor;
	}
	DWORD CDynamicListUI::GetHotItemBkColor() const
	{
		return m_ListInfo.dwHotBkColor;
	}

	LPCTSTR CDynamicListUI::GetHotItemImage() const
	{
		return m_ListInfo.sHotImage;
	}

	void CDynamicListUI::SetDisabledItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	void CDynamicListUI::SetDisabledItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwDisabledBkColor = dwBkColor;
		Invalidate();
	}

	void CDynamicListUI::SetDisabledItemImage(LPCTSTR pStrImage)
	{
		m_ListInfo.sDisabledImage = pStrImage;
		Invalidate();
	}

	DWORD CDynamicListUI::GetDisabledItemTextColor() const
	{
		return m_ListInfo.dwDisabledTextColor;
	}

	DWORD CDynamicListUI::GetDisabledItemBkColor() const
	{
		return m_ListInfo.dwDisabledBkColor;
	}

	LPCTSTR CDynamicListUI::GetDisabledItemImage() const
	{
		return m_ListInfo.sDisabledImage;
	}

	DWORD CDynamicListUI::GetItemLineColor() const
	{
		return m_ListInfo.dwLineColor;
	}

	void CDynamicListUI::SetItemLineColor(DWORD dwLineColor)
	{
		m_ListInfo.dwLineColor = dwLineColor;
		Invalidate();
	}

	bool CDynamicListUI::IsItemShowHtml()
	{
		return m_ListInfo.bShowHtml;
	}

	void CDynamicListUI::SetItemShowHtml(bool bShowHtml)
	{
		if( m_ListInfo.bShowHtml == bShowHtml ) return;

		m_ListInfo.bShowHtml = bShowHtml;
		NeedUpdate();
	}

	void CDynamicListUI::SetMultiExpanding(bool bMultiExpandable)
	{
		m_ListInfo.bMultiExpandable = bMultiExpandable;
	}

	bool CDynamicListUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
	{
		if( m_iExpandedItem >= 0 && !m_ListInfo.bMultiExpandable) {
			CControlUI* pControl = GetItemAt(m_iExpandedItem);
			if( pControl != NULL ) {
				IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if( pItem != NULL ) pItem->Expand(false);
			}
			m_iExpandedItem = -1;
		}
		if( bExpand ) {
			CControlUI* pControl = GetItemAt(iIndex);
			if( pControl == NULL ) return false;
			if( !pControl->IsVisible() ) return false;
			IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pItem == NULL ) return false;
			m_iExpandedItem = iIndex;
			if( !pItem->Expand(true) ) {
				m_iExpandedItem = -1;
				return false;
			}
		}
		NeedUpdate();
		return true;
	}

	int CDynamicListUI::GetExpandedItem() const
	{
		return m_iExpandedItem;
	}

	void CDynamicListUI::EnsureVisible(int iIndex)
	{

		RECT rcItem = m_pList->GetItemAt(iIndex)->GetPos();
		RECT rcList = m_pList->GetPos();
		RECT rcListInset = m_pList->GetInset();

		rcList.left += rcListInset.left;
		rcList.top += rcListInset.top;
		rcList.right -= rcListInset.right;
		rcList.bottom -= rcListInset.bottom;

		CScrollBarUI* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
		if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

		int iPos = m_pList->GetScrollPos().cy;
		if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
		int dx = 0;
		if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
		if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
		Scroll(0, dx);
	}

	// 相对当前显示区域移动多少个
	void CDynamicListUI::EnsureUpdateVisible(int iIndex)
	{
		RECT rcCurFirstItem = m_pList->GetItemAt(0)->GetPos();
		RECT rcItem;

		rcItem.top = rcCurFirstItem.top + (rcCurFirstItem.bottom - rcCurFirstItem.top)* (iIndex);
		rcItem.bottom = rcItem.top + rcCurFirstItem.bottom - rcCurFirstItem.top;
		RECT rcList = m_pList->GetPos();
		RECT rcListInset = m_pList->GetInset();

		rcList.left += rcListInset.left;
		rcList.top += rcListInset.top;
		rcList.right -= rcListInset.right;
		rcList.bottom -= rcListInset.bottom;

		CScrollBarUI* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
		if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();

		int iPos = m_pList->GetScrollPos().cy;
		if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
		int dx = 0;
		if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
		if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
		Scroll(0, dx);
	}

	void CDynamicListUI::Scroll(int dx, int dy)
	{
		if( dx == 0 && dy == 0 ) return;
		SIZE sz = m_pList->GetScrollPos();
		m_pList->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
	}

	void CDynamicListUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("header")) == 0 ) GetHeader()->SetVisible(_tcscmp(pstrValue, _T("hidden")) != 0);
		else if( _tcscmp(pstrName, _T("headerbkimage")) == 0 ) GetHeader()->SetBkImage(pstrValue);
		else if( _tcscmp(pstrName, _T("scrollselect")) == 0 ) SetScrollSelect(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("multiexpanding")) == 0 ) SetMultiExpanding(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("multipleitem")) == 0 ) SetMultipleItem(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("showvline")) == 0 ) SetShowVLine(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("showhline")) == 0 ) SetShowHLine(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("itemfont")) == 0 ) m_ListInfo.nFont = _ttoi(pstrValue);
		else if( _tcscmp(pstrName, _T("itemalign")) == 0 ) {
			if( _tcsstr(pstrValue, _T("left")) != NULL ) {
				m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_ListInfo.uTextStyle |= DT_LEFT;
			}
			if( _tcsstr(pstrValue, _T("center")) != NULL ) {
				m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
				m_ListInfo.uTextStyle |= DT_CENTER;
			}
			if( _tcsstr(pstrValue, _T("right")) != NULL ) {
				m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_ListInfo.uTextStyle |= DT_RIGHT;
			}
		}
		else if( _tcscmp(pstrName, _T("itemendellipsis")) == 0 ) {
			if( _tcscmp(pstrValue, _T("true")) == 0 ) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
			else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
		}    
		else if( _tcscmp(pstrName, _T("itemtextpadding")) == 0 ) {
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
			SetItemTextPadding(rcTextPadding);
		}
		else if( _tcscmp(pstrName, _T("itemcheckimgsize")) == 0 ) {
			SIZE szCheckImg;
			LPTSTR pstr = NULL;
			szCheckImg.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			szCheckImg.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    		
			SetCheckImgSize(szCheckImg);
		}
		else if( _tcscmp(pstrName, _T("itemiconimgsize")) == 0 ) {
			SIZE szIconImg;
			LPTSTR pstr = NULL;
			szIconImg.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			szIconImg.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    		
			SetIconImgSize(szIconImg);
		}
		else if( _tcscmp(pstrName, _T("itemtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetItemTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itembkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetItemBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itembkimage")) == 0 ) SetItemBkImage(pstrValue);
		else if( _tcscmp(pstrName, _T("itemaltbk")) == 0 ) SetAlternateBk(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("itemselectedtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedItemTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itemselectedbkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetSelectedItemBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itemselectedimage")) == 0 ) SetSelectedItemImage(pstrValue);
		else if( _tcscmp(pstrName, _T("itemhottextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotItemTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itemhotbkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetHotItemBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itemhotimage")) == 0 ) SetHotItemImage(pstrValue);
		else if( _tcscmp(pstrName, _T("itemdisabledtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledItemTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itemdisabledbkcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledItemBkColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itemdisabledimage")) == 0 ) SetDisabledItemImage(pstrValue);
		else if( _tcscmp(pstrName, _T("itemlinecolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetItemLineColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("itemshowhtml")) == 0 ) SetItemShowHtml(_tcscmp(pstrValue, _T("true")) == 0);
		else CVerticalLayoutUI::SetAttribute(pstrName, pstrValue);
	}

	IListCallbackUI* CDynamicListUI::GetTextCallback() const
	{
		return m_pCallback;
	}

	void CDynamicListUI::SetTextCallback(IListCallbackUI* pCallback)
	{
		m_pCallback = pCallback;
	}

	void CDynamicListUI::SetItemCallback(IDynamicListCallbackUI* pItemCallback)
	{
		m_pItemCallback = pItemCallback;
	}

	IDynamicListCallbackUI* CDynamicListUI::GetItemCallback() const
	{
		return m_pItemCallback;
	}

	SIZE CDynamicListUI::GetScrollPos() const
	{
		return m_pList->GetScrollPos();
	}

	SIZE CDynamicListUI::GetScrollRange() const
	{
		return m_pList->GetScrollRange();
	}

	void CDynamicListUI::SetScrollPos(SIZE szPos)
	{
		m_pList->SetScrollPos(szPos);
	}

	void CDynamicListUI::LineUp()
	{
		m_pList->LineUp();
	}

	void CDynamicListUI::LineDown()
	{
		m_pList->LineDown();
	}

	void CDynamicListUI::PageUp()
	{
		m_pList->PageUp();
	}

	void CDynamicListUI::PageDown()
	{
		m_pList->PageDown();
	}

	void CDynamicListUI::HomeUp()
	{
		m_pList->HomeUp();
	}

	void CDynamicListUI::EndDown()
	{
		m_pList->EndDown();
	}

	void CDynamicListUI::LineLeft()
	{
		m_pList->LineLeft();
	}

	void CDynamicListUI::LineRight()
	{
		m_pList->LineRight();
	}

	void CDynamicListUI::PageLeft()
	{
		m_pList->PageLeft();
	}

	void CDynamicListUI::PageRight()
	{
		m_pList->PageRight();
	}

	void CDynamicListUI::HomeLeft()
	{
		m_pList->HomeLeft();
	}

	void CDynamicListUI::EndRight()
	{
		m_pList->EndRight();
	}

	void CDynamicListUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
	{
		m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
	}

	CScrollBarUI* CDynamicListUI::GetVerticalScrollBar() const
	{
		return m_pList->GetVerticalScrollBar();
	}

	CScrollBarUI* CDynamicListUI::GetHorizontalScrollBar() const
	{
		return m_pList->GetHorizontalScrollBar();
	}

	bool CDynamicListUI::SelectMultiItem( int iIndex, bool bTakeFocus /*= false*/ )
	{
			if (m_bSingleSel)
			{
				return SelectItem(iIndex, bTakeFocus);
			}

			if( iIndex < 0 ) return false;
			CControlUI* pControl = GetItemAt(iIndex);
			if( pControl == NULL ) return false;
			if( !pControl->IsVisible() ) return false;
			if( !pControl->IsEnabled() ) return false;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL ) return false;

			if (m_aSelItems.Find((LPVOID)iIndex) >= 0)
				return false;

			if(m_bSingleSel && m_aSelItems.GetSize() > 0) {
				CControlUI* pControl = GetItemAt((int)m_aSelItems.GetAt(0));
				if( pControl != NULL) {
					IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
					if( pListItem != NULL ) pListItem->Select(false);
				}		
			}	

			if( !pListItem->Select(true) ) {		
				return false;
			}

			m_aSelItems.Add((LPVOID)iIndex);

			EnsureVisible(iIndex);
			if( bTakeFocus ) pControl->SetFocus();
			if( m_pManager != NULL ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex);
			}

			return true;
	}

	void CDynamicListUI::SetSingleSelect( bool bSingleSel )
	{
			m_bSingleSel = bSingleSel;
			UnSelectAllItems();
	}

	bool CDynamicListUI::GetSingleSelect() const
	{
			return m_bSingleSel;
	}

	bool CDynamicListUI::UnSelectItem( int iIndex )
	{
			if( iIndex < 0 ) return false;
			CControlUI* pControl = GetItemAt(iIndex);
			if( pControl == NULL ) return false;
			if( !pControl->IsVisible() ) return false;
			if( !pControl->IsEnabled() ) return false;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL ) return false;

			int aIndex = m_aSelItems.Find((LPVOID)iIndex);
			if (aIndex < 0)
				return false;

			if( !pListItem->Select(false) ) {		
				return false;
			}

			m_aSelItems.Remove(aIndex);
			return true;
	}

	bool CDynamicListUI::RemoveSelItemIndex(int iIndex)
	{
		m_aSelItemIndex.Remove(m_aSelItemIndex.Find((LPVOID)(iIndex + GetCurFirstItem())));
		m_nLastSelect = iIndex + GetCurFirstItem();
		m_bSelRange = false;
		return true;
	}

	void CDynamicListUI::SelectAllItems()
	{
			UnSelectAllItems();
			CControlUI* pControl;
			for (int i = 0; i < GetCount(); ++i)
			{
				pControl = GetItemAt(i);
				if(pControl == NULL)
					continue;
				if(!pControl->IsVisible())
					continue;
				if(!pControl->IsEnabled())
					continue;
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if( pListItem == NULL )
					continue;
				if( !pListItem->Select(true) )
					continue;
				m_aSelItems.Add((LPVOID)i);
			}
	}

	void CDynamicListUI::SelectItemsRange(int nRangeBegin, int nRangeEnd)
	{
		UnSelectAllItems();
		m_nRangeBegin = nRangeBegin;
		m_nRangeEnd = nRangeEnd;

		if (m_nRangeBegin >= m_nRangeEnd)
		{
			nRangeBegin = m_nRangeEnd;
			nRangeEnd = m_nRangeBegin;
		}

		m_bSelRange = true;
		int nFirstItem = GetCurFirstItem();
		for (; nRangeBegin <= nRangeEnd; ++nRangeBegin)
		{
			// 需要选中的不在当前区域 也需要将下标放入m_aSelItemIndex（如按住shift拖动滑块）
			if (nRangeBegin - nFirstItem < 0 || nRangeBegin >= (nFirstItem + GetCount()))
			{
				int pos = m_aSelItemIndex.Find(LPVOID(nRangeBegin));

				if (pos == -1)
					m_aSelItemIndex.Add(LPVOID(nRangeBegin));
				continue;
			}

			CControlUI* pControl = GetItemAt(nRangeBegin - nFirstItem);
			if(pControl == NULL)
				continue;
			if(!pControl->IsVisible())
				continue;
			if(!pControl->IsEnabled())
				continue;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));

			if (pListItem)
				pListItem->Select(true, false);
		}
	}

	void CDynamicListUI::UnSelectAllItems()
	{
			CControlUI* pControl;
			int itemIndex;
			for (int i = 0; i < m_aSelItems.GetSize(); ++i)
			{
				itemIndex = (int)m_aSelItems.GetAt(i);
				pControl = GetItemAt(itemIndex);
				if(pControl == NULL)
					continue;
				if(!pControl->IsVisible())
					continue;
				if(!pControl->IsEnabled())
					continue;
				IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if( pListItem == NULL )
					continue;
				if( !pListItem->Select(false) )
					continue;		
			}
			m_aSelItems.Empty();

			m_aSelItemIndex.Empty();
			m_nRangeBegin = -1;
			m_nRangeEnd = -1;
			m_bSelRange = false;
	}

	void CDynamicListUI::UnSelectOtherItems(int nIndex)
	{
		CControlUI* pControl;
		int itemIndex;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			itemIndex = (int)m_aSelItems.GetAt(i);

			if (itemIndex == nIndex)
				continue;

			pControl = GetItemAt(itemIndex);
			if(pControl == NULL)
				continue;
			if(!pControl->IsVisible())
				continue;
			if(!pControl->IsEnabled())
				continue;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem == NULL )
				continue;
			if( !pListItem->Select(false) )
				continue;
		}
		m_aSelItems.Empty();
		m_aSelItems.Add((LPVOID)nIndex);
		m_aSelItemIndex.Empty();
		m_aSelItemIndex.Add(LPVOID(m_pList->GetCurFirstItem() + nIndex));

		m_nRangeBegin = -1;
		m_nRangeEnd = -1;
		m_bSelRange = false;
		m_nLastSelect = m_pList->GetCurFirstItem() + nIndex;
	}

	int CDynamicListUI::GetSelectItemCount() const
	{
		return m_aSelItems.GetSize();
	}

	int CDynamicListUI::GetSelectItemTotalCount() const
	{
		return m_aSelItemIndex.GetSize();
	}

	int CDynamicListUI::GetNextSelItem( int nItem ) const
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;

		if (nItem < 0)
		{
			return (int)m_aSelItems.GetAt(0);
		}
		int aIndex = m_aSelItems.Find((LPVOID)nItem);
		if (aIndex < 0)
			return -1;
		if (aIndex + 1 > m_aSelItems.GetSize() - 1)
			return -1;
		return (int)m_aSelItems.GetAt(aIndex + 1);
	}

	int CDynamicListUI::GetSelItemByIndex(int nIndex) const
	{
		if (nIndex < 0)
			return -1;
		if (nIndex > m_aSelItemIndex.GetSize() - 1)
			return -1;

		return (int)m_aSelItemIndex.GetAt(nIndex);
	}

	void CDynamicListUI::SetCheckImgSize( SIZE szCheckImg )
	{
		m_ListInfo.szCheckImg = szCheckImg;
	}

	void CDynamicListUI::SetIconImgSize( SIZE szIconImg )
	{
			m_ListInfo.szIconImg = szIconImg;
	}

	void CDynamicListUI::SetShowVLine( bool bVLine )
	{
		m_ListInfo.bShowVLine = bVLine;
	}

	void CDynamicListUI::SetShowHLine( bool bHLine )
	{
		m_ListInfo.bShowHLine = bHLine;
	}

	SIZE CDynamicListUI::GetCheckImgSize() const
	{
		return m_ListInfo.szCheckImg;
	}

	SIZE CDynamicListUI::GetIconImgSize() const
	{
			return m_ListInfo.szIconImg;
	}

	bool CDynamicListUI::IsShowVLine() const
	{
		return m_ListInfo.bShowVLine;
	}

	bool CDynamicListUI::IsShowHLine() const
	{
		return m_ListInfo.bShowHLine;
	}

	BOOL CDynamicListUI::SortItems( PULVCompareFunc pfnCompare, UINT_PTR dwData )
	{
			if (!m_pList)
				return FALSE;
			return m_pList->SortItems(pfnCompare, dwData);	
	}

	CStdPtrArray CDynamicListUI::GetCurSelItemIndex()
	{
		return m_aSelItemIndex;
	}

	void CDynamicListUI::SetMultipleItem( bool bMultipleable )
	{
			m_bSingleSel = !bMultipleable;
	}

	bool CDynamicListUI::GetMultipleItem() const
	{
		return !m_bSingleSel;
	}



	/////////////////////////////////////////////////////////////////////////////////////
	//
	//


	CDynamicListBodyUI::CDynamicListBodyUI(CDynamicListUI* pOwner) : m_pOwner(pOwner), m_totalSize(0), m_totalHeight(0)
		, m_perHeight(0), m_curfirstItem(0), m_cyOffset(0)
	{
		ASSERT(m_pOwner);
	}

	void CDynamicListBodyUI::SetScrollPos(SIZE szPos)
	{
		if (m_perHeight == 0)
			return;

		int cx = 0;
		int cy = 0;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if( cx != 0 && m_pOwner ) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if( pHeader == NULL ) return;
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			pInfo->nColumns = MIN(pHeader->GetCount(), UILIST_MAX_COLUMNS);

			if( !pHeader->IsVisible() ) {
				for( int it = 0; it < pHeader->GetCount(); it++ ) {
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
				}
			}
			for( int i = 0; i < pInfo->nColumns; i++ ) {
				CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
				if( !pControl->IsVisible() ) continue;
				if( pControl->IsFloat() ) continue;

				RECT rcPos = pControl->GetPos();
				rcPos.left -= cx;
				rcPos.right -= cx;
				pControl->SetPos(rcPos);
				pInfo->rcColumn[i] = pControl->GetPos();
			}
			if( !pHeader->IsVisible() ) {
				for( int it = 0; it < pHeader->GetCount(); it++ ) {
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
				}
			}
		}
		
		if (m_pOwner)
		{
			if ((m_rcItem.bottom - m_rcItem.top) % m_perHeight == 0)
				m_totalHeight = (m_rcItem.bottom - m_rcItem.top) / m_perHeight + 1;
			else
				m_totalHeight = (m_rcItem.bottom - m_rcItem.top/* + m_perHeight*/) / m_perHeight + 1;

			int needItem = 0;
			int offset = cy % m_perHeight;
			
			if (cy > 0)
			{
				needItem = (cy + m_cyOffset) / m_perHeight;
				m_cyOffset = (cy + m_cyOffset) % m_perHeight;

				IDynamicListCallbackUI * pCallback = m_pOwner->GetItemCallback();
				RemoveAll(); //现在items 不支持移动先全部清掉，后面改成之清掉需要的
				m_curfirstItem = m_curfirstItem + needItem;
				cy = offset;

				for (int i = m_curfirstItem; i < m_curfirstItem + m_totalHeight; ++i)
				{
					CControlUI* pControl = pCallback->GetItem(i);
					CStdPtrArray aSelItems = m_pOwner->GetCurSelItemIndex();
					int nSelItemCount = aSelItems.GetSize();

					for (int index = 0; index < nSelItemCount; ++index)
					{
						int itemIndex = (int)aSelItems.GetAt(index);

						if (i == itemIndex)
						{
							IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
							pListItem->Select(true, false, true);
						}
					}
				}
			}

			if (cy < 0)
			{
				needItem = (abs(cy + m_cyOffset))/ m_perHeight;
				if (abs(cy + m_cyOffset) % m_perHeight != 0)
					needItem += 1;
				m_cyOffset = (cy + m_cyOffset) % m_perHeight;
				if (m_cyOffset < 0)
					m_cyOffset += m_perHeight;

				IDynamicListCallbackUI * pCallback = m_pOwner->GetItemCallback();
				RemoveAll(); //现在items 不支持移动先全部清掉，后面改成之清掉需要的

				m_curfirstItem = m_curfirstItem - needItem;

				if (m_curfirstItem < 0)
					m_curfirstItem = 0;

				cy = offset;

				for (int i = m_curfirstItem; i < m_curfirstItem + m_totalHeight; ++i)
				{
					CControlUI* pControl = pCallback->GetItem(i);
					CStdPtrArray aSelItems = m_pOwner->GetCurSelItemIndex();
					int nSelItemCount = aSelItems.GetSize();

					for (int index = 0; index < nSelItemCount; ++index)
					{
						int itemIndex = (int)aSelItems.GetAt(index);

						if (i == itemIndex)
						{
							IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
							pListItem->Select(true, false, true);
						}
					}
				}
			}

			if (cy == 0)
			{
				//DLOG(INFO)<<"m_curfirstItem: "<<m_curfirstItem;
				IDynamicListCallbackUI * pCallback = m_pOwner->GetItemCallback();
				RemoveAll(); //现在items 不支持移动先全部清掉，后面改成之清掉需要的
			
				if (m_curfirstItem + m_totalHeight > m_totalSize && m_curfirstItem >= 1)
				{
					for (int i = m_totalSize - m_totalHeight + 1; i < m_totalSize; ++i)
					{
						CControlUI* pControl = pCallback->GetItem(i);
					}

					m_curfirstItem = m_totalSize - m_totalHeight + 1;
				}
				else
				{
					for (int i = m_curfirstItem; i < m_curfirstItem + m_totalHeight; ++i)
					{
						CControlUI* pControl = pCallback->GetItem(i);
						CStdPtrArray aSelItems = m_pOwner->GetCurSelItemIndex();
						int nSelItemCount = aSelItems.GetSize();

						for (int index = 0; index < nSelItemCount; ++index)
						{
							int itemIndex = (int)aSelItems.GetAt(index);

							if (i == itemIndex)
							{
								IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
								pListItem->Select(true, false, true);
							}
						}
					}
				}
			}
		}
		
		RECT rcPos;
		for( int it2 = 0; it2 < m_items.GetSize(); it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;

			rcPos = pControl->GetPos();
			rcPos.left -= cx;
			rcPos.right -= cx;
			rcPos.top -= cy;
			rcPos.bottom -= cy;
			pControl->SetPos(rcPos);
		}

		Invalidate();


	}

	void CDynamicListBodyUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
		rc = m_rcItem;
		// Adjust for inset
		rc.left += m_rcInset.left;
		rc.top += m_rcInset.top;
		rc.right -= m_rcInset.right;
		rc.bottom -= m_rcInset.bottom;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int cxNeeded = 0;
		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimateNum = 0;
		for( int it1 = 0; it1 < m_items.GetSize(); it1++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if( sz.cy == 0 ) {
				nAdjustables++;
			}
			else {
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			}
			cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

			RECT rcPadding = pControl->GetPadding();
			sz.cx = MAX(sz.cx, 0L);
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			cxNeeded = MAX((LONG)cxNeeded, sz.cx);
			nEstimateNum++;
		}
		cyFixed += (nEstimateNum - 1) * m_iChildPadding;

		if( m_pOwner ) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if( pHeader != NULL && pHeader->GetCount() > 0 ) {
				cxNeeded = MAX(0L, pHeader->EstimateSize(CSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
			}
		}

		// Place elements
		int cyNeeded = 0;
		int cyExpand = 0;
		if( nAdjustables > 0 ) cyExpand = MAX(0L, (szAvailable.cy - cyFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosY = rc.top;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			iPosY -= m_pVerticalScrollBar->GetScrollPos();
		}
		int iPosX = rc.left;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;
		for( int it2 = 0; it2 < m_totalSize/*m_items.GetSize()*/; it2++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items.GetAt(0));
			
			if (pControl == NULL)
				return;
			
			if( !pControl->IsVisible() ) continue;
			if (it2 < m_items.GetSize())
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
				if (pControl->IsFloat())
				{
					SetFloatPos(it2);
					continue;
				}
			}

			RECT rcPadding = pControl->GetPadding();
			szRemaining.cy -= rcPadding.top;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if( sz.cy == 0 ) {
				iAdjustable++;
				sz.cy = cyExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if( iAdjustable == nAdjustables ) {
					sz.cy = MAX(0L, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
				} 
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			}
			else {
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
				cyFixedRemaining -= sz.cy;
			}

			sz.cx = MAX((LONG)cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);

			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			
			if (it2 - m_curfirstItem == 0)
			{
				iPosY = (iPosY - rc.top) % sz.cy + rc.top; //暂时写死，时间比较紧，后面改
			}

			RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
			if (it2 - m_curfirstItem < m_items.GetSize() && it2 - m_curfirstItem >= 0)
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[it2 - m_curfirstItem]);
				pControl->SetPos(rcCtrl);
			}
			

			iPosY += sz.cy + m_iChildPadding + rcPadding.top + rcPadding.bottom;
			cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
			szRemaining.cy -= sz.cy + m_iChildPadding + rcPadding.bottom;
		}
		cyNeeded += (nEstimateNum - 1) * m_iChildPadding;
		
		if( m_pHorizontalScrollBar != NULL ) {
			if( cxNeeded > rc.right - rc.left ) {
				if( m_pHorizontalScrollBar->IsVisible() ) {
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
				}
				else {
					m_pHorizontalScrollBar->SetVisible(true);
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
			else {
				if( m_pHorizontalScrollBar->IsVisible() ) {
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollRange(0);
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
		}

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}

	void CDynamicListBodyUI::ProcessScrollBar(RECT rc, int cxRequired, int cyRequired)
	{
		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) {
			RECT rcScrollBarPos = { rc.left, rc.bottom, rc.right, rc.bottom + m_pHorizontalScrollBar->GetFixedHeight()};
			m_pHorizontalScrollBar->SetPos(rcScrollBarPos);
		}

		if( m_pVerticalScrollBar == NULL ) return;

		if( cyRequired > rc.bottom - rc.top && !m_pVerticalScrollBar->IsVisible() ) {
			m_pVerticalScrollBar->SetVisible(true);
			m_pVerticalScrollBar->SetScrollRange(cyRequired - (rc.bottom - rc.top));
			m_pVerticalScrollBar->SetScrollPos(0);
			m_bScrollProcess = true;
			SetPos(m_rcItem);
			m_bScrollProcess = false;
			return;
		}
		// No scrollbar required
		if( !m_pVerticalScrollBar->IsVisible() ) return;

		// Scroll not needed anymore?
		int cyScroll = cyRequired - (rc.bottom - rc.top);
		if( cyScroll <= 0 && !m_bScrollProcess) {
			m_pVerticalScrollBar->SetVisible(false);
			m_pVerticalScrollBar->SetScrollPos(0);
			m_pVerticalScrollBar->SetScrollRange(0);
			SetPos(m_rcItem);
		}
		else
		{
			RECT rcScrollBarPos = { rc.right, rc.top, rc.right + m_pVerticalScrollBar->GetFixedWidth(), rc.bottom };
			m_pVerticalScrollBar->SetPos(rcScrollBarPos);

			if( m_pVerticalScrollBar->GetScrollRange() != cyScroll ) {
				int iScrollPos = m_pVerticalScrollBar->GetScrollPos();
				m_pVerticalScrollBar->SetScrollRange(::abs(cyScroll));
				if( m_pVerticalScrollBar->GetScrollRange() == 0 ) {
					m_pVerticalScrollBar->SetVisible(false);
					m_pVerticalScrollBar->SetScrollPos(0);
				}
				if( iScrollPos > m_pVerticalScrollBar->GetScrollPos() ) {
					SetPos(m_rcItem);
				}
			}
		}
	}

	void CDynamicListBodyUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	int __cdecl CDynamicListBodyUI::ItemComareFunc( const void *item1, const void *item2 )
	{
		CControlUI *pControl1 = *(CControlUI**)item1;
		CControlUI *pControl2 = *(CControlUI**)item2;
		return m_pCompareFunc((UINT_PTR)pControl1, (UINT_PTR)pControl2, m_compareData);
	}

	int __cdecl CDynamicListBodyUI::ItemComareFunc( void *pvlocale, const void *item1, const void *item2 )
	{
		CDynamicListBodyUI *pThis = (CDynamicListBodyUI*)pvlocale;
		if (!pThis || !item1 || !item2)
			return 0;
		return pThis->ItemComareFunc(item1, item2);
	}

	BOOL CDynamicListBodyUI::SortItems( PULVCompareFunc pfnCompare, UINT_PTR dwData )
	{
		if (!pfnCompare)
			return FALSE;
		m_pCompareFunc = pfnCompare;
		CControlUI **pData = (CControlUI **)m_items.GetData();
		qsort_s(m_items.GetData(), m_items.GetSize(), sizeof(CControlUI*), CDynamicListBodyUI::ItemComareFunc, this);	
		IListItemUI *pItem = NULL;
		for (int i = 0; i < m_items.GetSize(); ++i)
		{
			pItem = (IListItemUI*)(static_cast<CControlUI*>(m_items[i])->GetInterface(TEXT("ListItem")));
			if (pItem)
			{
				pItem->SetIndex(i);
				pItem->Select(false);
			}
		}
		m_pOwner->SelectItem(-1);
		if (m_pManager)
		{
			SetPos(GetPos());
			Invalidate();
		}

		return TRUE;
	}

	void CDynamicListBodyUI::SetDynamicAttr(int totalSize, int perHeight, bool bOnlyUpdate)
	{
		m_totalSize = totalSize;
		m_perHeight = perHeight;

		if (bOnlyUpdate)
			return;

		m_curfirstItem = 0;
		m_cyOffset = 0;
		m_pVerticalScrollBar->SetScrollPos(0);

		if (m_pOwner)
			m_pOwner->UnSelectAllItems();
	}

	int CDynamicListBodyUI::GetTotalCount()
	{
		return m_totalSize;
	}

	int CDynamicListBodyUI::GetCurFirstItem()
	{
		return m_curfirstItem;
	}

	int CDynamicListBodyUI::GetPerHeight()
	{
		return m_perHeight;
	}

	int CDynamicListBodyUI::GetTotalHeight()
	{
		return m_totalHeight;
	}

	void CDynamicListBodyUI::Update(int from /* = 0 */)
	{
		int dy = (m_perHeight)* (from - m_curfirstItem);
		SIZE sz = GetScrollPos();
		SetScrollPos(CSize(sz.cx, sz.cy + dy));
	}

} // namespace UiLib
