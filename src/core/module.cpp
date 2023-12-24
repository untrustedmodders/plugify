#include "module.h"

using namespace wizard;

Module::Module(fs::path filePath, LanguageModuleDescriptor descriptor) : _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {

}

bool Module::Initialize() {
    assert(IsInitialized());

    if (!fs::exists(_filePath) || !fs::is_regular_file(_filePath)) {
        WIZARD_LOG("Module binary '" + _filePath.string() + "' not exist!.", ErrorLevel::ERROR);
        return false;
    }

    _library = std::make_unique<Library>(_filePath);
    if (_library) {
        void* GetLanguageModulePtr = _library->GetFunction("GetLanguageModule");
        if (!GetLanguageModulePtr) {
            WIZARD_LOG("Function 'GetLanguageModule' not exist inside '" + _filePath.string() + "' library", ErrorLevel::ERROR);
            Terminate();
            return false;
        }

        using GetLanguageModuleFuncT = ILanguageModule*(*)();
        auto GetLanguageModuleFunc = reinterpret_cast<GetLanguageModuleFuncT>(GetLanguageModulePtr);
        ILanguageModule* languageModulePtr = GetLanguageModuleFunc();

        if (!languageModulePtr) {
            WIZARD_LOG("Function 'GetLanguageModule' inside '" + _filePath.string() + "' library. Not returned valid address of 'ILanguageModule' implementation!", ErrorLevel::ERROR);
            Terminate();
            return false;
        }

        if (languageModulePtr->Initialize()) {
            _languageModule = std::make_optional(std::ref(*languageModulePtr));
            return true;
        } else {
            Terminate();
        }
    }

    WIZARD_LOG("Failed to initialize module: '" + _name + "' at: '" + _filePath.string() + "'", ErrorLevel::ERROR);
    return false;
}

void Module::Terminate() {
    if (_languageModule.has_value()) {
        GetLanguageModule().Shutdown();
    }
    _languageModule.reset();
    _library.reset();
}
