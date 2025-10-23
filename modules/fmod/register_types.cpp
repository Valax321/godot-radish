// Copyright (c) Radish Games. All rights reserved.

#include "register_types.h"
#include "core/object/class_db.h"
#include "core/config/engine.h"

#include "fmod_manager.h"
#include "fmod_settings.h"
#include "event_instance.h"
#include "listener.h"

#define FMOD_SINGLETON_NAME "FMOD"

static FMODManager* fmod_instance = nullptr;

void initialize_fmod_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		GDREGISTER_CLASS(FMODManager);
		GDREGISTER_CLASS(FMODEventInstance);
		GDREGISTER_CLASS(FMODListener3D);
		GDREGISTER_CLASS(FMODListener2D);

		fmod_instance = memnew(FMODManager);
		Engine::get_singleton()->add_singleton(Engine::Singleton(FMOD_SINGLETON_NAME, fmod_instance));
		FMODProjectSettings::register_settings();

		fmod_instance->init();
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		FMODManager::get_singleton()->hook_process_signal();
	}
}

void uninitialize_fmod_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_CORE) {
		Engine::get_singleton()->remove_singleton(FMOD_SINGLETON_NAME);
		memdelete(fmod_instance);
		fmod_instance = nullptr;
	} else if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		FMODManager::get_singleton()->unhook_process_signal();
	}
}
