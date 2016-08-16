#include "PopWnd.h"
#include <regex>

LPCTSTR PopWnd::GetWindowClassName()const 
{
	return _T("PopWnd");
}

CDuiString PopWnd::GetSkinFile()
{
	return _T("resource/PopWnd.xml");
}
CDuiString PopWnd::GetSkinFolder()
{
	return _T("");
}
CDuiString GetValuesFormat(size_t propertyNumbers)
{
	if (propertyNumbers == 0)
	{
		return L" values()";
	}

	CDuiString str(L" values(?");
	for (size_t i = 0; i < propertyNumbers - 1; i++)
	{
		str += L",?";
	}

	str += L")";
	return str;
}
bool checkDate(const char* strDate)
{
	const std::regex pattern("\\d{4}-(10|11|12|0?[1-9])-(30|31|[1-2][0-9]|0?[1-9])");
	if(!regex_match(strDate,pattern))
		return false;
	int iyear = 0,imonth = 0,iday  =0;
	sscanf(strDate,"%d-%d-%d",&iyear,&imonth,&iday);
	if (2 == imonth )
	{
		if ( 0 == iyear%400 || (iyear % 4 == 0 && iyear % 100 != 0) )
		{
			if (iday>29 || iday<1)
			{
				return false;
			}
		}
		else
		{
			if (iday>28 || iday<1)
			{
				return false;
			}
		}
	}
	else if (imonth == 1 || imonth == 3 || imonth == 5 || imonth == 7 || imonth == 8 || imonth == 10 || imonth == 12)
	{
		if (iday > 31 || iday < 1)
		{
			return false;
		}
	}
	else
	{
		if (iday > 30 || iday < 1)
		{
			return false;
		}
	}
	return true;
}

void PopWnd::clearFrame()
{
	for (size_t i = 0; i < m_vPropertyName.size(); i++)
	{
		m_pDomain->Remove(m_vPropertyName[i]);
		m_pDomain->Remove(m_vData[i]);
	}
	m_vPropertyName.clear();
	m_vData.clear();
	m_vOriginalData.clear();
}

void PopWnd::DeletAndSelectOtherTextItem(CListUI* pFrameList, vector<CListTextElementUI*>& vFrameTextElem, CListTextElementUI* &pDeleteTextElem)
{
	CListTextElementUI*		pPreviousTextElem = NULL;
	for (vector<CListTextElementUI*>::iterator ite =vFrameTextElem.begin(); ite != vFrameTextElem.end(); ++ite)
	{
		if (*ite == pDeleteTextElem)
		{
			pFrameList->Remove(pDeleteTextElem);

			if (ite == vFrameTextElem.begin())
			{
				if (ite + 1 != vFrameTextElem.end())
				{
					CListTextElementUI* pNextTextElem = *(ite + 1) ;
					pDeleteTextElem = pNextTextElem;
					pNextTextElem->Select(true);

					for (size_t i = 0; i < m_vData.size(); ++i)
					{
						m_vData[i]->SetText(pNextTextElem->GetText(i));
						m_vOriginalData[i] = m_vData[i]->GetText();
					}
				}
				else
				{
					pDeleteTextElem = NULL ; 
					for (size_t i = 0; i < m_vData.size(); ++i)
					{
						m_vData[i]->SetText(L"");
						m_vOriginalData[i]=m_vData[i]->GetText();
					}
				}	
			}
			else
			{
				pPreviousTextElem = *(ite - 1) ;
				pDeleteTextElem   = pPreviousTextElem;
				pPreviousTextElem->Select(true);

				for (size_t i = 0; i < m_vData.size(); ++i)
				{
					m_vData[i]->SetText(pPreviousTextElem->GetText(i));
					m_vOriginalData[i]=m_vData[i]->GetText();
				}
			}
			vFrameTextElem.erase(ite);
			break;
		}
	}
}

void PopWnd::OnPreviousBtn()
{
	vector<CListTextElementUI*> vFrameTextElem;
	CListTextElementUI* &pCurTextElem =  m_pParentWnd->m_pCurListTextElem;
	if (m_pParentWnd->m_pList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vCurListTextElem;
	}
	else if (m_pParentWnd->m_pSqlList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vSqlListTextElem;
	}
	for (vector<CListTextElementUI*>::iterator ite = vFrameTextElem.begin(); ite != vFrameTextElem.end(); ++ite)
	{
		if (*ite == pCurTextElem)
		{
			if (ite == vFrameTextElem.begin())
			{
				return ;
			}
			else
			{
				pCurTextElem->Select(false);

				CListTextElementUI* pPreviousTextElem = *(ite - 1);
				pCurTextElem = pPreviousTextElem;
				pPreviousTextElem->Select(true);

				for (size_t i = 0; i < m_vData.size(); ++i)
				{
					m_vData[i]->SetText(pPreviousTextElem->GetText(i));
					m_vOriginalData[i]=m_vData[i]->GetText();
				}
			}
			break;
		}
	}
}

void PopWnd::OnNextBtn()
{
	vector<CListTextElementUI*> vFrameTextElem;
	CListTextElementUI* &pCurTextElem =  m_pParentWnd->m_pCurListTextElem;
	if (m_pParentWnd->m_pList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vCurListTextElem;
	}
	else if (m_pParentWnd->m_pSqlList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vSqlListTextElem;
	}


	for (vector<CListTextElementUI*>::iterator ite = vFrameTextElem.begin(); ite != vFrameTextElem.end(); ++ite)
	{
		if (*ite == pCurTextElem)
		{
			if (ite + 1 == vFrameTextElem.end())
			{
				return ;
			}
			else
			{
				pCurTextElem->Select(false);

				CListTextElementUI* pNextTextElem = *(ite + 1);
				pCurTextElem = pNextTextElem;
				pNextTextElem->Select(true);

				for (size_t i = 0; i < m_vData.size(); ++i)
				{
					m_vData[i]->SetText(pNextTextElem->GetText(i));
					m_vOriginalData[i]=m_vData[i]->GetText();
				}
			}
			break;
		}
	}
	
}

void PopWnd::OnFirstBtn()
{
	vector<CListTextElementUI*> vFrameTextElem;
	CListTextElementUI* &pCurTextElem =  m_pParentWnd->m_pCurListTextElem;
	if (m_pParentWnd->m_pList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vCurListTextElem;
	}
	else if (m_pParentWnd->m_pSqlList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vSqlListTextElem;
	}

	if (!vFrameTextElem.empty())
	{
		pCurTextElem->Select(false);

		pCurTextElem = *vFrameTextElem.begin();
		pCurTextElem->Select(true);

		for (size_t i = 0; i < m_vData.size(); ++i)
		{
			m_vData[i]->SetText(pCurTextElem->GetText(i));
			m_vOriginalData[i]=m_vData[i]->GetText();
		}
	}
}

void PopWnd::OnLastBtn()
{
	vector<CListTextElementUI*> vFrameTextElem;
	CListTextElementUI* &pCurTextElem =  m_pParentWnd->m_pCurListTextElem;
	if (m_pParentWnd->m_pList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vCurListTextElem;
	}
	else if (m_pParentWnd->m_pSqlList->Activate())
	{
		vFrameTextElem = m_pParentWnd->m_vSqlListTextElem;
	}
	
	if (!vFrameTextElem.empty())
	{
		pCurTextElem->Select(false);

		pCurTextElem = *(vFrameTextElem.end()-1);
		pCurTextElem->Select(true);

		for (size_t i = 0; i < m_vData.size(); ++i)
		{
			m_vData[i]->SetText(pCurTextElem->GetText(i));
			m_vOriginalData[i]=m_vData[i]->GetText();
		}
	}
}

void PopWnd::OnDeleteBtn()
{
	try 
	{
		m_enumSQL_STATU						=	YX__SQL__DELETE;
		CDuiString		strTableName		=	m_pParentWnd->m_CDuiStrActiveListItemTableName;
		CDuiString		strPrimaryKey		=	m_vPropertyName[0]->GetText();
		CDuiString		strPrimaryKeyData	=	m_vOriginalData[0];
		CDuiString		sql					=	CDuiString(L"delete from ") + strTableName+CDuiString(L" where ")+strTableName+CDuiString(L".[")+strPrimaryKey+CDuiString(L"] ")+CDuiString(L" = \"") + strPrimaryKeyData + CDuiString(L"\" ");
		CppSQLite3Query query				=	m_pParentWnd->m_vSqliteDB[m_pParentWnd->m_iCurDBIndex]->execQuery(sql.GetStringUtf8().c_str());
		query.finalize();


		if (m_pParentWnd->m_pList->Activate())
		{
			DeletAndSelectOtherTextItem(m_pParentWnd->m_pList,m_pParentWnd->m_vCurListTextElem,m_pParentWnd->m_pCurListTextElem);
		}
		else if (m_pParentWnd->m_pSqlList->Activate())
		{
			DeletAndSelectOtherTextItem(m_pParentWnd->m_pSqlList,m_pParentWnd->m_vSqlListTextElem,m_pParentWnd->m_pCurListTextElem);
		}
	}
	catch(CppSQLite3Exception e)
	{
		MessageBoxA(NULL,e.errorMessage(),"Error",MB_OK);
	}
}

void PopWnd::OnAddBtn()
{
	for (size_t i = 0; i < m_vData.size(); i++)
	{
		m_vData[i]->SetText(L"");
	}
	m_enumSQL_STATU = YX__SQL__ADD ;
}

void PopWnd::OnOkBtn()
{
	if (YX__SQL__ADD == m_enumSQL_STATU)
	{
		try
		{
			CDuiString			strTableName	=	m_pParentWnd->m_CDuiStrActiveListItemTableName;
			CDuiString			sql				=	CDuiString(L"insert into ") + strTableName + GetValuesFormat(m_vData.size());
			int					a				=	1;
			CppSQLite3Statement stm				=	m_pParentWnd->m_vSqliteDB[m_pParentWnd->m_iCurDBIndex]->compileStatement(sql.GetStringA().c_str());

								sql				=	CDuiString(L"select * from ") + strTableName;
			CppSQLite3Query		query			=	m_pParentWnd->m_vSqliteDB[m_pParentWnd->m_iCurDBIndex]->execQuery(sql.GetStringA().c_str());
			for (size_t i = 0; i < m_vData.size(); i++)
			{
				stm.bind(i+1,m_vData[i]->GetText().GetStringUtf8().c_str());
				if (string(query.fieldDeclType(i)) == string("date"))
				{
					if (!checkDate(m_vData[i]->GetText().GetStringA().c_str()))
					{
						stm.finalize();
						MessageBoxA(NULL,"日期格式错误","faild",MB_OK);
						return ;
					}
				}
				query.nextRow();
			}
			stm.execDML();
			stm.finalize();

			//更新list数据
			CListTextElementUI* pListElement = new CListTextElementUI;
			if (m_pParentWnd->m_pList->Activate())
			{
				m_pParentWnd->m_pList->Add(pListElement);
			}
			else if (m_pParentWnd->m_pSqlList->Activate())
			{
				m_pParentWnd->m_pSqlList->Add(pListElement);
			}

			pListElement->SetAttribute(L"margin", L"20, 0, 0, 0");
			pListElement->SetAttribute(L"textpadding",L"20, 20, 20, 20");
			for (size_t i = 0; i < m_vData.size(); i++)
			{
				pListElement->SetText(i,m_vData[i]->GetText());
			}
			if (NULL != m_pParentWnd->m_pCurListTextElem)
			{
				m_pParentWnd->m_pCurListTextElem->Select(false);
			}
			m_pParentWnd->m_pCurListTextElem = pListElement;
			pListElement->Select(true);

			if (m_pParentWnd->m_pList->Activate())
			{
				m_pParentWnd->m_vCurListTextElem.push_back(pListElement);
			}
			else if (m_pParentWnd->m_pSqlList->Activate())
			{
				m_pParentWnd->m_vSqlListTextElem.push_back(pListElement);
			}
		}
		catch(CppSQLite3Exception e)
		{
			MessageBoxA(NULL,e.errorMessage(),"Error",MB_OK);
		}
	}
	else if (YX__SQL__UPDATE == m_enumSQL_STATU)
	{
		try
		{
			if (NULL == m_pParentWnd->m_pCurListTextElem )
			{
				return ;
			}
			CDuiString		strTableName			=	m_pParentWnd->m_CDuiStrActiveListItemTableName;
			CDuiString		sql						=	CDuiString(L"update ")+strTableName+CDuiString(L" set ");
			CppSQLite3Query query					=	m_pParentWnd->m_vSqliteDB[m_pParentWnd->m_iCurDBIndex]->execQuery(CDuiString(CDuiString(L"select* from ")+strTableName).GetStringA().c_str());
			for (size_t i = 0; i < m_vData.size(); i++)
			{
				if (i != 0)
				{
					sql+=L" , ";
				}
				sql	+=	CDuiString(L"\"")+m_vPropertyName[i]->GetText()+CDuiString(L"\"")+L" = \""+ m_vData[i]->GetText()+L"\"";

				if (string(query.fieldDeclType(i))==string("date"))
				{
					if (!checkDate(m_vData[i]->GetText().GetStringA().c_str()))
					{
						MessageBoxA(NULL,"日期格式错误","faild",MB_OK);
						return ;
					}
				}
				query.nextRow();
			}

			sql += CDuiString(L" where ")+strTableName+CDuiString(L".[")+m_vPropertyName[0]->GetText()+CDuiString(L"] ")+L" = \""+m_vOriginalData[0]+L"\"";
			m_pParentWnd->m_vSqliteDB[m_pParentWnd->m_iCurDBIndex]->execDML(sql.GetStringUtf8().c_str());

			for (size_t i = 0; i < m_vData.size(); i++)
			{
				m_pParentWnd->m_pCurListTextElem->SetText(i,m_vData[i]->GetText());
			}
		}
		catch(CppSQLite3Exception e)
		{
			MessageBoxA(NULL,e.errorMessage(),"Error",MB_OK);
		}
	}
}

void PopWnd::loadFrame()
{
	vector<CListHeaderItemUI*>	vParentListItem;
	if (m_pParentWnd->m_pList->Activate())
	{
		vParentListItem = m_pParentWnd->m_vCurListItem;
	}
	else if (m_pParentWnd->m_pSqlList->Activate())
	{
		vParentListItem = m_pParentWnd->m_vSqlListHeader;
	}

	for (size_t i = 0; i < vParentListItem.size(); i++)
	{
		CLabelUI*	pLabel = new CLabelUI;
		pLabel->SetText(vParentListItem[i]->GetText());
		pLabel->SetAttribute(L"textcolor",L"#FF555555");
		pLabel->SetAttribute(L"glowsize",L"0");
		pLabel->SetAttribute(L"width",L"120");
		pLabel->SetAttribute(L"font",L"4");
		m_vPropertyName.push_back(pLabel);
		m_pDomain->Add(pLabel);

		CRichEditUI* pRichEdit = new CRichEditUI ;
		pRichEdit->SetAttribute(L"inset", L"3,3,3,3");
		pRichEdit->SetAttribute(L"bordersize",L"1");
		pRichEdit->SetAttribute(L"bordercolor",L"#FF666666");
		pRichEdit->SetAttribute(L"height",L"50");
		pRichEdit->SetAttribute(L"readonly",L"false");
		pRichEdit->SetAttribute(L"vscrollbar",L"false");
		pRichEdit->SetBkColor(0Xffffffff);
		pRichEdit->SetText(m_pParentWnd->m_pCurListTextElem->GetText(i));
		m_vData.push_back(pRichEdit);
		m_pDomain->Add(pRichEdit);

		m_vOriginalData.push_back(pRichEdit->GetText());
	}
}

void PopWnd::InitWindow()
{
	m_pDomain		=	static_cast<CVerticalLayoutUI*>(m_PaintManager.FindControl(_T("PopWndVerticalLayout")));
	m_pDeleteBtn	=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("DeleteBtn")));
	m_pAddBtn		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("AddBtn")));
	m_pOkBtn		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("OKBtn")));
	m_pFirstBtn		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("GetFirstBtn")));
	m_pPreviousBtn		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("GetPreviousBtn")));
	m_pNextBtn		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("GetNextBtn")));
	m_pLastBtn		=	static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("GetLastBtn")));

	m_enumSQL_STATU =	YX__SQL__UPDATE;
	
	clearFrame();
	loadFrame();
}
void PopWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{
		if (msg.pSender == m_pDeleteBtn)
		{
			OnDeleteBtn();
		}
		else if (msg.pSender == m_pAddBtn)
		{
			OnAddBtn();
		}
		else if (msg.pSender == m_pOkBtn)
		{
			OnOkBtn();
		}
		else if (msg.pSender == m_pFirstBtn)
		{
			OnFirstBtn();
		}
		else if (msg.pSender == m_pPreviousBtn)
		{
			OnPreviousBtn();
		}
		else if (msg.pSender == m_pNextBtn)
		{
			OnNextBtn();
		}
		else if (msg.pSender == m_pLastBtn)
		{
			OnLastBtn();
		}
		else if (msg.pSender->GetName() == _T("closebtn") )
		{
			this->ShowWindow(false);
			m_pParentWnd->PopWndIsShowing = false;
			return; 
		}
	}
	__super::Notify(msg);
}

/////////////////////////////////////
void PopWnd::setParentWnd(CDuiFrameWnd* parentWnd)
{
	this->m_pParentWnd = parentWnd;
}