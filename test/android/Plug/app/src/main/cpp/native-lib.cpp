#include <jni.h>
#include <string>

#include "logger.h"

#include <plugify/compat_format.h>
#include <plugify/plugify.h>
#include <plugify/plugin.h>
#include <plugify/module.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin_reference_descriptor.h>
#include <plugify/language_module_descriptor.h>
#include <plugify/package.h>
#include <plugify/plugin_manager.h>
#include <plugify/package_manager.h>

std::shared_ptr<plug::AndroidLogger> s_logger;
std::shared_ptr<plugify::IPlugify> s_context;

extern "C" JNIEXPORT void JNICALL
Java_com_example_plug_MainActivity_makePlugify(JNIEnv* env, jobject /* this */, jstring path) {
    const char * res;

    jboolean isCopy;
    res = env->GetStringUTFChars(path, &isCopy);

	s_context = plugify::MakePlugify();

	s_logger = std::make_shared<plug::AndroidLogger>();
	s_logger->SetSeverity(plugify::Severity::Info);
	s_context->SetLogger(s_logger);

	std::filesystem::path rootDir(res);

    if (isCopy == JNI_TRUE) {
        (env)->ReleaseStringUTFChars(path, res);
    }

    auto result = s_context->Initialize(rootDir);
	if (result)
	{
		s_logger->SetSeverity(s_context->GetConfig().logSeverity);

		if (auto packageManager = s_context->GetPackageManager().lock())
		{
			packageManager->Initialize();

			if (packageManager->HasMissedPackages())
			{
				__android_log_print(ANDROID_LOG_ERROR, "Plugify", "Plugin manager has missing packages, run 'update --missing' to resolve issues.");
				return;
			}
			if (packageManager->HasConflictedPackages())
			{
				__android_log_print(ANDROID_LOG_ERROR, "Plugify", "Plugin manager has conflicted packages, run 'remove --conflict' to resolve issues.");
				return;
			}
		}

		if (auto pluginManager = s_context->GetPluginManager().lock())
		{
			pluginManager->Initialize();
		}
	}
}