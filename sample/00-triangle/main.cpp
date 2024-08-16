#include <iostream>

#include "application.h"
#include "Triangle.h"

int main(int args, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    Application app{860, 860};
    
    TriangleScene scene(app.m_context);   
    
    int res = app.exec(&scene);

    return res;
}