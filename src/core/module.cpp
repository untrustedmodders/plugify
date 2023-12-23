#include "module.h"

using namespace wizard;

Module::Module(fs::path filePath, LanguageModuleDescriptor descriptor) : m_filePath{std::move(filePath)}, m_descriptor{std::move(descriptor)} {

}
