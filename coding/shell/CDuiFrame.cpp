#include <shlobj.h>
#include <string>
#include "stdafx.h"
#include "CDuiFrame.h"
#include "hdid.h"
#include "PopWnd.h"
//#include "resource.h"

char* TCHAR2char(const TCHAR* tchStr) 
{ 
	int   iLen  = 2*wcslen(tchStr);//CString,TCHAR汉字算一个字符，因此不用普通计算长度 
	char* chRtn = new char[iLen+1] ;
	wcstombs(chRtn,tchStr,iLen+1);//转换成功返回为非负值 
	return chRtn; 
}

TCHAR *char2tchar(char *str)
{
	int   iLen	 = strlen(str);
	TCHAR *chRtn = new TCHAR[iLen+1];
	mbstowcs(chRtn, str, iLen+1);
	return chRtn;
}

char* Utf82Unicode(const char* utf)
{
	if(!utf || !strlen(utf))
	{
		return NULL;
	}
	int			dwUnicodeLen	=	MultiByteToWideChar(CP_UTF8,0,utf,-1,NULL,0);
	size_t		num				=	dwUnicodeLen*sizeof(wchar_t);
	wchar_t*	pwText			=	(wchar_t*)malloc(num);
	memset(pwText,0,num);
	MultiByteToWideChar(CP_UTF8, 0, utf,-1,pwText,dwUnicodeLen);
	return (char*)pwText;
}

char* Unicode2Utf8(const char* unicode)
{
	if(!unicode || !strlen(unicode))
	{
		return NULL;
	}
	
	int len			= WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, NULL, 0, NULL, NULL);
	char* szUtf8	= (char*)malloc(len + 1);
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, (const wchar_t*)unicode, -1, szUtf8, len, NULL,NULL);
	return szUtf8;
}

LPCTSTR CDuiFrameWnd::GetWindowClassName()const
{
	return _T("CDuiFrameWnd");
}

CDuiString CDuiFrameWnd::GetSkinFile()
{
	return _T("resource/duilib.xml");
}

CDuiString CDuiFrameWnd::GetSkinFolder()
{
	return _T("");
}

/*
 LPCTSTR CDuiFrameWnd::GetResourceID() const  
{  
	return MAKEINTRESOURCE(IDR_ZIPRES1);  
}

 UILIB_RESOURCETYPE CDuiFrameWnd::GetResourceType() const  
{  
	return UILIB_ZIPRESOURCE;   
} 
*/

CDuiString getTableNameFromSQL(const char* sql)//select  适用于简单的语句
{
	string tableName;
	string strSQL(sql);
	for ( size_t i = 0; i < strSQL.size(); ++i )
	{
		if (isalpha(strSQL[i]))
		{
			strSQL[i] = tolower(strSQL[i]);
		}
	}
	size_t pos = strSQL.find("from");
	if ( pos != string::npos )
	{
		pos += 4;
		while (pos < strSQL.size() && ' ' == strSQL[pos])
		{
			++pos;
		}
		while (pos < strSQL.size() && strSQL[pos] != '(' && strSQL[pos] != ' ')
		{
			tableName.push_back(strSQL[pos++]);
		}
	}
	return LPCTSTR(Utf82Unicode(tableName.c_str()));
}

void CDuiFrameWnd::initDBNode(CTreeNodeUI* pHeadNode, string DBName)
{
	pHeadNode->SetName(L"DataBaseNode");
	pHeadNode->SetAttribute(L"menu",L"true");
	pHeadNode->SetItemText(LPCTSTR(Utf82Unicode(DBName.c_str())));
	pHeadNode->GetItemButton()->SetFont(4);
	pHeadNode->SetBkColor(0X44444000);
	pHeadNode->GetCheckBox()->SetAttribute(L"width",L"15");
	pHeadNode->GetCheckBox()->SetAttribute(L"height",L"15");
	pHeadNode->GetCheckBox()->SetAttribute(L"normalimage",L"resource/db.png");
	pHeadNode->GetFolderButton()->SetAttribute(L"normalimage",L" file='resource/treeview_b.png' source='0,0,16,16' ");
	pHeadNode->GetFolderButton()->SetAttribute(L"hotimage",L" file='resource/treeview_b.png' source='16,0,32,16' ");
	pHeadNode->GetFolderButton()->SetAttribute(L"selectedimage",L"file='resource/treeview_a.png' source='0,0,16,16'");
	pHeadNode->GetFolderButton()->SetAttribute(L"selectedhotimage",L" file='resource/treeview_a.png' source='16,0,32,16' ");
	m_pTreeView->Add(pHeadNode);
	pHeadNode->SetTreeView(m_pTreeView);
}

char* CDuiFrameWnd::GetDbPath()
{
	int csidl[3] = {CSIDL_APPDATA,CSIDL_COMMON_APPDATA,CSIDL_LOCAL_APPDATA};
	for (int i = 0; i < 3 ; i++)
	{
		TCHAR szPath[MAX_PATH] = {0};
		SHGetSpecialFolderPath(NULL,szPath,csidl[i],TRUE);
		::PathAddBackslashW(szPath);
		_tcscat_s(szPath,MAX_PATH,L"Kingsoft\\PowerWord\\Powerword.db");
		if(PathFileExists(szPath))
			return TCHAR2char(szPath);
	}
	return NULL;
}

void CDuiFrameWnd::addDesignListHeader()
{
	for (int i = 0; i< 2; i++)
	{
		CListHeaderItemUI*	pListHeader = new CListHeaderItemUI;
		if (0 == i)
		{
			pListHeader->SetText(L"name");
		}
		else
		{
			pListHeader->SetText(L"declType");
		}
		pListHeader->SetFont(4);
		pListHeader->SetHotImage(L"resource/list_header_hot.png");
		pListHeader->SetPushedImage(L"resource/list_header_pushed.png");
		pListHeader->SetSepImage(L"resource/list_header_sep.png");
		pListHeader->SetSepWidth(1);

		m_pDesignList->Add(pListHeader);

		
	}
}

void CDuiFrameWnd::loadPowerWordDB()
{
	m_vSqliteDB.push_back(new CppSQLite3DB());
	m_iCurDBIndex	=	m_iCurDBIndex + 1;
	loadDB(GetDbPath(), 0, "PowerWord.db");
	m_vDBPath.push_back(LPCTSTR(Utf82Unicode(GetDbPath())));
	m_vTreeRootNode[m_iCurDBIndex]->Select(true);  
	m_pDBPathLabel->SetText(m_vDBPath[m_iCurDBIndex]);
}

void CDuiFrameWnd::InitWindow()
{
	m_pTreeView			=	static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("TreeViewDemo")));
	m_pList				=	static_cast<CListUI*>(m_PaintManager.FindControl(_T("ListDemo1")));
	m_btnOpenFile		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("OpenFile")));
	m_btnCloseFile		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("CloseFile")));
	m_pSqlEdit			=	static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("SQLedit")));
	m_btnSql			=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("SQLBtn")));
	m_pSqlList			=	static_cast<CListUI*>(m_PaintManager.FindControl(_T("SQLList")));
	m_btnRefreshDB		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("Refresh")));
	m_btnRefreshTable	=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("RefreshTable")));
	m_pDesignList		=	static_cast<CListUI*>(m_PaintManager.FindControl(_T("designList")));
	m_pDBPathLabel		=	static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("DBPath")));
	m_pCurTreeNode		=	NULL;
	m_pCurListTextElem	=	NULL;
	m_pPopWnd			=	NULL;

	m_pPopWnd = new PopWnd();
	m_pPopWnd->setParentWnd(this);
	m_pPopWnd->Create(this->m_hWnd,_T("Record Editor"), WS_POPUP | WS_VISIBLE, 0L, 0, 0, 800, 572);
	m_pPopWnd->ShowWindow(false);
	m_iCurDBIndex = -1;

	loadPowerWordDB();
	addDesignListHeader();
}

void CDuiFrameWnd::loadDB(char* PathName, int DBIndex, string DBName)
{
	//获取密码
	std::wstring strhdid;
	GenerateHDID(strhdid);
	char* ch = TCHAR2char(strhdid.c_str());
	string strh(ch);
	
	vector<char*> vTableName;

	try
	{
		m_vSqliteDB[DBIndex]->open(PathName);
		if (m_vSqliteDB[DBIndex]->isOpen())
		{
			CppSQLite3Query query = m_vSqliteDB[DBIndex]->execQuery("SELECT name FROM sqlite_master WHERE type='table' order by name");
			while (!query.eof())
			{
				const char* res = query.fieldValue(0);
				char* ch = Utf82Unicode(res);
				vTableName.push_back(ch);
				query.nextRow();
			}
			query.finalize();
		}
	}
	catch (CppSQLite3Exception e)
	{
		if (e.errorCode() == SQLITE_NOTADB)
		{
			m_vSqliteDB[DBIndex]->open(PathName,strh.c_str(),strh.length());
			CppSQLite3Query query = m_vSqliteDB[DBIndex]->execQuery("SELECT name FROM sqlite_master WHERE type='table' order by name");
			while(!query.eof())
			{
				const char* res = query.fieldValue(0);
				char*		ch  = Utf82Unicode(res);
				vTableName.push_back(ch);
				query.nextRow();
			}
			query.finalize();
		}
	}
	AddTable(DBIndex,DBName,vTableName);
}

void CDuiFrameWnd::AddTable(int DBIndex, string DBName, vector<char*> vTableName)
{
	CTreeNodeUI* pHeadNode = new CTreeNodeUI();
	m_vTreeRootNode.push_back(pHeadNode);
	initDBNode(pHeadNode, DBName);
	addTableNode(DBIndex,vTableName);
}

void CDuiFrameWnd::executeSQL(string sql, int DBIndex)
{
	if (sql.empty())
	{
		MessageBoxA(NULL,"Please select the SQL statement.","faild",MB_OK);
		return ;
	}
	if (DBIndex < 0)
	{
		MessageBoxA(NULL,"请选择一个数据库","faild",MB_OK);
		return ;
	}

	m_pSqlList->RemoveAll();
	m_pSqlList->GetHeader()->RemoveAll();
	
	m_pSqlList->SetAttribute(L"itemalign",L"center");

	try
	{
		string mod;
		for (size_t i = 0;i < sql.size() && sql[i]!=' '; i++)
		{
			if (sql[i]>'Z')
			{
				mod.push_back(sql[i]-('a'-'A'));
			}
			else
			{
				mod.push_back(sql[i]);
			}
		}
		if (mod != string("SELECT"))
		{
			CppSQLite3Query query = m_vSqliteDB[DBIndex]->execQuery(sql.c_str());
			return ;
		}
		m_CDuiStrActiveListItemTableName = getTableNameFromSQL(sql.c_str());
	}
	catch(CppSQLite3Exception e)
	{
		MessageBoxA(NULL, e.errorMessage(), "Error", MB_OK);
		return ;
	}
	ShowList(sql,DBIndex, m_pSqlList);
}

void CDuiFrameWnd::addTableNode(int DBIndex, vector<char*> vTableName)
{
	for (size_t i = 0; i < vTableName.size(); i++)
	{
		CTreeNodeUI* pTreeNodeElement = new CTreeNodeUI(m_vTreeRootNode[DBIndex]);
		pTreeNodeElement->SetName(LPCTSTR(vTableName[i]));
		pTreeNodeElement->SetItemText(LPCTSTR(vTableName[i]));
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"width",L"15");
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"height",L"15");
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"normalimage",L"resource/tables.png");
		pTreeNodeElement->GetItemButton()->SetFont(4);
		pTreeNodeElement->GetTreeNodeHoriznotal()->SetToolTip(LPCTSTR(vTableName[i]));
		pTreeNodeElement->SetParentNode(m_vTreeRootNode[DBIndex]);
		if(i & 1)
		{
			pTreeNodeElement->SetBkColor(0X11001100);
		}
		else
		{
			pTreeNodeElement->SetBkColor(0X22110022);
		}
		m_vTreeRootNode[DBIndex]->Add(pTreeNodeElement);
	}
}

void CDuiFrameWnd::refreshDB()
{
	if (m_iCurDBIndex < 0)
	{
		MessageBoxA(NULL,"请选择一个数据库","faild",MB_OK);
		return ;
	}

	while (m_vTreeRootNode[m_iCurDBIndex]->GetCountChild())  
	{
		m_vTreeRootNode[m_iCurDBIndex]->Remove(m_vTreeRootNode[m_iCurDBIndex]->GetChildNode(0));
	}
	
	vector<char*>	vTableName;
	try
	{
		if (m_vSqliteDB[m_iCurDBIndex]->isOpen())
		{
			CppSQLite3Query query = m_vSqliteDB[m_iCurDBIndex]->execQuery("SELECT name FROM sqlite_master WHERE type='table' order by name");
			
			while (!query.eof())
			{
				const char* res = query.fieldValue(0);
				char*		ch  = Utf82Unicode(res);
				vTableName.push_back(ch);
				query.nextRow();
			}
			query.finalize();
		}
	}
	catch (CppSQLite3Exception e)
	{
		if (e.errorCode() == SQLITE_NOTADB)
		{
			CppSQLite3Query query = m_vSqliteDB[m_iCurDBIndex]->execQuery("SELECT name FROM sqlite_master WHERE type='table' order by name");
			while(!query.eof())
			{
				const char* res = query.fieldValue(0);
				char*		ch  = Utf82Unicode(res);
				vTableName.push_back(ch);
				query.nextRow();
			}
			query.finalize();
		}
	}

	addTableNode(m_iCurDBIndex,vTableName);
	m_vTreeRootNode[m_iCurDBIndex]->Select(true);
}

void CDuiFrameWnd::unloadDB()
{
	if (m_iCurDBIndex < 0)
	{
		MessageBoxA(NULL,"请选择一个数据库","faild",MB_OK);
		return ;
	}
	if (m_vSqliteDB[m_iCurDBIndex]->isOpen())
	{
		m_vSqliteDB[m_iCurDBIndex]->close();
	}

	m_pList->RemoveAll();
	m_pList->GetHeader()->RemoveAll();

	while (m_vTreeRootNode[m_iCurDBIndex]->GetCountChild())  
	{
		m_vTreeRootNode[m_iCurDBIndex]->Remove(m_vTreeRootNode[m_iCurDBIndex]->GetChildNode(0));
	}
	m_pTreeView->Remove(m_vTreeRootNode[m_iCurDBIndex]);

	m_vSqliteDB.erase(std::find(m_vSqliteDB.begin(),m_vSqliteDB.end(),m_vSqliteDB[m_iCurDBIndex]));
	m_vTreeRootNode.erase(std::find(m_vTreeRootNode.begin(),m_vTreeRootNode.end(),m_vTreeRootNode[m_iCurDBIndex])); 
	m_vDBPath.erase(std::find(m_vDBPath.begin(),m_vDBPath.end(),m_pDBPathLabel->GetText()));

	m_pDesignList->RemoveAll();

	if (0 == m_vSqliteDB.size())
	{
		m_pDesignList->GetHeader()->RemoveAll();
		
	}

	if (0 == m_iCurDBIndex)
	{
		if (m_vSqliteDB.empty())
		{
			m_iCurDBIndex	=  -1 ;
			m_pCurTreeNode	=  NULL;

			m_pDBPathLabel->SetText(L"");
		}
		else
		{
			m_vTreeRootNode[m_iCurDBIndex]->Select(true); 
		}
	}
	else
	{
		m_iCurDBIndex = -- m_iCurDBIndex;
		int sz = m_vTreeRootNode.size();
		m_vTreeRootNode[m_iCurDBIndex]->Select(true);
	}
	m_pPopWnd->ShowWindow(false);
	m_bPopWndIsShowing =  false;
}

void CDuiFrameWnd::showDesign(string tabName, int DBIndex)
{
	CListUI* pDesignList = static_cast<CListUI*>(m_PaintManager.FindControl(_T("designList")));
	pDesignList->RemoveAll();

	string					sql				= string("select * from ") + tabName;
	CppSQLite3Query			query			= m_vSqliteDB[m_iCurDBIndex]->execQuery(sql.c_str());
	int						col				= query.numFields();
	for (int i = 0; i < col; ++i)
	{
		const char*			strName			=	query.fieldName(i);
		const char*			strDeclType		=	query.fieldDeclType(i);
		CListTextElementUI* pListTextElem	= new CListTextElementUI();
		
		pDesignList->Add(pListTextElem);
		pListTextElem->SetText(0,LPCTSTR(Utf82Unicode(strName)));
		pListTextElem->SetText(1,LPCTSTR(Utf82Unicode(strDeclType)));
	}
	pDesignList->NeedParentUpdate();
}

void CDuiFrameWnd::OnListTextElemActive(TNotifyUI& msg)
{
	
	for (int i = 0; i < m_pList->GetList()->GetCount(); i++)
	{
		if (msg.pSender == m_pList->GetList()->GetItemAt(i))
		{
			m_pCurListTextElem = static_cast<CListTextElementUI*>(m_pList->GetList()->GetItemAt(i));
			m_CDuiStrActiveListItemTableName = m_pCurTreeNode->GetItemText();
			m_pPopWnd->InitWindow();
			m_pPopWnd->CenterWindow();
			m_pPopWnd->ShowWindow(true);
			m_bPopWndIsShowing = true;
			return ;
		}
	}
	for (int i = 0; i < m_pSqlList->GetList()->GetCount(); ++i)
	{
		CListTextElementUI*	pListTextElem = static_cast<CListTextElementUI*>(m_pSqlList->GetList()->GetItemAt(i));
		if (msg.pSender == pListTextElem)
		{
			m_pCurListTextElem = pListTextElem;
			m_pPopWnd->InitWindow();
			m_pPopWnd->CenterWindow();
			m_pPopWnd->ShowWindow(true);
			m_bPopWndIsShowing = true;
			return ;
		}
	}
}

void CDuiFrameWnd::OnClickOpenFileBtn()
{
	OPENFILENAME ofn;
	char szFile[MAX_PATH];
	ZeroMemory(&ofn,sizeof(ofn));
	ofn.lStructSize		=	 sizeof(ofn);
	ofn.lpstrFile		=	 LPWSTR(szFile);
	ofn.lpstrFile[0]	=	 TEXT('\0');
	ofn.nMaxFile		=	 sizeof(szFile);
	ofn.lpstrFilter		=	 TEXT("ALL\0*.*\0Text\0*.TXT\0");
	ofn.nFilterIndex	=	 1;
	ofn.lpstrFileTitle	=	 NULL;
	ofn.nMaxFileTitle	=	 0;
	ofn.lpstrInitialDir =	 NULL;
	ofn.Flags			=	 OFN_EXPLORER |OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
	{
		ofn.lpstrFileTitle;
		char * path = Unicode2Utf8(szFile);
		string s(path);

		string addName;
		for (int i = s.size()-1; i >= 0 && s[i] != '\\'; --i)
		{
			addName.push_back(s[i]);
		}
		addName = string(addName.rbegin(),addName.rend());

		m_vSqliteDB.push_back(new CppSQLite3DB());
		m_iCurDBIndex = m_vSqliteDB.size() - 1;
		loadDB(path, m_iCurDBIndex, addName);
		m_vDBPath.push_back( LPCTSTR(Utf82Unicode(path)));

		m_vTreeRootNode[m_iCurDBIndex]->Select(true);

		m_pList->RemoveAll();
		m_pList->GetHeader()->RemoveAll();
	
		m_pDesignList->RemoveAll();

		if (1 == m_vSqliteDB.size()) 
		{
			addDesignListHeader();
		}
	}
}

void CDuiFrameWnd::OnTreeNodeClickOrSelect(TNotifyUI& msg)
{
	m_pPopWnd->ClearFrame();
	int count = msg.wParam + 1;//触发事件的节点在treeView中的编号
	for (size_t i = 0; i < m_vTreeRootNode.size(); i++)
	{
		if (count > m_vTreeRootNode[i]->GetCountChild() + 1)
		{
			count -= (m_vTreeRootNode[i]->GetCountChild() + 1);
		}
		else
		{
			m_iCurDBIndex = i;
			m_pDBPathLabel->SetText(m_vDBPath[i]);

			if (count == 1)
			{
				m_pCurTreeNode = m_vTreeRootNode[i];
				m_pDesignList->RemoveAll();
				m_pDesignList->GetHeader()->RemoveAll();
			

				m_pList->RemoveAll();
				m_pList->GetHeader()->RemoveAll();
			}
			else
			{
				m_pCurTreeNode = m_vTreeRootNode[i]->GetChildNode(count - 2);
				string name = m_pCurTreeNode->GetName().GetStringUtf8();
				
				ShowTable(name, m_iCurDBIndex);
				showDesign(name, m_iCurDBIndex);

				if (!m_pDesignList->GetHeader()->GetCount())
				{
					addDesignListHeader();
				}
			}
			break;
		}
	}
}

void CDuiFrameWnd::updatePopWnd(TNotifyUI& msg)
{
	CDuiString name = msg.pSender->GetName();

	if (msg.pSender == m_pList || msg.pSender == m_pSqlList)  
	{
		for (int i = 0; i < m_pList->GetList()->GetCount(); i++)
		{
			CListTextElementUI*	pListTextElem = static_cast<CListTextElementUI*>(m_pList->GetList()->GetItemAt(i));
			if (pListTextElem->IsSelected())  //psender 并不是 pListTextElem
			{
				pListTextElem->Select(true);    
				if (true == m_bPopWndIsShowing)
				{
					m_pCurListTextElem = pListTextElem;
					m_CDuiStrActiveListItemTableName = m_pCurTreeNode->GetItemText();
					m_pPopWnd->InitWindow(); 
					m_pPopWnd->ShowWindow(true);
					m_bPopWndIsShowing = true;
					return ;
				}
			}
		}
		for (int i = 0; i < m_pSqlList->GetList()->GetCount(); ++i)
		{
			CListTextElementUI* pListTextElem = static_cast<CListTextElementUI*>(m_pSqlList->GetList()->GetItemAt(i));
			if (pListTextElem->IsSelected())
			{
				if (true == m_bPopWndIsShowing)
				{
					m_pCurListTextElem = pListTextElem;
					m_pPopWnd->InitWindow();
					m_pPopWnd->ShowWindow(true);
					m_bPopWndIsShowing = true;

					return ;
				}
			}
		}
		return ;
	}

	for (int i = 0; i < m_pList->GetList()->GetCount(); i++)
	{
		CListTextElementUI*	pListTextElem = static_cast<CListTextElementUI*>(m_pList->GetList()->GetItemAt(i));
		if (msg.pSender != pListTextElem && pListTextElem->IsSelected())
		{
			pListTextElem->Select(false);
		}
	}
	for (int i =0; i < m_pSqlList->GetList()->GetCount(); ++i)
	{
		CListTextElementUI* pListTextElem = static_cast<CListTextElementUI*>(m_pSqlList->GetList()->GetItemAt(i));
		if (msg.pSender != pListTextElem && pListTextElem->IsSelected())
		{
			pListTextElem->Select(false);
		}
	}

	for (int i = 0; i < m_pList->GetList()->GetCount(); i++)
	{
		CListTextElementUI*	pListTextElem = static_cast<CListTextElementUI*>(m_pList->GetList()->GetItemAt(i));
		if (msg.pSender == pListTextElem )
		{
			pListTextElem->Select(true);
			if (true == m_bPopWndIsShowing)
			{
				m_pCurListTextElem = pListTextElem;
				m_CDuiStrActiveListItemTableName = m_pCurTreeNode->GetItemText();

				m_pPopWnd->InitWindow();
				m_pPopWnd->ShowWindow(true);
				m_bPopWndIsShowing = true;
				return ;
			}
		}
	}
	for (int i = 0; i < m_pSqlList->GetList()->GetCount(); ++i)
	{
		CListTextElementUI*	pListTextElem = static_cast<CListTextElementUI*>(m_pSqlList->GetList()->GetItemAt(i));
		if (msg.pSender == pListTextElem )
		{
			pListTextElem->Select(true);
			if (true == m_bPopWndIsShowing)
			{
				m_pCurListTextElem = pListTextElem;

				m_pPopWnd->InitWindow();
				m_pPopWnd->ShowWindow(true);
				m_bPopWndIsShowing = true;

				return ;
			}
		}
	}
}

void CDuiFrameWnd::OnClickTabSwitch(TNotifyUI& msg)
{
	CDuiString		strName	 = msg.pSender->GetName();
	CTabLayoutUI*	pControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_switch")));
	if (strName == _T("data_tab"))
	{
		pControl->SelectItem(0);
	} 
	else if (strName == _T("sql_tab"))
	{
		pControl->SelectItem(1);
	}
	else if (strName == _T("design_tab"))
	{
		pControl->SelectItem(2);
	}
}

void CDuiFrameWnd::ShowList(string sql,int DBIndex,CListUI* pList)
{
	if (DBIndex < 0 )
	{
		return ;
	}

	pList->RemoveAll();
	pList->GetHeader()->RemoveAll();

	try
	{
		CppSQLite3Query query	=	m_vSqliteDB[DBIndex]->execQuery(sql.c_str());
		int				col		=	query.numFields();
		vector<int >				vTextLen(col,0);
		vector<CListHeaderItemUI*>	vHeadItem;
		
		CString str(" ");
		HDC hDC = ::GetDC(this->m_hWnd);
		SIZE sz;
		::GetTextExtentPoint32(hDC, str, str.GetLength(),&sz);
		::ReleaseDC(this->m_hWnd, hDC);
		int per = sz.cx+4;

		for (int i = 0; i < col; i++)
		{
			const char* res = query.fieldName(i);

			CString Cstr(res);
			CListHeaderItemUI*  pListHItem = new CListHeaderItemUI;
			vHeadItem.push_back(pListHItem);

			//获取字体的宽度和高度
			HDC hDC = ::GetDC(this->m_hWnd);
			SIZE sz;
			::GetTextExtentPoint32(hDC, Cstr, Cstr.GetLength(), &sz);
			::ReleaseDC(this->m_hWnd, hDC);
			int x = sz.cx;

			vTextLen[i] =  vTextLen[i]>x?vTextLen[i]:x;
			vTextLen[i] += 60+col+1;

			pListHItem->SetName(Cstr);
			pListHItem->SetText(Cstr);
			pListHItem->SetFont(4);
			pListHItem->SetAttribute(L"hotimage",L"resource/list_header_hot.png");
			pListHItem->SetAttribute(L"pushedimage",L"resource/list_header_pushed.png");
			pListHItem->SetAttribute(L"sepimage",L"resource/list_header_sep.png");
			pListHItem->SetAttribute(L"sepwidth",L"1");

			pList->Add(pListHItem);
		}

		pList->NeedParentUpdate();
		query = m_vSqliteDB[DBIndex]->execQuery(sql.c_str());

		while(!query.eof())
		{
			CListTextElementUI* pListElement = new CListTextElementUI;
			pList->Add(pListElement);

			int col = query.numFields();
			for(int i=0;i<col;i++)
			{
				const char* str =  query.getStringField(i);
				int			w	=  strlen(str)*per;
				vTextLen[i]		=  vTextLen[i]>w?vTextLen[i]:w;

				pListElement->SetText(i,LPCTSTR(Utf82Unicode(str)));
				pListElement->SetAttribute(L"wordbreak",L"true");
			}
			query.nextRow();
		}
		query.finalize();
		for(int i=0;i<col;i++)
		{
			vHeadItem[i]->SetFixedWidth(vTextLen[i] + 10);
		}
	}
	catch(CppSQLite3Exception e)
	{
		MessageBoxA(NULL, e.errorMessage(), "Error", MB_OK);
	}
}

void CDuiFrameWnd::ShowTable(string TabName,int DBIndex)
{
	string sql = string("select * from ") + TabName;
	ShowList(sql,DBIndex,m_pList);
}

void CDuiFrameWnd::Notify(TNotifyUI& msg)
{
	if ( msg.sType == _T("itemactivate") )
	{
		OnListTextElemActive(msg);
	}
	else if (msg.sType == _T("itemclick") || msg.sType == _T("itemselect"))
	{
		if (msg.pSender == m_pTreeView)
		{
			OnTreeNodeClickOrSelect(msg);
		}
		updatePopWnd(msg);
	}
	else if (msg.sType == _T("click"))  //响应按钮的点击
	{
		if (msg.pSender == m_btnOpenFile)
		{
			OnClickOpenFileBtn();
		}
		else if (msg.pSender == m_btnSql)
		{
			executeSQL(m_pSqlEdit->GetSelText().GetStringUtf8(), m_iCurDBIndex);
		}
		else if (msg.pSender == m_btnCloseFile)
		{
			unloadDB();
		}
		else if (msg.pSender == m_btnRefreshDB)
		{
			refreshDB();
		}
		else if (msg.pSender == m_btnRefreshTable)
		{
			if (m_pCurTreeNode)
			{
				ShowTable(m_pCurTreeNode->GetItemButton()->GetText().GetStringA(), m_iCurDBIndex);
			}
		}
	}
	else if (msg.sType == _T("selectchanged"))
	{
		OnClickTabSwitch(msg);
	}
	__super::Notify(msg);
}

CDuiFrameWnd::~CDuiFrameWnd()
{
	for (size_t i = 0; i < m_vSqliteDB.size(); i++)
	{
		if (m_vSqliteDB[i]->isOpen())
		{
			m_vSqliteDB[i]->close();
		}
	}
	m_vSqliteDB.clear();
}
