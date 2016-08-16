#ifndef __UIDYNAMICILIST_H__
#define __UIDYNAMICILIST_H__

#pragma once
#include "layout/uiverticallayout.h"
#include "layout/uihorizontallayout.h"
#include "uilist_i.h"

namespace UiLib {
/////////////////////////////////////////////////////////////////////////////////////
//
class IDynamicListCallbackUI
{
public:
	virtual CControlUI* GetItem(int iItem) = 0;
};

class CDynamicListBodyUI;
class CListHeaderUI;

class UILIB_API CDynamicListUI : public CVerticalLayoutUI, public IListUI
{
public:
    CDynamicListUI();
	virtual ~CDynamicListUI();
	void SetDynamicAttr(int totalSize, int perHeight, bool bOnlyUpdate = false);
	void UpdateItem(int form = 0);
	void UpdateItemOnly();
    LPCTSTR GetClass() const;
    UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    bool GetScrollSelect();
    void SetScrollSelect(bool bScrollSelect);
    int GetCurSel() const;
	int GetLastSel() const;
    bool SelectItem(int iIndex, bool bTakeFocus = false, bool bUnSelectOther = true, bool bSelected = false);

	bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
	void SetSingleSelect(bool bSingleSel);
	bool GetSingleSelect() const;
	bool UnSelectItem(int iIndex);
	bool RemoveSelItemIndex(int iIndex);
	void SelectAllItems();
	void SelectItemsRange(int nRangeBegin, int nRangeEnd);
	void UnSelectAllItems();
	void UnSelectOtherItems(int nIndex);
	int GetSelectItemCount() const;
	int GetSelectItemTotalCount() const;
	int GetNextSelItem(int nItem) const;
	int GetSelItemByIndex(int nIndex) const;

    CListHeaderUI* GetHeader() const;  
    CContainerUI* GetList() const;
    TListInfoUI* GetListInfo();

    CControlUI* GetItemAt(int iIndex) const;
	CControlUI* GetItemInDList(int iIndex) const;
    int GetItemIndex(CControlUI* pControl) const;
    bool SetItemIndex(CControlUI* pControl, int iIndex);
    int GetCount() const;
	int GetTotalCount() const;
	int GetCurFirstItem() const;
    bool Add(CControlUI* pControl);
    bool AddAt(CControlUI* pControl, int iIndex);
    bool Remove(CControlUI* pControl);
    bool RemoveAt(int iIndex);
    void RemoveAll();

    void EnsureVisible(int iIndex);
	void EnsureUpdateVisible(int iIndex);
    void Scroll(int dx, int dy);

	bool IsDelayedDestroy() const;
	void SetDelayedDestroy(bool bDelayed);
    int GetChildPadding() const;
    void SetChildPadding(int iPadding);

    void SetItemFont(int index);
    void SetItemTextStyle(UINT uStyle);
    void SetItemTextPadding(RECT rc);
    void SetItemTextColor(DWORD dwTextColor);
    void SetItemBkColor(DWORD dwBkColor);
    void SetItemBkImage(LPCTSTR pStrImage);
    void SetAlternateBk(bool bAlternateBk);
    void SetSelectedItemTextColor(DWORD dwTextColor);
    void SetSelectedItemBkColor(DWORD dwBkColor);
    void SetSelectedItemImage(LPCTSTR pStrImage); 
    void SetHotItemTextColor(DWORD dwTextColor);
    void SetHotItemBkColor(DWORD dwBkColor);
    void SetHotItemImage(LPCTSTR pStrImage);
    void SetDisabledItemTextColor(DWORD dwTextColor);
    void SetDisabledItemBkColor(DWORD dwBkColor);
    void SetDisabledItemImage(LPCTSTR pStrImage);
	void SetItemLineColor(DWORD dwLineColor);
	void SetCheckImgSize(SIZE szCheckImg);
	void SetIconImgSize(SIZE szIconImg);
	void SetShowVLine(bool bVLine);
	void SetShowHLine(bool bHLine);
    bool IsItemShowHtml();
    void SetItemShowHtml(bool bShowHtml = true);
	RECT GetItemTextPadding() const;
	DWORD GetItemTextColor() const;
	DWORD GetItemBkColor() const;
	LPCTSTR GetItemBkImage() const;
    bool IsAlternateBk() const;
	DWORD GetSelectedItemTextColor() const;
	DWORD GetSelectedItemBkColor() const;
	LPCTSTR GetSelectedItemImage() const;
	DWORD GetHotItemTextColor() const;
	DWORD GetHotItemBkColor() const;
	LPCTSTR GetHotItemImage() const;
	DWORD GetDisabledItemTextColor() const;
	DWORD GetDisabledItemBkColor() const;
	LPCTSTR GetDisabledItemImage() const;
	DWORD GetItemLineColor() const;
	SIZE GetCheckImgSize() const;
	SIZE GetIconImgSize() const;
	bool IsShowVLine() const;
	bool IsShowHLine() const;

    void SetMultiExpanding(bool bMultiExpandable);
	int GetExpandedItem() const;
	void SetMultipleItem(bool bMultipleable);
	bool GetMultipleItem() const;
    bool ExpandItem(int iIndex, bool bExpand = true);

	void SetPos(RECT rc);
    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    IListCallbackUI* GetTextCallback() const;
    void SetTextCallback(IListCallbackUI* pCallback);

	IDynamicListCallbackUI* GetItemCallback() const;
	void SetItemCallback(IDynamicListCallbackUI* pItemCallback);

    SIZE GetScrollPos() const;
    SIZE GetScrollRange() const;
    void SetScrollPos(SIZE szPos);
    void LineUp();
    void LineDown();
    void PageUp();
    void PageDown();
    void HomeUp();
    void EndDown();
    void LineLeft();
    void LineRight();
    void PageLeft();
    void PageRight();
    void HomeLeft();
    void EndRight();
    void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
    virtual CScrollBarUI* GetVerticalScrollBar() const;
    virtual CScrollBarUI* GetHorizontalScrollBar() const;

	BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);
	CStdPtrArray GetCurSelItemIndex();

protected:
	int GetMinSelItemIndex();
	int GetMaxSelItemIndex();

protected:
    bool m_bScrollSelect;
	//int m_iCurSel;
	bool m_bSingleSel;
	CStdPtrArray m_aSelItems;
    int m_iExpandedItem;
    IListCallbackUI* m_pCallback;
	IDynamicListCallbackUI* m_pItemCallback;
    CDynamicListBodyUI* m_pList;
    CListHeaderUI* m_pHeader;
    TListInfoUI m_ListInfo;

protected:
	CStdPtrArray m_aSelItemIndex;
	bool m_bSelRange;
	int m_nRangeBegin;
	int m_nRangeEnd;
	int m_nLastSelect;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CDynamicListBodyUI : public CVerticalLayoutUI
{
public:
    CDynamicListBodyUI(CDynamicListUI* pOwner);

	void SetDynamicAttr(int totalSize, int perHeight, bool bOnlyUpdate = false);
    void SetScrollPos(SIZE szPos);
    void SetPos(RECT rc);
    void DoEvent(TEventUI& event);
	void ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);

	BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

	int GetTotalCount();
	int GetCurFirstItem();
	int GetPerHeight();
	int GetTotalHeight();
	void Update(int from = 0);

protected:
	static int __cdecl ItemComareFunc(void *pvlocale, const void *item1, const void *item2);
	int __cdecl ItemComareFunc(const void *item1, const void *item2);

protected:
    CDynamicListUI* m_pOwner;
	int m_totalSize;
	int m_totalHeight;
	int m_perHeight;
	int m_curfirstItem;
	int m_cyOffset;
	PULVCompareFunc m_pCompareFunc;
	UINT_PTR m_compareData;
};

} // namespace UiLib

#endif // __UIDYNAMICILIST_H__
