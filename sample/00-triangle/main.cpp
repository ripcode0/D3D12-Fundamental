#include <iostream>

#include "application.h"

int main(int args, char* argv[])
{
    application app{500, 500};

    int res = app.exec();
    return res;
}