#ifndef __UISLIDER_H__
#define __UISLIDER_H__

#pragma once

namespace UiLib
{
	class UILIB_API CSliderUI : public CProgressUI
	{
	public:
		CSliderUI();

		LPCTSTR GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(LPCTSTR pstrName);

		void SetEnabled(bool bEnable = true);

		int GetChangeStep();
		void SetChangeStep(int step);
		void SetThumbSize(SIZE szXY);
		RECT GetThumbRect() const;
		LPCTSTR GetThumbImage() const;
		void SetThumbImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbHotImage() const;
		void SetThumbHotImage(LPCTSTR pStrImage);
		LPCTSTR GetThumbPushedImage() const;
		void SetThumbPushedImage(LPCTSTR pStrImage);

		void DoEvent(TEventUI& event);//2014.7.28 redrain
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void PaintStatusImage(HDC hDC);

		void SetValue(int nValue);//2014.7.28 redrain
		void SetCanSendMove(bool bCanSend);//2014.7.28 redrain
		bool GetCanSendMove() const;//2014.7.28 redrain
	protected:
		SIZE m_szThumb;
		UINT m_uButtonState;
		int m_nStep;

		CDuiString m_sThumbImage;
		CDuiString m_sThumbHotImage;
		CDuiString m_sThumbPushedImage;

		CDuiString m_sImageModify;
		bool	   m_bSendMove;//2014.7.28 redrain
	};
}

#endif // __UISLIDER_H__