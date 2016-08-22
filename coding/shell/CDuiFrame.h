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

	void AddTreeNodes(int DbIndex,string DBName, vector<char*> vTableName);
	void ShowListFromTable(string tabName,int DBIndex);
	void ShowList(string sql,int DBIndex, CListUI* pList);
	char* GetPowerWordDBPath();
	void LoadDB(char *PathName, int DBIndex, string DBName);
	void ExecuteSQL(string sql,int DBIndex);
	void showDesignList(string tabName,int DBIndex);
	void UnloadDB();
	void RefreshDB();
	void LoadPowerWordDB();
	void OnListTextElemActive(TNotifyUI& msg);
	void OnClickOpenFileBtn();
	void OnTreeNodeClickOrSelect(TNotifyUI& msg);
	void OnClickTabSwitch(TNotifyUI& msg);
	void UpdatePopWnd(TNotifyUI& msg);
	void AddDesignListHeader();
	void AddLowerLevelTreeNodes(int DBIndex, vector<char*> vTableName);
	void InitDBNode(CTreeNodeUI* pHeadNode, string DBName);
private :
	 int								m_iCurDBIndex;   //当前数据库的索引
	 vector<CppSQLite3DB*>				m_vSqliteDB;     //保存多个数据库
	 CTreeViewUI*						m_pTreeView;    //一个树视图
	 vector<CTreeNodeUI*>				m_vTreeRootNode;//记录代表各个数据库的节点
	 CRichEditUI*						m_pSqlEdit;     //sql语句编辑框
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
	 PopWnd*							m_pPopWnd;       //指向子窗口
	 bool								m_bPopWndIsShowing;
}; 