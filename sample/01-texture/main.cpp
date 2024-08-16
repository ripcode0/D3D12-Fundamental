#include "application.h"
#include "texture.h"

int main(int args, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Application app(1024, 680);
    
    TextureScene scene(app.m_context);

    return app.exec(&scene);
}