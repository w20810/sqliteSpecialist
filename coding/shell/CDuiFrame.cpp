#include <shlobj.h>
#include <string>
#include "stdafx.h"
#include "CDuiFrame.h"
#include "hdid.h"
#include "PopWnd.h"
//#include "resource.h"

char* TCHAR2char(const TCHAR* tchStr) 
{ 
	int   iLen  = 2*wcslen(tchStr);//CString,TCHAR������һ���ַ�����˲�����ͨ���㳤�� 
	char* chRtn = new char[iLen+1] ;
	wcstombs(chRtn,tchStr,iLen+1);//ת���ɹ�����Ϊ�Ǹ�ֵ 
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

CDuiString getTableNameFromSQL(const char* sql)//select  �����ڼ򵥵����
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
		
		m_vDesignListHeader.push_back(pListHeader);
	}
}

void CDuiFrameWnd::loadPowerWordDB()
{
	m_vSqliteDB.push_back(new CppSQLite3DB());
	m_vNameOfDB.push_back(string("Powerword.db"));
	m_iCurDBIndex	= 0 ;
	loadDB(GetDbPath(),0);
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
void CDuiFrameWnd::loadDB(char* PathName, int DBIndex)
{
	//��ȡ����
	std::wstring strhdid;
	GenerateHDID(strhdid);
	char* ch = TCHAR2char(strhdid.c_str());
	string strh(ch);
	
	try
	{
		m_vSqliteDB[DBIndex]->open(PathName);
		if (m_vSqliteDB[DBIndex]->isOpen())
		{
			CppSQLite3Query query = m_vSqliteDB[DBIndex]->execQuery("SELECT name FROM sqlite_master WHERE type='table' order by name");
			m_vvTableName.push_back(new vector<char*>);
			while (!query.eof())
			{
				const char* res = query.fieldValue(0);
				char* ch = Utf82Unicode(res);
				m_vvTableName[DBIndex]->push_back(ch);
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
				m_vvTableName[DBIndex]->push_back(ch);
				query.nextRow();
			}
			query.finalize();
		}
	}
	AddTable(DBIndex);
}

void CDuiFrameWnd::AddTable(int DBIndex)
{
	
	CTreeNodeUI* pHeadNode = new CTreeNodeUI();
	m_vTreeRootNode.push_back(pHeadNode);

	pHeadNode->SetName(L"DataBaseNode");
	pHeadNode->SetAttribute(L"menu",L"true");
	pHeadNode->SetItemText(LPCTSTR(Utf82Unicode(m_vNameOfDB[DBIndex].c_str())));
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
	for (size_t i = 0; i < m_vvTableName[DBIndex]->size(); i++)
	{
		CTreeNodeUI* pTreeNodeElement = new CTreeNodeUI(pHeadNode);
		pTreeNodeElement->SetName(LPCTSTR((*m_vvTableName[DBIndex])[i]));
		pTreeNodeElement->SetItemText(LPCTSTR((*m_vvTableName[DBIndex])[i]));
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"width",L"15");
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"height",L"15");
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"normalimage",L"resource/tables.png");
		pTreeNodeElement->GetItemButton()->SetFont(4);
		pTreeNodeElement->GetTreeNodeHoriznotal()->SetToolTip(LPCTSTR((*m_vvTableName[DBIndex])[i]));
		pTreeNodeElement->SetParentNode(pHeadNode);
		if(i&1)
			pTreeNodeElement->SetBkColor(0X11001100);
		else
			pTreeNodeElement->SetBkColor(0X22110022);

		pHeadNode->Add(pTreeNodeElement);
	}
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
		MessageBoxA(NULL,"��ѡ��һ�����ݿ�","faild",MB_OK);
		return ;
	}

	m_pSqlList->RemoveAll();
	for (size_t i = 0; i < m_vSqlListHeader.size(); i++)
	{
		m_pSqlList->Remove(m_vSqlListHeader[i]);
	}
	m_vSqlListTextElem.clear();
	m_vSqlListHeader.clear();

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

	ShowList(sql,DBIndex, m_pSqlList,m_vSqlListHeader, m_vSqlListTextElem);
}
void CDuiFrameWnd::refreshDB()
{
	if (m_iCurDBIndex < 0)
	{
		MessageBoxA(NULL,"��ѡ��һ�����ݿ�","faild",MB_OK);
		return ;
	}

	while (m_vTreeRootNode[m_iCurDBIndex]->GetCountChild())  
	{
		m_vTreeRootNode[m_iCurDBIndex]->Remove(m_vTreeRootNode[m_iCurDBIndex]->GetChildNode(0));
	}
	(*m_vvTableName[m_iCurDBIndex]).clear();

	try
	{
		if (m_vSqliteDB[m_iCurDBIndex]->isOpen())
		{
			CppSQLite3Query query = m_vSqliteDB[m_iCurDBIndex]->execQuery("SELECT name FROM sqlite_master WHERE type='table' order by name");
			
			while (!query.eof())
			{
				const char* res = query.fieldValue(0);
				char*		ch  = Utf82Unicode(res);
				m_vvTableName[m_iCurDBIndex]->push_back(ch);
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
				m_vvTableName[m_iCurDBIndex]->push_back(ch);
				query.nextRow();
			}
			query.finalize();
		}
	}

	for (size_t i = 0; i < m_vvTableName[m_iCurDBIndex]->size(); i++)
	{
		CTreeNodeUI* pTreeNodeElement = new CTreeNodeUI(m_vTreeRootNode[m_iCurDBIndex]);
		pTreeNodeElement->SetName(LPCTSTR((*m_vvTableName[m_iCurDBIndex])[i]));
		pTreeNodeElement->SetItemText(LPCTSTR((*m_vvTableName[m_iCurDBIndex])[i]));

		pTreeNodeElement->GetCheckBox()->SetAttribute(L"width",L"15");
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"height",L"15");
		pTreeNodeElement->GetCheckBox()->SetAttribute(L"normalimage",L"resource/tables.png");

		pTreeNodeElement->GetItemButton()->SetFont(4);
		pTreeNodeElement->GetTreeNodeHoriznotal()->SetToolTip(LPCTSTR((*m_vvTableName[m_iCurDBIndex])[i]));
		pTreeNodeElement->SetParentNode(m_vTreeRootNode[m_iCurDBIndex]);
		if(i&1)
			pTreeNodeElement->SetBkColor(0X11001100);
		else
			pTreeNodeElement->SetBkColor(0X22110022);
		m_vTreeRootNode[m_iCurDBIndex]->Add(pTreeNodeElement);

	}
	m_vTreeRootNode[m_iCurDBIndex]->Select(true);
}

void CDuiFrameWnd::unloadDB()
{
	if (m_iCurDBIndex < 0)
	{
		MessageBoxA(NULL,"��ѡ��һ�����ݿ�","faild",MB_OK);
		return ;
	}
	if (m_vSqliteDB[m_iCurDBIndex]->isOpen())
	{
		m_vSqliteDB[m_iCurDBIndex]->close();
	}

	m_pList->RemoveAll();
	for (size_t i = 0; i < m_vCurListItem.size(); i++)
	{
		m_pList->Remove(m_vCurListItem[i]);
	}
	m_vCurListTextElem.clear();
	m_vCurListItem.clear();

	while (m_vTreeRootNode[m_iCurDBIndex]->GetCountChild())  
	{
		m_vTreeRootNode[m_iCurDBIndex]->Remove(m_vTreeRootNode[m_iCurDBIndex]->GetChildNode(0));
	}
	m_pTreeView->Remove(m_vTreeRootNode[m_iCurDBIndex]);

	m_vNameOfDB.erase(std::find(m_vNameOfDB.begin(),m_vNameOfDB.end(),m_vNameOfDB[m_iCurDBIndex]));
	m_vSqliteDB.erase(std::find(m_vSqliteDB.begin(),m_vSqliteDB.end(),m_vSqliteDB[m_iCurDBIndex]));
	m_vTreeRootNode.erase(std::find(m_vTreeRootNode.begin(),m_vTreeRootNode.end(),m_vTreeRootNode[m_iCurDBIndex])); 
	m_vvTableName.erase(std::find(m_vvTableName.begin(),m_vvTableName.end(),m_vvTableName[m_iCurDBIndex]));
	m_vDBPath.erase(std::find(m_vDBPath.begin(),m_vDBPath.end(),m_pDBPathLabel->GetText()));

	m_pDesignList->RemoveAll();

	if (0 == m_vSqliteDB.size())
	{
		for (size_t i = 0; i < m_vDesignListHeader.size(); i++)
		{
			m_pDesignList->Remove(m_vDesignListHeader[i]);
		}
		m_vDesignListHeader.clear();
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
	for (size_t i = 0; i < m_vCurListTextElem.size(); i++)
	{
		if (msg.pSender == m_vCurListTextElem[i])
		{
			m_pCurListTextElem = m_vCurListTextElem[i];
			m_CDuiStrActiveListItemTableName = m_pCurTreeNode->GetItemText();
			m_pPopWnd->InitWindow();
			m_pPopWnd->CenterWindow();
			m_pPopWnd->ShowWindow(true);
			m_bPopWndIsShowing = true;
			return ;
		}
	}
	for (size_t i =0; i < m_vSqlListTextElem.size(); i++)
	{
		if (msg.pSender == m_vSqlListTextElem[i])
		{
			m_pCurListTextElem = m_vSqlListTextElem[i];

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
		m_vNameOfDB.push_back(addName);

		m_vSqliteDB.push_back(new CppSQLite3DB());
		m_iCurDBIndex = m_vSqliteDB.size() - 1;
		loadDB(path,m_iCurDBIndex);
		m_vDBPath.push_back( LPCTSTR(Utf82Unicode(path)));

		m_vTreeRootNode[m_iCurDBIndex]->Select(true);

		m_pList->RemoveAll();
		for (vector<CListHeaderItemUI*>::iterator ite = m_vCurListItem.begin(); ite != m_vCurListItem.end(); ++ite)
		{
			m_pList->Remove(*ite);
		}
		m_vCurListItem.clear();
		m_vCurListTextElem.clear();

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
	int count = msg.wParam + 1;//�����¼��Ľڵ���treeView�еı��
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
				for (size_t i = 0; i < m_vDesignListHeader.size(); i++)
				{
					m_pDesignList->Remove(m_vDesignListHeader[i]);
				}
				m_vDesignListHeader.clear();

				m_pList->RemoveAll();
				for (size_t i = 0; i < m_vCurListItem.size(); ++i)
				{
					m_pList->Remove(m_vCurListItem[i]);
				}
				m_vCurListItem.clear();
				m_vCurListTextElem.clear();
			}
			else
			{
				m_pCurTreeNode = m_vTreeRootNode[i]->GetChildNode(count - 2);
				string name = m_pCurTreeNode->GetName().GetStringUtf8();
				
				ShowTable(name, m_iCurDBIndex);
				showDesign(name, m_iCurDBIndex);

				if (m_vDesignListHeader.empty())
				{
					addDesignListHeader();
				}
				break;
			}
		}
	}
}

void CDuiFrameWnd::updatePopWnd(TNotifyUI& msg)
{
	CDuiString name = msg.pSender->GetName();

	if (msg.pSender == m_pList || msg.pSender == m_pSqlList)  
	{
		for (size_t i = 0; i < m_vCurListTextElem.size(); i++)
		{
			if ( m_vCurListTextElem[i]->IsSelected() )  //psender ������ m_vCurListTextElem[i]
			{
				m_vCurListTextElem[i]->Select(true);    
				if (true == m_bPopWndIsShowing)
				{
					m_pCurListTextElem = m_vCurListTextElem[i];
					m_CDuiStrActiveListItemTableName = m_pCurTreeNode->GetItemText();

					m_pPopWnd->InitWindow();
					m_pPopWnd->ShowWindow(true);
					m_bPopWndIsShowing = true;
					return ;
				}
			}
		}
		for (size_t i =0; i < m_vSqlListTextElem.size(); i++)
		{
			if ( m_vSqlListTextElem[i]->IsSelected() )
			{
				m_vSqlListTextElem[i]->Select(true);
				if (true == m_bPopWndIsShowing)
				{
					m_pCurListTextElem = m_vSqlListTextElem[i];

					m_pPopWnd->InitWindow();
					m_pPopWnd->ShowWindow(true);
					m_bPopWndIsShowing = true;

					return ;
				}
			}
		}
		return ;
	}

	for (size_t i = 0; i < m_vCurListTextElem.size(); i++)
	{
		if (msg.pSender != m_vCurListTextElem[i] && m_vCurListTextElem[i]->IsSelected())
		{
			m_vCurListTextElem[i]->Select(false);
		}
	}
	for (size_t i =0; i < m_vSqlListTextElem.size(); i++)
	{
		if (msg.pSender != m_vSqlListTextElem[i] && m_vSqlListTextElem[i]->IsSelected())
		{
			m_vSqlListTextElem[i]->Select(false);
		}
	}

	for (size_t i = 0; i < m_vCurListTextElem.size(); i++)
	{
		if (msg.pSender == m_vCurListTextElem[i] )
		{
			m_vCurListTextElem[i]->Select(true);
			if (true == m_bPopWndIsShowing)
			{
				m_pCurListTextElem = m_vCurListTextElem[i];
				m_CDuiStrActiveListItemTableName = m_pCurTreeNode->GetItemText();

				m_pPopWnd->InitWindow();
				m_pPopWnd->ShowWindow(true);
				m_bPopWndIsShowing = true;
				return ;
			}
		}
	}
	for (size_t i =0; i < m_vSqlListTextElem.size(); i++)
	{
		if (msg.pSender == m_vSqlListTextElem[i] )
		{
			m_vSqlListTextElem[i]->Select(true);
			if (true == m_bPopWndIsShowing)
			{
				m_pCurListTextElem = m_vSqlListTextElem[i];

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

void CDuiFrameWnd::ShowList(string sql,int DBIndex,CListUI* pList, vector<CListHeaderItemUI* >& vCurListHeader, vector<CListTextElementUI*>& vCurTextElem)
{
	if (DBIndex < 0 )
	{
		return ;
	}

	pList->RemoveAll();
	for (size_t i = 0; i < vCurListHeader.size(); i++)
		pList->Remove(vCurListHeader[i]);
	vCurTextElem.clear();
	vCurListHeader.clear();

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

			//��ȡ����Ŀ�Ⱥ͸߶�
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

			vCurListHeader.push_back(pListHItem);
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
			vCurTextElem.push_back(pListElement);
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
	ShowList(sql,DBIndex,m_pList, m_vCurListItem, m_vCurListTextElem);
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
	else if (msg.sType == _T("click"))  //��Ӧ��ť�ĵ��
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
