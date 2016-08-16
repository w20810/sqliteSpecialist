mxx_package(uilib STATIC)
	mxx_add_definitions(
		_USRDLL
		UILIB_EXPORTS
		)
	mxx_include_directories(
		../../modules/log/src
		../123
		)
#	mxx_link_packages(
#		log
#		)
	mxx_add_sources(
		PCH stdafx.h stdafx.cpp
#		control/wke/wke.h
#		control/wke/wkewebkit.cpp
#		control/wke/wkewebkit.h
		control/uiactivex.cpp
		control/uiactivex.h
		control/uianimation.cpp
		control/uianimation.h
		control/uibutton.cpp
		control/uibutton.h
		control/uibuttonex.cpp
		control/uibuttonex.h
		control/uichartview.cpp
		control/uichartview.h
		control/uicheckbox.cpp
		control/uicheckbox.h
		control/uicombo.cpp
		control/uicombo.h
		control/uicombobox.cpp
		control/uicombobox.h
		control/uidatetime.cpp
		control/uidatetime.h
		control/uiedit.cpp
		control/uiedit.h
		control/uieditdisablermenu.cpp
		control/uieditdisablermenu.h
		control/uifadebutton.cpp
		control/uifadebutton.h
		control/uiflash.cpp
		control/uiflash.h
		control/uigifanim.cpp
		control/uigifanim.h
		control/uilabel.cpp
		control/uilabel.h
		control/uidynamiclist.cpp
		control/uidynamiclist.h
		control/uilist.cpp
		control/uilist.h
		control/uilist_i.h
		control/uimenu.cpp
		control/uimenu.h
		control/uioption.cpp
		control/uioption.h
		control/uioptionex.cpp
		control/uioptionex.h
		control/uiprogress.cpp
		control/uiprogress.h
		control/uirichedit.cpp
		control/uirichedit.h
		control/uiscrollbar.cpp
		control/uiscrollbar.h
		control/uislider.cpp
		control/uislider.h
		control/uitext.cpp
		control/uitext.h
		control/uitreeview.cpp
		control/uitreeview.h
		control/uiwebbrowser.cpp
		control/uiwebbrowser.h
		core/uibase.cpp
		core/uibase.h
		core/uicontainer.cpp
		core/uicontainer.h
		core/uicontrol.cpp
		core/uicontrol.h
		core/uidefine.h
		core/uidlgbuilder.cpp
		core/uidlgbuilder.h
		core/uidxanimation.cpp
		core/uidxanimation.h
		core/uimanager.cpp
		core/uimanager.h
		core/uimarkup.cpp
		core/uimarkup.h
		core/uirender.cpp
		core/uirender.h
		core/uitimer.cpp
		core/uitimer.h
		layout/uianimationtablayout.cpp
		layout/uianimationtablayout.h
		layout/uichildlayout.cpp
		layout/uichildlayout.h
		layout/uihorizontallayout.cpp
		layout/uihorizontallayout.h
		layout/uihorizontallayoutfunction.cpp
		layout/uihorizontallayoutfunction.h
		layout/uitablayout.cpp
		layout/uitablayout.h
		layout/uitilelayout.cpp
		layout/uitilelayout.h
		layout/uiverticallayout.cpp
		layout/uiverticallayout.h
		utils/downloadmgr.h
		utils/duiautocomplete.cpp
		utils/duiautocomplete.h
		utils/duipointer.h
		utils/duitrayicon.cpp
		utils/duitrayicon.h
		utils/flasheventhandler.h
		utils/internal.h
		utils/iwindowbase.cpp
		utils/iwindowbase.h
		utils/stb_image.c
		utils/uidelegate.cpp
		utils/uidelegate.h
		utils/uishadow.cpp
		utils/uishadow.h
		utils/utils.cpp
		utils/utils.h
		utils/webbrowsereventhandler.h
		utils/winimplbase.cpp
		utils/winimplbase.h
		utils/xunzip.cpp
		uilib.cpp
		uilib.h
		)

#	mxx_link_wke()

	mxx_declare_fileinfo(
		FILE_DESCRIPTION "direct ui"
		)
mxx_end_package()

