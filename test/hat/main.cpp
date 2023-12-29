#include "std_logger.h"
#include <wizard/wizard.h>

int main() {
    std::shared_ptr<wizard::IWizard> sorcerer = wizard::MakeWizard();
    if (sorcerer) {
        auto logger = std::make_shared<sorcerer::StdLogger>();
        logger->SetSeverity(wizard::Severity::Debug);
        sorcerer->SetLogger(std::move(logger));

        bool init = sorcerer->Initialize();
        if (!init) {
            WZ_LOG_ERROR("No feet, no sweets!");
        }
    }

    return 0;
}
