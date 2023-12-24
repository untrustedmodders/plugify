#include "plugin.h"

using namespace wizard;

Plugin::Plugin(uint64_t id, std::string name, fs::path filePath, PluginDescriptor descriptor) : m_id{id}, m_name{std::move(name)}, m_filePath{std::move(filePath)}, m_descriptor{std::move(descriptor)} {

}