module;

#include "plugify/alias.hpp"
#include "plugify/assembly.hpp"
#include "plugify/assembly_loader.hpp"
#include "plugify/binding.hpp"
#include "plugify/call.hpp"
#include "plugify/callback.hpp"
#include "plugify/class.hpp"
#include "plugify/config.hpp"
#include "plugify/conflict.hpp"
#include "plugify/dependency.hpp"
#include "plugify/dependency_resolver.hpp"
#include "plugify/enum_object.hpp"
#include "plugify/enum_value.hpp"
#include "plugify/event_bus.hpp"
#include "plugify/extension.hpp"
#include "plugify/file_system.hpp"
#include "plugify/global.h"
#include "plugify/language_module.hpp"
#include "plugify/lifecycle.hpp"
#include "plugify/load_flag.hpp"
#include "plugify/logger.hpp"
#include "plugify/manager.hpp"
#include "plugify/manifest.hpp"
#include "plugify/manifest_parser.hpp"
#include "plugify/mem_addr.hpp"
#include "plugify/method.hpp"
#include "plugify/platform_ops.hpp"
#include "plugify/plugify.hpp"
#include "plugify/property.hpp"
#include "plugify/provider.hpp"
#include "plugify/registrar.hpp"
#include "plugify/service_locator.hpp"
#include "plugify/signarure.hpp"
#include "plugify/types.hpp"
#include "plugify/value_type.hpp"

export module plugify;

export namespace plugify {
	using namespace plugify;
} // namespace plugify