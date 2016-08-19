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
	virtual void Notify(TNotifyUI& msg);
	~CDuiFrameWnd();
	
	//virtual LPCTSTR GetResourceID() const  ; 
	//virtual UILIB_RESOURCETYPE GetResourceType() const ;
	
	void AddTable(int DbIndex,string DBName, vector<char*> vTableName);
	void ShowTable(string tabName,int DBIndex);
	void ShowList(string sql,int DBIndex, CListUI* pList);
	char* GetDbPath();
	void loadDB(char *PathName, int DBIndex, string DBName);
	void executeSQL(string sql,int DBIndex);
	void showDesign(string tabName,int DBIndex);
	void unloadDB();
	void refreshDB();
	void loadPowerWordDB();

	void OnListTextElemActive(TNotifyUI& msg);
	void OnClickOpenFileBtn();
	void OnTreeNodeClickOrSelect(TNotifyUI& msg);
	void OnClickTabSwitch(TNotifyUI& msg);
	void updatePopWnd(TNotifyUI& msg);

	void addDesignListHeader();
	void addTableNode(int DBIndex, vector<char*> vTableName);
	void initDBNode(CTreeNodeUI* pHeadNode, string DBName);
private :

	 CButtonUI*							m_btnOpenFile;
	 CButtonUI*							m_btnCloseFile;
	 CButtonUI*							m_btnRefreshDB;
	 CButtonUI*							m_btnRefreshTable;
	 CButtonUI*							m_btnSql;

	 int								m_iCurDBIndex;   //当前数据库的索引
	 vector<CppSQLite3DB*>				m_vSqliteDB;     //保存多个数据库

	 CTreeViewUI*						m_pTreeView;    //一个树视图
	 vector<CTreeNodeUI*>				m_vTreeRootNode;//记录代表各个数据库的节点

	 CRichEditUI*						m_pSqlEdit;
	 CTreeNodeUI*						m_pCurTreeNode;
	 CListTextElementUI*				m_pCurListTextElem;

	 CListUI*							m_pList;
	 CListUI*							m_pSqlList;
	 CListUI*							m_pDesignList;

	 CDuiString							m_CDuiStrActiveListItemTableName;
	 vector<CDuiString>					m_vDBPath;
	 CLabelUI*							m_pDBPathLabel;
private:
	 PopWnd*							m_pPopWnd;       //指向子窗口
	 bool								m_bPopWndIsShowing;
}; 