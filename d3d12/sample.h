#pragma once

class d3d12_context;
class sample
{
public:
    sample(d3d12_context* context) : m_context(context){

    }

    virtual~sample(){}

    virtual void init() = 0;

    d3d12_context* m_context;

};