#pragma once

#include "config.h"

class d3d12_context;
class application
{
public:
    application(int cx, int cy);
    ~application();

    int exec(void* scene = nullptr);

    d3d12_context* m_context;
protected:
    static LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND m_hwnd{};
};