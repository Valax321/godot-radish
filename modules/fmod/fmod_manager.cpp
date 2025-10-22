#include "fmod_manager.h"
#include "fmod_settings.h"

#include "api/fmod_studio.hpp"
#include "api/fmod_errors.h"
#include "core/config/engine.h"
#include "core/os/os.h"

#define FMOD_SUCCESS(c) (check_result<false>((c), #c))
#define FMOD_CHECKED(c) (check_result<true>((c), #c))
#define FMOD_IS_VALID (this->system != nullptr && this->system->isValid())

template<bool ShouldPrint>
static bool check_result(const FMOD_RESULT result, const char* expr) {
	if (result != FMOD_OK) {
		if constexpr (ShouldPrint) {
			ERR_PRINT(vformat("FMOD error: %s, %s", String(FMOD_ErrorString(result)), String(expr)));
		}
		return false;
	}
	return true;
}

F_CALL static FMOD_RESULT fmod_log_callback(FMOD_DEBUG_FLAGS flags, const char *file, int line, const char* func, const char* message) {
	String msg = String(message).strip_edges();
	if (flags & FMOD_DEBUG_LEVEL_ERROR) {
		_err_print_error(func, file, line, msg);
	} else if (flags & FMOD_DEBUG_LEVEL_WARNING) {
		_err_print_error(func, file, line, msg, false, ERR_HANDLER_WARNING);
	}
	return FMOD_OK;
}

FMODManager::FMODManager() {
	singleton_instance = this;
}

FMODManager::~FMODManager() {
	for (const auto& kv : banks) {
			kv.value->unload();
	}

	if (FMOD_IS_VALID) {
		system->release();
		system = nullptr;
	}

	singleton_instance = nullptr;
}

void FMODManager::init() {
	FMOD::Debug_Initialize(FMOD_DEBUG_LEVEL_WARNING | FMOD_DEBUG_LEVEL_ERROR, FMOD_DEBUG_MODE_CALLBACK, &fmod_log_callback);

	if (!FMOD_CHECKED(FMOD::Studio::System::create(&system))) {
		return;
	}

	FMOD_STUDIO_INITFLAGS flags = FMOD_STUDIO_INIT_NORMAL;
	if (OS::get_singleton()->has_feature("editor") || OS::get_singleton()->has_feature("live_update")) {
		flags |= FMOD_STUDIO_INIT_LIVEUPDATE;
	}

	if (!FMOD_CHECKED(system->initialize(FMODProjectSettings::get_max_channels(), flags, FMOD_INIT_3D_RIGHTHANDED, nullptr))) {
		return;
	}

	if (!Engine::get_singleton()->is_editor_hint()) {
		for (const auto& bank : FMODProjectSettings::get_autoload_banks()) {
			load_bank(bank);
		}
	}
}

bool FMODManager::load_bank(const String &p_path) {
	//TODO: handle paths starting with res://
	if (!FMOD_IS_VALID) {
		return false;
	}

	// Use the bank name as the key
	const String file_name = p_path.get_basename();
	if (banks.has(file_name)) {
		// Already loaded
		return true;
	}

	// Make full path to the bank file
	const String res_dir = OS::get_singleton()->get_resource_dir();
	const String bank_path = res_dir.path_join(p_path);

	FMOD::Studio::Bank* out_bank{nullptr};
	if (const FMOD_RESULT r = system->loadBankFile(bank_path.utf8().get_data(), FMOD_STUDIO_LOAD_BANK_NORMAL, &out_bank); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to load FMOD bank \"%s\": %s", bank_path, String(FMOD_ErrorString(r))));
		return false;
	}

	banks.insert(file_name, out_bank);
	return true;
}

void FMODManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_bank", "path"), &FMODManager::load_bank);
}
