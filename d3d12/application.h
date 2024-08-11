#pragma once

#include "config.h"

class d3d12_context;
class DX12Context;
class Scene;
class Application
{
public:
    Application(int cx, int cy);
    ~Application();

    int exec(Scene* scene = nullptr);

    
    DX12Context* m_context{};
protected:
    static LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND m_hwnd{};
};