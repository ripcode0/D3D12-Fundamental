#include "application.h"
#include "context.h"

application::application(int cx, int cy)
{
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = application::wnd_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "d3d12 sample";

    RegisterClassExA(&wc);

    int x = (GetSystemMetrics(SM_CXSCREEN) - cx ) /2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - cy ) /2;

    m_hwnd = ::CreateWindowExA(NULL, wc.lpszClassName, wc.lpszClassName,
        WS_OVERLAPPEDWINDOW, x, y, cx, cy,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    m_context = new d3d12_context(cx, cy, m_hwnd);

    ::ShowWindow(m_hwnd, SW_SHOW);
}

application::~application() {
    safe_delete(m_context);
}

int application::exec(void *scene)
{
    MSG msg{};

    while (msg.message != WM_QUIT)
    {
        if(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)){
            if((msg.message == WM_KEYDOWN) &&(msg.wParam == VK_ESCAPE)){
                PostQuitMessage(0);
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        /* code */
    }
    

    return static_cast<int>(msg.lParam);
}

LRESULT application::wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return DefWindowProc(hwnd, msg, wp, lp);
}
