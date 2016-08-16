#include "CDuiFrame.h"

class PopWnd : public WindowImplBase
{
	//friend class CDuiFrameWnd;
public:
	virtual LPCTSTR GetWindowClassName()const ;
	virtual CDuiString GetSkinFile() ;
	virtual CDuiString GetSkinFolder();
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);

	void clearFrame();
	void OnPreviousBtn();
	void OnNextBtn();
	void OnFirstBtn();
	void OnLastBtn();
	void OnDeleteBtn();
	void OnAddBtn();
	void OnOkBtn();
	void loadFrame();
	void DeletAndSelectOtherTextItem(CListUI* pFrameList,vector<CListTextElementUI*>& vFrameTextElem,CListTextElementUI* &pDeleteTextElem);

	void setParentWnd(CDuiFrameWnd* parentWnd);

private:
	vector<CLabelUI* >			m_vPropertyName; //表的所有属性的名字
	vector<CRichEditUI*>		m_vData;		 //数据编辑框
	vector<CDuiString>			m_vOriginalData; //编辑框未修改之前的内容
	CVerticalLayoutUI*			m_pDomain;
	CButtonUI*					m_pDeleteBtn;
	CButtonUI*					m_pAddBtn;
	CButtonUI*					m_pOkBtn;
	CButtonUI*					m_pFirstBtn;
	CButtonUI*					m_pPreviousBtn;
	CButtonUI*					m_pNextBtn;
	CButtonUI*					m_pLastBtn;

private:
	enum
	{
		YX__SQL__DELETE , YX__SQL__ADD , YX__SQL__UPDATE
	}m_enumSQL_STATU;

private:
	CDuiFrameWnd*				m_pParentWnd;	 //指向父类窗口
};