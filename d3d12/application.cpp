#include "application.h"
#include "DX12.h"
#include "DX12Context.h"
#include "Scene.h"

Application::Application(int cx, int cy)
{
    WNDCLASSEXA wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = Application::wnd_proc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "D3D12 Fundamental";

    RegisterClassExA(&wc);

    int x = (GetSystemMetrics(SM_CXSCREEN) - cx ) /2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - cy ) /2;

    m_hwnd = ::CreateWindowExA(NULL, wc.lpszClassName, wc.lpszClassName,
        WS_OVERLAPPEDWINDOW, x, y, cx, cy,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    //m_context = new d3d12_context(cx, cy, m_hwnd);
    RECT rc{};
    GetClientRect(m_hwnd, &rc);
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
    cx = rc.right - rc.left;
    cy = rc.bottom - rc.top;


    m_context = new DX12Context();
    m_context->initialize(cx, cy, m_hwnd);

    ::ShowWindow(m_hwnd, SW_SHOW);
}

Application::~Application() {
    safe_delete(m_context);
}

int Application::exec(Scene* current_scene)
{
    if(current_scene){
        current_scene->initialize();
    }
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
        //making ur own math lib
        else{
            m_context->begine_frame();

            if(current_scene){
               current_scene->update_frame(0.f);
               current_scene->render_frame();
            }

            m_context->end_frame();
        }
        
    }
    
    if(current_scene) current_scene->release();
    safe_delete(m_context);

    return static_cast<int>(msg.lParam);
}

LRESULT Application::wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    return DefWindowProc(hwnd, msg, wp, lp);
}
