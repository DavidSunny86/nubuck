#pragma once

#include <exception>
#include <Windows.h>

#include <Nubuck\generic\uncopyable.h>

namespace SYS {

	class Window;

	struct PixelFormatException : std::exception {
		const char* what(void) const {
			return "setting pixel format failed";
		}
	};

	class DeviceContext : private GEN::Uncopyable {
	protected:
		HWND _windowHandle;
		HDC _contextHandle;
	public:
		DeviceContext(Window& window);
        explicit DeviceContext(HWND windowHandle);
		virtual ~DeviceContext(void);

		HDC GetNativeHandle(void) { return _contextHandle; }
	};

	class RenderingContext : private GEN::Uncopyable {
	private:
		DeviceContext _deviceContext;
		HGLRC _contextHandle;
		bool _initialized;

		static bool _extensionsInitialized;
        static int _major, _minor;
		static void InitExtensions(void);

        void Init(void);
	public:
		RenderingContext(Window& window);
        explicit RenderingContext(HWND windowHandle);
		~RenderingContext(void);

		void Use(void);
		void Flip(void);
	};

	bool IsRenderingContextActive(void);

} // namespace SYS