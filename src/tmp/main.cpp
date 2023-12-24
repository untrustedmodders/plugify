#include "std_logger.h"
#include <wizard/wazard.h>

int main() {
    std::shared_ptr<wizard::IWizard> sorcerer = wizard::MakeWizard();
    if (sorcerer) {
        sorcerer->SetLogger(std::make_shared<sorcerer::StdLogger>());

        bool init = sorcerer->Initialize();
        if (!init) {
            std::cout << "!!! ERROR !!!  No feet, no sweets!" << std::endl;
        }
    }

    return 0;
}
