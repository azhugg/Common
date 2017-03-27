////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2015 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#ifndef SFML_WINDOWIMPLWIN32_HPP
#define SFML_WINDOWIMPLWIN32_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "../Event.hpp"
#include "../WindowImpl.hpp"
#include <string>
#include <windows.h>
#include <cstdint>

namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Windows implementation of WindowImpl
///
////////////////////////////////////////////////////////////
	class WindowImplWin32 : public WindowImpl
	{
	public:

		////////////////////////////////////////////////////////////
		/// \brief Construct the window implementation from an existing control
		///
		/// \param handle Platform-specific handle of the control
		///
		////////////////////////////////////////////////////////////
		WindowImplWin32(WindowHandle handle);

		////////////////////////////////////////////////////////////
		/// \brief Create the window implementation
		///
		/// \param mode  Video mode to use
		/// \param title Title of the window
		/// \param style Window style
		/// \param settings Additional settings for the underlying OpenGL context
		///
		////////////////////////////////////////////////////////////
		WindowImplWin32(VideoMode mode, const std::string& title, std::uint32_t style);

		////////////////////////////////////////////////////////////
		/// \brief Destructor
		///
		////////////////////////////////////////////////////////////
		~WindowImplWin32();

		////////////////////////////////////////////////////////////
		/// \brief Get the OS-specific handle of the window
		///
		/// \return Handle of the window
		///
		////////////////////////////////////////////////////////////
		virtual WindowHandle getSystemHandle() const;

		////////////////////////////////////////////////////////////
		/// \brief Get the position of the window
		///
		/// \return Position of the window, in pixels
		///
		////////////////////////////////////////////////////////////
		virtual iPoint getPosition() const;

		////////////////////////////////////////////////////////////
		/// \brief Change the position of the window on screen
		///
		/// \param position New position of the window, in pixels
		///
		////////////////////////////////////////////////////////////
		virtual void setPosition(const iPoint& position);

		////////////////////////////////////////////////////////////
		/// \brief Get the client size of the window
		///
		/// \return Size of the window, in pixels
		///
		////////////////////////////////////////////////////////////
		virtual uSize getSize() const;

		////////////////////////////////////////////////////////////
		/// \brief Change the size of the rendering region of the window
		///
		/// \param size New size, in pixels
		///
		////////////////////////////////////////////////////////////
		virtual void setSize(const uSize& size);

		////////////////////////////////////////////////////////////
		/// \brief Change the title of the window
		///
		/// \param title New title
		///
		////////////////////////////////////////////////////////////
		virtual void setTitle(const std::string& title);

		////////////////////////////////////////////////////////////
		/// \brief Change the window's icon
		///
		/// \param width  Icon's width, in pixels
		/// \param height Icon's height, in pixels
		/// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
		///
		////////////////////////////////////////////////////////////
		virtual void setIcon(unsigned int width, unsigned int height, const std::uint8_t* pixels);

		////////////////////////////////////////////////////////////
		/// \brief Show or hide the window
		///
		/// \param visible True to show, false to hide
		///
		////////////////////////////////////////////////////////////
		virtual void setVisible(bool visible);
		virtual void maximize();
		virtual void minimize();
		virtual void restore();
		virtual void setAlpha(float alpha01);
		////////////////////////////////////////////////////////////
		/// \brief Show or hide the mouse cursor
		///
		/// \param visible True to show, false to hide
		///
		////////////////////////////////////////////////////////////
		virtual void setMouseCursorVisible(bool visible);

		////////////////////////////////////////////////////////////
		/// \brief Grab or release the mouse cursor
		///
		/// \param grabbed True to enable, false to disable
		///
		////////////////////////////////////////////////////////////
		virtual void setMouseCursorGrabbed(bool grabbed);

		////////////////////////////////////////////////////////////
		/// \brief Set the displayed cursor to a native system cursor
		///
		/// \param cursor Native system cursor type to display
		///
		////////////////////////////////////////////////////////////
		virtual void setMouseCursor(Window::Cursor cursor);
		
		////////////////////////////////////////////////////////////
		/// \brief Set the displayed cursor to the provided image
		///
		/// \param pixels   Array of pixels of the image
		/// \param width    Width of the image
		/// \param height   Height of the image
		/// \param hotspotX X location of the hotspot
		/// \param hotspotY Y location of the hotspot
		///
		////////////////////////////////////////////////////////////
		virtual void setMouseCursor(const std::uint8_t* pixels, unsigned int width, unsigned int height, unsigned int hotspotX, unsigned int hotspotY);
		
		////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////
		/// \brief Enable or disable automatic key-repeat
		///
		/// \param enabled True to enable, false to disable
		///
		////////////////////////////////////////////////////////////
		virtual void setKeyRepeatEnabled(bool enabled);

		////////////////////////////////////////////////////////////
		/// \brief Request the current window to be made the active
		///        foreground window
		///
		////////////////////////////////////////////////////////////
		virtual void requestFocus();

		////////////////////////////////////////////////////////////
		/// \brief Check whether the window has the input focus
		///
		/// \return True if window has focus, false otherwise
		///
		////////////////////////////////////////////////////////////
		virtual bool hasFocus() const;

	protected:

		////////////////////////////////////////////////////////////
		/// \brief Process incoming events from the operating system
		///
		////////////////////////////////////////////////////////////
		virtual void processEvents();

	private:

		////////////////////////////////////////////////////////////
		/// Register the window class
		///
		////////////////////////////////////////////////////////////
		void registerWindowClass();

		////////////////////////////////////////////////////////////
		/// \brief Switch to fullscreen mode
		///
		/// \param mode Video mode to switch to
		///
		////////////////////////////////////////////////////////////
		void switchToFullscreen(const VideoMode& mode);

		////////////////////////////////////////////////////////////
		/// \brief Free all the graphical resources attached to the window
		///
		////////////////////////////////////////////////////////////
		void cleanup();

		////////////////////////////////////////////////////////////
		/// \brief Process a Win32 event
		///
		/// \param message Message to process
		/// \param wParam  First parameter of the event
		/// \param lParam  Second parameter of the event
		///
		////////////////////////////////////////////////////////////
		void processEvent(UINT message, WPARAM wParam, LPARAM lParam);

		////////////////////////////////////////////////////////////
		/// \brief Enables or disables tracking for the mouse cursor leaving the window
		///
		/// \param track True to enable, false to disable
		///
		////////////////////////////////////////////////////////////
		void setTracking(bool track);

		////////////////////////////////////////////////////////////
		/// \brief Grab or release the mouse cursor
		///
		/// This is not to be confused with setMouseCursorGrabbed.
		/// Here m_cursorGrabbed is not modified; it is used,
		/// for example, to release the cursor when switching to
		/// another application.
		///
		/// \param grabbed True to enable, false to disable
		///
		////////////////////////////////////////////////////////////
		void grabCursor(bool grabbed);

		////////////////////////////////////////////////////////////
		/// \brief Convert a Win32 virtual key code to a SFML key code
		///
		/// \param key   Virtual key code to convert
		/// \param flags Additional flags
		///
		/// \return SFML key code corresponding to the key
		///
		////////////////////////////////////////////////////////////
		static Keyboard::Key virtualKeyCodeToSF(WPARAM key, LPARAM flags);

		////////////////////////////////////////////////////////////
		/// \brief Function called whenever one of our windows receives a message
		///
		/// \param handle  Win32 handle of the window
		/// \param message Message received
		/// \param wParam  First parameter of the message
		/// \param lParam  Second parameter of the message
		///
		/// \return True to discard the event after it has been processed
		///
		////////////////////////////////////////////////////////////
		static LRESULT CALLBACK globalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

		////////////////////////////////////////////////////////////
		// Member data
		////////////////////////////////////////////////////////////
		HWND     m_handle;           ///< Win32 handle of the window
		LONG_PTR m_callback;         ///< Stores the original event callback function of the control
		HCURSOR  m_cursor;           ///< The system cursor to display into the window
		HCURSOR  m_loadedCursor;     ///< The system cursor selected to be displayed into the window
		HICON    m_icon;             ///< Custom icon assigned to the window
		bool     m_keyRepeatEnabled; ///< Automatic key-repeat state for keydown events
		uSize	 m_lastSize;         ///< The last handled size of the window
		bool     m_resizing;         ///< Is the window being resized?
		std::uint16_t   m_surrogate;        ///< First half of the surrogate pair, in case we're receiving a Unicode character in two events
		bool     m_mouseInside;      ///< Mouse is inside the window?
		bool     m_fullscreen;       ///< Is the window fullscreen?
		bool     m_cursorGrabbed;    ///< Is the mouse cursor trapped?
	};

} // namespace priv

} // namespace sf

#endif // SFML_WINDOWIMPLWIN32_HPP
