#include "module.h"
#include "library.h"

using namespace wizard;

Module::Module(fs::path filePath, LanguageModuleDescriptor descriptor) : m_filePath{std::move(filePath)}, m_descriptor{std::move(descriptor)} {

}

bool Module::Initialize() {
    assert(IsInitialized());

    if (!fs::exists(m_filePath) || !fs::is_regular_file(m_filePath)) {
        WIZARD_LOG("Module binary '" + m_filePath.string() + "' not exist!.", ErrorLevel::ERROR);
        return false;
    }

    library = std::make_unique<Library>(m_filePath);
    if (library) {
        void* GetLanguageModulePtr = library->GetFunction("GetLanguageModule");
        if (!GetLanguageModulePtr) {
            WIZARD_LOG("Function 'GetLanguageModule' not exist inside '" + m_filePath.string() + "' library", ErrorLevel::ERROR);
            Shutdown();
            return false;
        }

        ILanguageModule* (*GetLanguageModuleFunc)() = reinterpret_cast<ILanguageModule* (*)()>(GetLanguageModulePtr);
        ILanguageModule* languageModulePtr = GetLanguageModuleFunc();

        if (!languageModulePtr) {
            WIZARD_LOG("Function 'GetLanguageModule' inside '" + m_filePath.string() + "' library. Not returned valid address of 'ILanguageModule' implementation!", ErrorLevel::ERROR);
            Shutdown();
            return false;
        }

        if (languageModulePtr->Initialize()) {
            languageModule = std::make_optional(std::ref(*languageModulePtr));
            return true;
        } else {
            Shutdown();
        }
    }

    WIZARD_LOG("Failed to initialize module: '" + m_name + "' at: '" + m_filePath.string() + "'", ErrorLevel::ERROR);
    return false;
}

void Module::Shutdown() {
    if (languageModule.has_value()) {
        GetLanguageModule().Shutdown();
    }
    languageModule.reset();
    library.reset();
}
