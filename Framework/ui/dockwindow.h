//
// 2017-04-05, jjuiddong
// Docking Window
//
#pragma once

#include "dock.h"


namespace framework
{
	class cRenderWindow;

	class cDockWindow
	{
	public:
		cDockWindow();
		virtual ~cDockWindow();

		virtual bool Create(const eDockState::Enum state, const eDockType::Enum type, 
			cRenderWindow *owner, cDockWindow *parent);
		virtual bool Dock(const eDockType::Enum type, 
			cDockWindow *child);
		virtual bool Undock(const bool newWindow=true);
		virtual bool Undock(cDockWindow *dock);
		void RenderDock(const Vector2 &pos= Vector2(0, 0));
		virtual void Update(const float deltaSeconds);
		virtual void Render() {}
		virtual void RenderTab();
		virtual bool RemoveTab(cDockWindow *tab);
		virtual void LostDevice();
		virtual void ResetDevice(graphic::cRenderer *shared = NULL);
		void CalcWindowSize(cDockWindow *dock);
		void CalcResizeWindow(const sRectf &rect);
		void CalcResizeWindow(const int deltaSize);
		bool IsInSizerSpace(const Vector2 &pos);
		eDockSizingType::Enum GetDockSizingType();
		void SetBindState(const bool enable=true);
		void ClearConnection();
		void Clear();


	protected:
		void SetParentDockPtr(cDockWindow *dock);
		bool Merge(cDockWindow *udock);
		cDockWindow* UndockTab();
		eDockType::Enum render_dock_slot_preview(const ImVec2& mouse_pos, const ImVec2& cPos, const ImVec2& cSize);
		virtual void OnLostDevice() {}
		virtual void OnResetDevice(graphic::cRenderer *shared) {}


	public:
		bool m_isBind;
		eDockState::Enum m_state;
		eDockType::Enum m_dockType;
		cRenderWindow *m_owner;
		cDockWindow *m_lower; // only availible for VIRTUAL state
		cDockWindow *m_upper; // only availible for VIRTUAL state
		cDockWindow *m_parent;
		vector<cDockWindow*> m_tabs;
		int m_selectTab;
		string m_name;
		sRectf m_rect;
		static int s_id;
	};

}