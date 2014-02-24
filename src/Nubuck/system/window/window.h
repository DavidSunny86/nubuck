#pragma once

#include <Windows.h>

#include <Nubuck\generic/uncopyable.h>

namespace SYS {

	struct MouseEvent {
		enum Type {
			TYPE_BUTTON_DOWN,
			TYPE_BUTTON_UP,
			TYPE_MOVE
		};
		enum Button {
			BUTTON_LEFT,
			BUTTON_RIGHT,
			BUTTON_NONE
		};

		Type type;
		Button reason;

		int flags;
		int x, y;
	};

	const DWORD WS_NONRESIZABLE = 
		WS_OVERLAPPED | 
		WS_CAPTION | 
		WS_SYSMENU | 
		WS_MINIMIZEBOX;

	class Window : private GEN::Uncopyable {
		friend LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
	private:
		HWND _windowHandle;
		unsigned _width;
		unsigned _height;

		bool _exitOnClose;
	protected:
		virtual void ResizeEvent(unsigned width, unsigned height) { 
			_width = width;
			_height = height;
		}
		virtual void MouseEvent(const MouseEvent& event) { }
		virtual void CloseEvent(void) { }
	public:
		Window(HINSTANCE instance, const TCHAR* title,
			   unsigned width, unsigned height, 
			   DWORD style = WS_OVERLAPPEDWINDOW);
		virtual ~Window(void);

		void SetExitOnClose(bool exitOnClose);

		unsigned Width(void) const { return _width; }
		unsigned Height(void) const { return _height; }

		HWND GetNativeHandle(void) { return _windowHandle; }
	};

} // namespace SYS