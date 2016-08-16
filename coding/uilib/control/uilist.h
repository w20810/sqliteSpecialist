#ifndef __UILIST_H__
#define __UILIST_H__

#pragma once
#include "layout/uiverticallayout.h"
#include "layout/uihorizontallayout.h"
#include "uilist_i.h"
#include "uilabel.h"

namespace UiLib {

/////////////////////////////////////////////////////////////////////////////////////
//

class CListBodyUI;
class CListHeaderUI;

class UILIB_API CListUI : public CVerticalLayoutUI, public IListUI
{
public:
    CListUI();

    LPCTSTR GetClass() const;
    UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    bool GetScrollSelect();
    void SetScrollSelect(bool bScrollSelect);
    int GetCurSel() const;
	int GetLastSel() const;
	int GetCurFirstItem() const;
    bool SelectItem(int iIndex, bool bTakeFocus = false, bool bUnSelectOther = true, bool bSelected = false);

	void ResetMulSelVal();
	bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
	void SetSingleSelect(bool bSingleSel);
	bool GetSingleSelect() const;
	bool UnSelectItem(int iIndex);
	void SelectAllItems();
	void SelectItemsRange(int nRangeBegin, int nRangeEnd);
	bool RemoveSelItemIndex(int iIndex);
	void UnSelectAllItems();
	void UnSelectOtherItems(int iIndex);
	int GetSelectItemCount() const;
	int GetNextSelItem(int nItem) const;
	int GetSelItemByIndex(int nIndex) const;

    CListHeaderUI* GetHeader() const;  
    CContainerUI* GetList() const;
    TListInfoUI* GetListInfo();

    CControlUI* GetItemAt(int iIndex) const;
    int GetItemIndex(CControlUI* pControl) const;
    bool SetItemIndex(CControlUI* pControl, int iIndex);
    int GetCount() const;
    bool Add(CControlUI* pControl);
    bool AddAt(CControlUI* pControl, int iIndex);
    bool Remove(CControlUI* pControl);
    bool RemoveAt(int iIndex);
    void RemoveAll();

    void EnsureVisible(int iIndex);
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
	void SetSelectedItemFont(int nFont);
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
	int GetSelectedItemFont() const;
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
    CListBodyUI* m_pList;
    CListHeaderUI* m_pHeader;
	int nSelectFont;
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

class UILIB_API CListBodyUI : public CVerticalLayoutUI
{
public:
    CListBodyUI(CListUI* pOwner);

    void SetScrollPos(SIZE szPos);
    void SetPos(RECT rc);
    void DoEvent(TEventUI& event);

	BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

protected:
	static int __cdecl ItemComareFunc(void *pvlocale, const void *item1, const void *item2);
	int __cdecl ItemComareFunc(const void *item1, const void *item2);

protected:
    CListUI* m_pOwner;
	PULVCompareFunc m_pCompareFunc;
	UINT_PTR m_compareData;
};

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListHeaderUI : public CHorizontalLayoutUI
{
public:
    CListHeaderUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	SIZE EstimateSize(SIZE szAvailable);
	//开启百分比表头功能 redrain 2014.12.29
	void SetPos(RECT rc);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

	void SetScaleHeader(bool bIsScale);
	bool IsScaleHeader() const;

private:
	bool m_bIsScaleHeader;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListHeaderItemUI : public CControlUI
{
public:
    CListHeaderItemUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

    void SetEnabled(bool bEnable = true);

	bool IsDragable() const;
    void SetDragable(bool bDragable);
	DWORD GetSepWidth() const;
    void SetSepWidth(int iWidth);
	DWORD GetTextStyle() const;
    void SetTextStyle(UINT uStyle);
	DWORD GetTextColor() const;
    void SetTextColor(DWORD dwTextColor);
	void SetTextPadding(RECT rc);
	RECT GetTextPadding() const;
    void SetFont(int index);
    bool IsShowHtml();
    void SetShowHtml(bool bShowHtml = true);
    LPCTSTR GetNormalImage() const;
    void SetNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetHotImage() const;
    void SetHotImage(LPCTSTR pStrImage);
    LPCTSTR GetPushedImage() const;
    void SetPushedImage(LPCTSTR pStrImage);
    LPCTSTR GetFocusedImage() const;
    void SetFocusedImage(LPCTSTR pStrImage);
    LPCTSTR GetSepImage() const;
    void SetSepImage(LPCTSTR pStrImage);
	//开启百分比表头功能,百分比 redrain 2014.12.29
	void SetScale(int nScale);
	int GetScale() const;

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    RECT GetThumbRect() const;

    void PaintText(HDC hDC);
    void PaintStatusImage(HDC hDC);

protected:
    POINT ptLastMouse;
    bool m_bDragable;
    UINT m_uButtonState;
    int m_iSepWidth;
    DWORD m_dwTextColor;
    int m_iFont;
    UINT m_uTextStyle;
    bool m_bShowHtml;
	RECT m_rcTextPadding;
    CDuiString m_sNormalImage;
    CDuiString m_sHotImage;
    CDuiString m_sPushedImage;
    CDuiString m_sFocusedImage;
    CDuiString m_sSepImage;
    CDuiString m_sSepImageModify;
	int m_nScale;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListElementUI : public CLabelUI, public IListItemUI
{
public:
    CListElementUI();

    LPCTSTR GetClass() const;
    UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void SetEnabled(bool bEnable = true);

    int GetIndex() const;
    void SetIndex(int iIndex);

    IListOwnerUI* GetOwner();
    void SetOwner(CControlUI* pOwner);
    void SetVisible(bool bVisible = true);

    bool IsSelected() const;
    bool Select(bool bSelect = true, bool bUnSelectAll = true, bool bSelected = false);
    bool IsExpanded() const;
    bool Expand(bool bExpand = true);

    void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
    bool Activate();

    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

    void DrawItemBk(HDC hDC, const RECT& rcItem);

protected:
    int m_iIndex;
    bool m_bSelected;
    UINT m_uButtonState;
    IListOwnerUI* m_pOwner;
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListLabelElementUI : public CListElementUI
{
public:
    CListLabelElementUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);
    void DoPaint(HDC hDC, const RECT& rcPaint);

    void DrawItemText(HDC hDC, const RECT& rcItem);
};


/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CListTextElementUI : public CListLabelElementUI
{
public:
    CListTextElementUI();
    ~CListTextElementUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

	bool Select(bool bSelect = true);

    LPCTSTR GetText(int iIndex) const;
    void SetText(int iIndex, LPCTSTR pstrText);

    void SetOwner(CControlUI* pOwner);
    CDuiString* GetLinkContent(int iIndex);

    void DoEvent(TEventUI& event);
    SIZE EstimateSize(SIZE szAvailable);

    void DrawItemText(HDC hDC, const RECT& rcItem);

protected:
    enum { MAX_LINK = 8 };
    int m_nLinks;
    RECT m_rcLinks[MAX_LINK];
    CDuiString m_sLinks[MAX_LINK];
    int m_nHoverLink;
    IListUI* m_pOwner;
    CStdPtrArray m_aTexts;
};

/////////////////////////////////////////////////////////////////////////////////////
//


class UILIB_API CListImageTextElementUI : public CListTextElementUI
{
public:
	CListImageTextElementUI();
	~CListImageTextElementUI();

	LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

	LPCTSTR GetNorImg(int iIndex) const;
	LPCTSTR GetSelImg(int iIndex) const;
	void SetNorImg(int iIndex, LPCTSTR pstrText);
	void SetSelImg(int iIndex, LPCTSTR pstrText);

	void SetCheckFlag(int iIndex, bool bStartCheck = TRUE);
	bool GetCheckFlag(int iIndex) const;
	void SetCheck(int iIndex, bool bCheck);
	bool GetCheck(int iIndex) const;	
	LPCTSTR GetNorCheckImg(int iIndex) const;
	LPCTSTR GetSelCheckImg(int iIndex) const;
	void SetNorCheckImg(int iIndex, LPCTSTR pstrText);
	void SetSelCheckImg(int iIndex, LPCTSTR pstrText);

	int HitTest(POINT pt, UINT* pFlags = NULL);

	RECT GetItemRect(int iIndex);
	RECT GetItemRectPadding(int iIndex);
	RECT GetItemCheckRect(int iIndex);
	RECT GetItemImgRect(int iIndex);
	RECT GetItemTextRect(int iIndex);

	void DoEvent(TEventUI& event);
	void DrawItemText(HDC hDC, const RECT& rcItem);	

protected:	
	CStdPtrArray m_aImgNor;
	CStdPtrArray m_aImgSel;
	CStdValArray m_aStartCheck;
	CStdValArray m_aCheck;
	CStdPtrArray m_aImgNorCheck;
	CStdPtrArray m_aImgSelCheck;
};


/////////////////////////////////////////////////////////////////////////////////////
//
class UILIB_API CListContainerElementUI : public CHorizontalLayoutUI, public IListItemUI
{
public:
    CListContainerElementUI();

    LPCTSTR GetClass() const;
    UINT GetControlFlags() const;
    LPVOID GetInterface(LPCTSTR pstrName);

    int GetIndex() const;
    void SetIndex(int iIndex);

    IListOwnerUI* GetOwner();
    void SetOwner(CControlUI* pOwner);
    void SetVisible(bool bVisible = true);
    void SetEnabled(bool bEnable = true);

    bool IsSelected() const;
    bool Select(bool bSelect = true, bool bUnSelectAll = true, bool bSelected = false);
    bool IsExpanded() const;
    bool Expand(bool bExpand = true);

    void Invalidate(); // 直接CControl::Invalidate会导致滚动条刷新，重写减少刷新区域
    bool Activate();

    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    void DoPaint(HDC hDC, const RECT& rcPaint);

    virtual void DrawItemText(HDC hDC, const RECT& rcItem);    
    virtual void DrawItemBk(HDC hDC, const RECT& rcItem);
	void SetPos(RECT rc);

protected:
    int m_iIndex;
    bool m_bSelected;
    UINT m_uButtonState;
    IListOwnerUI* m_pOwner;

};

} // namespace UiLib

#endif // __UILIST_H__
