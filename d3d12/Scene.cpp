#include "Scene.h"
#include "DX12Context.h"

DX12Scene::DX12Scene(DX12Context *context)
    : m_context(context)
{
    m_device = context->get_device();
    m_command_list = context->get_command_list();
}