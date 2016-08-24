#pragma once
#include "cppsqlite3.h"
#include "sqlite3.h"
#include "stdafx.h"

class CDuiFrameWnd : public WindowImplBase
{
	friend class PopWnd;
public :
	virtual LPCTSTR GetWindowClassName()const ;
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual void InitWindow();

	//virtual LPCTSTR GetResourceID() const  ; 
	//virtual UILIB_RESOURCETYPE GetResourceType() const ;


	void InitDBNode(CTreeNodeUI* pHeadNode, string DBName);
	void AddLowerLevelTreeNodes(int DBIndex, vector<char*> vTableName);
	void AddTreeNodes(int DbIndex, string DBName, vector<char*> vTableName);

	char* GetPowerWordDBPath();
	void LoadPowerWordDB();
	void LoadDB(char *PathName, int DBIndex, string DBName);

	void ShowListFromTable(string tabName,int DBIndex);
	void ShowList(string sql,int DBIndex, CListUI* pList);

	void ExecuteSQL(string sql,int DBIndex);
	void AddDesignListHeader();
	void ShowDesignList(string tabName,int DBIndex);
	void RefreshDB();
	void UnloadDB();
	
	void OnListTextElemActive(TNotifyUI& msg);
	void OnClickOpenFileBtn();
	void OnTreeNodeClickOrSelect(TNotifyUI& msg);
	void OnClickTabSwitch(TNotifyUI& msg);
	void UpdatePopWnd(TNotifyUI& msg);
	
	virtual void Notify(TNotifyUI& msg);
	~CDuiFrameWnd();
private :
	 int								m_iCurDBIndex;   //��ǰ���ݿ������
	 vector<CppSQLite3DB*>				m_vSqliteDB;     //���������ݿ�
	 CTreeViewUI*						m_pTreeView;    //һ������ͼ
	 vector<CTreeNodeUI*>				m_vTreeRootNode;//��¼����������ݿ�Ľڵ�
	 CRichEditUI*						m_pSqlEdit;     //sql���༭��
	 CTreeNodeUI*						m_pCurTreeNode;
	 CListTextElementUI*				m_pCurListTextElem;
	 CListUI*							m_pList;
	 CListUI*							m_pSqlList;
	 CListUI*							m_pDesignList;
	 CButtonUI*							m_btnOpenFile;
	 CButtonUI*							m_btnCloseFile;
	 CButtonUI*							m_btnRefreshDB;
	 CButtonUI*							m_btnRefreshTable;
	 CButtonUI*							m_btnSql;
	 CDuiString							m_CDuiStrActiveListItemTableName;
	 vector<CDuiString>					m_vDBPath;
	 CLabelUI*							m_pDBPathLabel;
private:
	 PopWnd*							m_pPopWnd;       //ָ���Ӵ���
	 bool								m_bPopWndIsShowing;
}; 