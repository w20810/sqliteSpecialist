#include "stdafx.h"
#include "CDuiFrame.h"

using namespace std;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);

	HRESULT Hr = ::CoInitialize(NULL);
	if(FAILED(Hr))
		return 0;

	CDuiFrameWnd duiFrame;
	duiFrame.Create(NULL,_T("sqliteSpecialist"),UI_WNDSTYLE_FRAME,WS_EX_WINDOWEDGE);
	duiFrame.CenterWindow();
	duiFrame.ShowWindow();
	duiFrame.ShowModal();
	::CoUninitialize();
	
	return 0;
}
