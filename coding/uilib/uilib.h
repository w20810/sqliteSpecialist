#ifndef UIlib_h__
#define UIlib_h__

#ifndef UILIB_EXPORTS
#	define UILIB_API
#else
#	ifdef UILIB_EXPORTS
#		if _MSC_VER >= 1500
#			define UILIB_API __declspec(dllexport)
#		else
#			define UILIB_API 
#		endif
#	else
#		if _MSC_VER >= 1500
#			define UILIB_API __declspec(dllimport)
#		else
#			define UILIB_API 
#		endif
#	endif
#endif

#define UILIB_COMDAT __declspec(selectany) 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stddef.h>
#include <richedit.h>
#include <tchar.h>
#include <assert.h>
#include <crtdbg.h>
#include <malloc.h>
#include <atlbase.h>
#include <atlstr.h>
#include <xstring>
#include <comdef.h>

#include <algorithm>
using namespace std;

#include <gdiplus.h>

#pragma comment(lib,"oledlg.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"gdiplus.lib")

#include "utils/duipointer.h"
#include "utils/utils.h"
#include "utils/uishadow.h"
#include "utils/internal.h"
#include "utils/uidelegate.h"
#include "utils/duiautocomplete.h"
#include "utils/duitrayicon.h"

#include "core/uitimer.h"
#include "core/uidefine.h"
#include "core/uibase.h"
#include "core/uimanager.h"
#include "core/uidxanimation.h"

#include "core/uicontrol.h"
#include "core/uiContainer.h"
#include "core/uimarkup.h"
#include "core/uidlgBuilder.h"
#include "core/uirender.h"

#include "layout/uiverticallayout.h"
#include "layout/uihorizontallayout.h"
#include "layout/uihorizontallayoutfunction.h"
#include "layout/uitilelayout.h"
#include "layout/uitablayout.h"
#include "layout/uichildlayout.h"

#include "control/uilist.h"
#include "control/uidynamiclist.h"
#include "control/uicombo.h"
#include "control/uiscrollbar.h"
#include "control/uitreeview.h"
#include "control/uichartview.h"

#include "control/uilabel.h"
#include "control/uitext.h"
#include "control/uiedit.h"
#include "control/uieditdisablermenu.h"
#include "control/uigifanim.h"

#include <vector>
#include <deque>
#include "control/uianimation.h"
#include "layout/uianimationtablayout.h"
#include "control/uifadebutton.h"
#include "control/uibutton.h"
#include "control/uibuttonex.h"
#include "control/uioption.h"
#include "control/uioptionex.h"
#include "control/uicheckbox.h"

#include "control/uiprogress.h"
#include "control/uislider.h"

#include "control/uicombobox.h"
#include "control/uirichedit.h"
#include "control/uidatetime.h"

#include "control/uiactivex.h"
#include "control/uiwebbrowser.h"
#include "control/uiflash.h"

#include "control/uimenu.h"


#include "utils/winimplbase.h"
#include "utils/iwindowbase.h"

namespace DuiLib = UiLib;
#endif // UIlib_h__
