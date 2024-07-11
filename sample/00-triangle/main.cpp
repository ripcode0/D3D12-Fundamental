#include <iostream>

#include "application.h"
#include "sample.h"

class triangle : public sample
{
public:
    triangle(d3d12_context* context) : sample(context){

    }

    void init() override{

    }
};

int main(int args, char* argv[])
{
    application app{500, 500};
    
    triangle tri(app.m_context);
    tri.init();

    int res = app.exec();
    return res;
}