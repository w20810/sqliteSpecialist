#include "CDuiFrame.h"

class PopWnd : public WindowImplBase
{

public:
	virtual LPCTSTR GetWindowClassName()const ;
	virtual CDuiString GetSkinFile() ;
	virtual CDuiString GetSkinFolder();
	virtual void InitWindow();
	virtual void Notify(TNotifyUI& msg);
	
	void OkAdd();
	void OkUpdate();
	void ClearFrame();
	void OnClickPreviousBtn();
	void OnClickNextBtn();
	void OnClickFirstBtn();
	void OnClickLastBtn();
	void OnClickDeleteBtn();
	void OnClickAddBtn();
	void OnClickOkBtn();
	void LoadFrame();
	void SetParentWnd(CDuiFrameWnd* parentWnd);
	void AddListTextELem();
	void DeletAndSelectOtherTextItem(CListUI* pFrameList, CListTextElementUI* &pDeleteTextElem);

private:
	vector<CLabelUI* >			m_vPropertyName; //����������Ե�����
	vector<CRichEditUI*>		m_vData;		 //���ݱ༭��
	vector<CDuiString>			m_vOriginalData; //�༭��δ�޸�֮ǰ������
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
	CDuiFrameWnd*				m_pParentWnd;	 //ָ���ര��
};