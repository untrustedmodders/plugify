#include "module.h"

using namespace wizard;

Module::Module(fs::path filePath, LanguageModuleDescriptor descriptor) : _filePath{std::move(filePath)}, _descriptor{std::move(descriptor)} {

}

Module::~Module() {
    Terminate();
}

bool Module::Initialize() {
    // TODO: assert(IsInitialized());

    if (!fs::exists(_filePath) || !fs::is_regular_file(_filePath)) {
        SetError("Module binary '" + _filePath.string() + "' not exist!.");
        return false;
    }

    _library = std::make_unique<Library>(_filePath);
    if (_library) {
        void* GetLanguageModulePtr = _library->GetFunction("GetLanguageModule");
        if (!GetLanguageModulePtr) {
            SetError("Function 'GetLanguageModule' not exist inside '" + _filePath.string() + "' library");
            Terminate();
            return false;
        }

        using GetLanguageModuleFuncT = ILanguageModule*(*)();
        auto GetLanguageModuleFunc = reinterpret_cast<GetLanguageModuleFuncT>(GetLanguageModulePtr);
        ILanguageModule* languageModulePtr = GetLanguageModuleFunc();

        if (!languageModulePtr) {
            SetError("Function 'GetLanguageModule' inside '" + _filePath.string() + "' library. Not returned valid address of 'ILanguageModule' implementation!");
            Terminate();
            return false;
        }

        if (languageModulePtr->Initialize()) {
            _languageModule = std::make_optional(std::ref(*languageModulePtr));
            SetLoaded();
            return true;
        } else {
            Terminate();
        }
    }

    SetError("Failed to initialize module: '" + _name + "' at: '" + _filePath.string() + "'");
    return false;
}

void Module::Terminate() {
    if (_languageModule.has_value()) {
        GetLanguageModule().Shutdown();
    }
    _languageModule.reset();
    _library.reset();
}
