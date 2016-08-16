#ifndef __UILIST_H_H__
#define __UILIST_H_H__

#pragma once
#include "layout/uiverticallayout.h"
#include "layout/uihorizontallayout.h"

namespace UiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

#define ULVHT_NOWHERE				0x0001
#define ULVHT_ITEM					0x0002
#define ULVHT_CHECKBOX				0x0004

typedef int (CALLBACK *PULVCompareFunc)(UINT_PTR, UINT_PTR, UINT_PTR);

class CListHeaderUI;

#define UILIST_MAX_COLUMNS 32

typedef struct tagTListInfoUI
{
    int nColumns;
    RECT rcColumn[UILIST_MAX_COLUMNS];
    int nFont;
	int nSelectedFont;
    UINT uTextStyle;
    RECT rcTextPadding;
    DWORD dwTextColor;
    DWORD dwBkColor;
    CDuiString sBkImage;
    bool bAlternateBk;
    DWORD dwSelectedTextColor;
    DWORD dwSelectedBkColor;
    CDuiString sSelectedImage;
    DWORD dwHotTextColor;
    DWORD dwHotBkColor;
    CDuiString sHotImage;
    DWORD dwDisabledTextColor;
    DWORD dwDisabledBkColor;
    CDuiString sDisabledImage;
    DWORD dwLineColor;
    bool bShowHtml;
    bool bMultiExpandable;
	SIZE szCheckImg;
	SIZE szIconImg;
	bool bShowVLine;
	bool bShowHLine;
} TListInfoUI;


/////////////////////////////////////////////////////////////////////////////////////
//

class IListCallbackUI
{
public:
    virtual LPCTSTR GetItemText(CControlUI* pList, int iItem, int iSubItem) = 0;
};

class IListOwnerUI
{
public:
    virtual TListInfoUI* GetListInfo() = 0;
    virtual int GetCurSel() const = 0;
	virtual int GetLastSel() const = 0;
	virtual int GetCurFirstItem() const = 0;
    virtual bool SelectItem(int iIndex, bool bTakeFocus = false, bool bUnSelectOther = true, bool bSelected = false) = 0;
	virtual void SelectAllItems() = 0;
	virtual void SelectItemsRange(int nRangeBegin, int nRangeEnd) = 0;
	virtual bool RemoveSelItemIndex(int iIndex) = 0;
	virtual void UnSelectAllItems() = 0;
	virtual void UnSelectOtherItems(int iIndex) = 0;
    virtual void DoEvent(TEventUI& event) = 0;
};

class IListUI : public IListOwnerUI
{
public:
    virtual CListHeaderUI* GetHeader() const = 0;
    virtual CContainerUI* GetList() const = 0;
    virtual IListCallbackUI* GetTextCallback() const = 0;
    virtual void SetTextCallback(IListCallbackUI* pCallback) = 0;
    virtual bool ExpandItem(int iIndex, bool bExpand = true) = 0;
    virtual int GetExpandedItem() const = 0;

	virtual bool SelectMultiItem(int iIndex, bool bTakeFocus = false) = 0;
	virtual void SetSingleSelect(bool bSingleSel) = 0;
	virtual bool GetSingleSelect() const = 0;
	virtual bool UnSelectItem(int iIndex) = 0;
	virtual int GetSelectItemCount() const = 0;
	virtual int GetNextSelItem(int nItem) const = 0;
};

class IListItemUI
{
public:
    virtual int GetIndex() const = 0;
    virtual void SetIndex(int iIndex) = 0;
    virtual IListOwnerUI* GetOwner() = 0;
    virtual void SetOwner(CControlUI* pOwner) = 0;
    virtual bool IsSelected() const = 0;
    virtual bool Select(bool bSelect = true, bool bUnSelectAll = true, bool bSelected = false) = 0;
    virtual bool IsExpanded() const = 0;
    virtual bool Expand(bool bExpand = true) = 0;
    virtual void DrawItemText(HDC hDC, const RECT& rcItem) = 0;
};

} // namespace UiLib

#endif // __UILIST_H__
