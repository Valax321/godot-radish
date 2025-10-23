#include "fmod_manager.h"
#include "fmod_settings.h"

#include "api/fmod_studio.hpp"
#include "api/fmod_errors.h"
#include "core/config/engine.h"
#include "core/os/os.h"
#include "scene/main/scene_tree.h"

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
	const String msg = String(message).strip_edges();
	if (flags & FMOD_DEBUG_LEVEL_ERROR) {
		_err_print_error(func, file, line, msg);
	} else if (flags & FMOD_DEBUG_LEVEL_WARNING) {
		_err_print_error(func, file, line, msg, false, ERR_HANDLER_WARNING);
	}
	return FMOD_OK;
}

FMODManager* FMODManager::get_singleton() {
	return singleton_instance;
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

	// Enable live update in editor builds but ONLY in game mode
	// Also if the user requests it via feature for a specific export build
	if ((OS::get_singleton()->has_feature("editor") && !Engine::get_singleton()->is_editor_hint())
		|| OS::get_singleton()->has_feature("live_update")) {
		flags |= FMOD_STUDIO_INIT_LIVEUPDATE;
	}

	if (!FMOD_CHECKED(system->initialize(FMODProjectSettings::get_max_channels(), flags, FMOD_INIT_3D_RIGHTHANDED, nullptr))) {
		return;
	}

	if (!Engine::get_singleton()->is_editor_hint()) {
		for (const auto& bank : FMODProjectSettings::get_autoload_banks()) {
			// We block for initial bank loads to ensure they're ready
			load_bank(bank, false);
		}
	}
}

void FMODManager::hook_process_signal() {
	SceneTree* scene_tree = SceneTree::get_singleton();
	ERR_FAIL_NULL_MSG(scene_tree, "Scene tree needed to hook main loop");
	scene_tree->connect("process_frame", callable_mp(this, &FMODManager::process_frame));
	hooked_process_frame = true;
}

void FMODManager::unhook_process_signal() {
	if (hooked_process_frame) {
		SceneTree* scene_tree = SceneTree::get_singleton();
		ERR_FAIL_NULL_MSG(scene_tree, "Scene tree needed to hook main loop");
		scene_tree->disconnect("process_frame", callable_mp(this, &FMODManager::process_frame));
		hooked_process_frame = false;
	}
}

void FMODManager::process_frame() {
	if (!FMOD_IS_VALID) {
		return;
	}
	system->update();
}

bool FMODManager::load_bank(const StringName &p_name, bool non_blocking) {
	//TODO: handle paths starting with res://
	if (!FMOD_IS_VALID) {
		return false;
	}

	// Use the bank name as the key
	if (banks.has(p_name)) {
		// Already loaded
		return true;
	}

	// Make full path to the bank file
	const String res_dir = OS::get_singleton()->get_resource_dir();
	const String banks_path = res_dir.path_join(FMODProjectSettings::get_banks_path());
	const String platform_path = banks_path.path_join(FMODProjectSettings::get_platform_name());
	const String bank_path = platform_path.path_join(vformat("%s.bank", p_name));

	FMOD::Studio::Bank* out_bank{nullptr};
	if (const FMOD_RESULT r = system->loadBankFile(bank_path.utf8().get_data(), non_blocking ? FMOD_STUDIO_LOAD_BANK_NONBLOCKING : FMOD_STUDIO_LOAD_BANK_NORMAL, &out_bank); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to load FMOD bank \"%s\": %s", bank_path, String(FMOD_ErrorString(r))));
		return false;
	}

	banks.insert(p_name, out_bank);
	return true;
}

bool FMODManager::unload_bank(const StringName &p_name) {
	if (!FMOD_IS_VALID) {
		return false;
	}

	// Use the bank name as the key
	const auto& bank_itr = banks.find(p_name);
	if (bank_itr == banks.end()) {
		WARN_PRINT(vformat("Bank named \"%s\" was not loaded", p_name));
		return false;
	}

	bool success = true;
	if (!FMOD_CHECKED(bank_itr->value->unload())) {
		success = false;
	}
	banks.remove(bank_itr);
	return success;
}

FMOD_STUDIO_LOADING_STATE FMODManager::get_bank_loading_state(const StringName &p_name) const {
	if (!FMOD_IS_VALID) {
		return FMOD_STUDIO_LOADING_STATE_ERROR;
	}

	const auto& bank_itr = banks.find(p_name);
	if (bank_itr == banks.end()) {
		return FMOD_STUDIO_LOADING_STATE_ERROR;
	}

	FMOD_STUDIO_LOADING_STATE load_state{};
	if (!FMOD_CHECKED(bank_itr->value->getLoadingState(&load_state))) {
		return FMOD_STUDIO_LOADING_STATE_ERROR;
	}

	return load_state;
}

Ref<EventInstance> FMODManager::create_instance(const String& path_or_guid) const {
	if (!FMOD_IS_VALID) {
		return nullptr;
	}

	FMOD::Studio::EventDescription* desc{};
	if (!FMOD_CHECKED(system->getEvent(path_or_guid.utf8().get_data(), &desc))) {
		return nullptr;
	}

	FMOD::Studio::EventInstance* instance{};
	if (!FMOD_CHECKED(desc->createInstance(&instance))) {
		return nullptr;
	}

	return EventInstance::create(instance);
}

void FMODManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_bank", "name", "non_blocking"), &FMODManager::load_bank, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("unload_bank", "name"), &FMODManager::unload_bank);
	ClassDB::bind_method(D_METHOD("get_bank_loading_state", "name"), &FMODManager::get_bank_loading_state);

	ClassDB::bind_method(D_METHOD("create_instance", "path"), &FMODManager::create_instance);

	#pragma region One million enum

	// Result
	BIND_ENUM_CONSTANT(FMOD_OK);
	BIND_ENUM_CONSTANT(FMOD_ERR_BADCOMMAND);
	BIND_ENUM_CONSTANT(FMOD_ERR_CHANNEL_ALLOC);
	BIND_ENUM_CONSTANT(FMOD_ERR_CHANNEL_STOLEN);
	BIND_ENUM_CONSTANT(FMOD_ERR_DMA);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_CONNECTION);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_DONTPROCESS);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_FORMAT);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_INUSE);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_NOTFOUND);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_RESERVED);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_SILENCE);
	BIND_ENUM_CONSTANT(FMOD_ERR_DSP_TYPE);
	BIND_ENUM_CONSTANT(FMOD_ERR_FILE_BAD);
	BIND_ENUM_CONSTANT(FMOD_ERR_FILE_COULDNOTSEEK);
	BIND_ENUM_CONSTANT(FMOD_ERR_FILE_DISKEJECTED);
	BIND_ENUM_CONSTANT(FMOD_ERR_FILE_EOF);
	BIND_ENUM_CONSTANT(FMOD_ERR_FILE_ENDOFDATA);
	BIND_ENUM_CONSTANT(FMOD_ERR_FILE_NOTFOUND);
	BIND_ENUM_CONSTANT(FMOD_ERR_FORMAT);
	BIND_ENUM_CONSTANT(FMOD_ERR_HEADER_MISMATCH);
	BIND_ENUM_CONSTANT(FMOD_ERR_HTTP);
	BIND_ENUM_CONSTANT(FMOD_ERR_HTTP_ACCESS);
	BIND_ENUM_CONSTANT(FMOD_ERR_HTTP_PROXY_AUTH);
	BIND_ENUM_CONSTANT(FMOD_ERR_HTTP_SERVER_ERROR);
	BIND_ENUM_CONSTANT(FMOD_ERR_HTTP_TIMEOUT);
	BIND_ENUM_CONSTANT(FMOD_ERR_INITIALIZATION);
	BIND_ENUM_CONSTANT(FMOD_ERR_INITIALIZED);
	BIND_ENUM_CONSTANT(FMOD_ERR_INTERNAL);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_FLOAT);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_HANDLE);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_PARAM);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_POSITION);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_SPEAKER);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_SYNCPOINT);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_THREAD);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_VECTOR);
	BIND_ENUM_CONSTANT(FMOD_ERR_MAXAUDIBLE);
	BIND_ENUM_CONSTANT(FMOD_ERR_MEMORY);
	BIND_ENUM_CONSTANT(FMOD_ERR_MEMORY_CANTPOINT);
	BIND_ENUM_CONSTANT(FMOD_ERR_NEEDS3D);
	BIND_ENUM_CONSTANT(FMOD_ERR_NEEDSHARDWARE);
	BIND_ENUM_CONSTANT(FMOD_ERR_NET_CONNECT);
	BIND_ENUM_CONSTANT(FMOD_ERR_NET_SOCKET_ERROR);
	BIND_ENUM_CONSTANT(FMOD_ERR_NET_URL);
	BIND_ENUM_CONSTANT(FMOD_ERR_NET_WOULD_BLOCK);
	BIND_ENUM_CONSTANT(FMOD_ERR_NOTREADY);
	BIND_ENUM_CONSTANT(FMOD_ERR_OUTPUT_ALLOCATED);
	BIND_ENUM_CONSTANT(FMOD_ERR_OUTPUT_CREATEBUFFER);
	BIND_ENUM_CONSTANT(FMOD_ERR_OUTPUT_DRIVERCALL);
	BIND_ENUM_CONSTANT(FMOD_ERR_OUTPUT_FORMAT);
	BIND_ENUM_CONSTANT(FMOD_ERR_OUTPUT_INIT);
	BIND_ENUM_CONSTANT(FMOD_ERR_OUTPUT_NODRIVERS);
	BIND_ENUM_CONSTANT(FMOD_ERR_PLUGIN);
	BIND_ENUM_CONSTANT(FMOD_ERR_PLUGIN_MISSING);
	BIND_ENUM_CONSTANT(FMOD_ERR_PLUGIN_RESOURCE);
	BIND_ENUM_CONSTANT(FMOD_ERR_PLUGIN_VERSION);
	BIND_ENUM_CONSTANT(FMOD_ERR_RECORD);
	BIND_ENUM_CONSTANT(FMOD_ERR_REVERB_CHANNELGROUP);
	BIND_ENUM_CONSTANT(FMOD_ERR_REVERB_INSTANCE);
	BIND_ENUM_CONSTANT(FMOD_ERR_SUBSOUNDS);
	BIND_ENUM_CONSTANT(FMOD_ERR_SUBSOUND_ALLOCATED);
	BIND_ENUM_CONSTANT(FMOD_ERR_SUBSOUND_CANTMOVE);
	BIND_ENUM_CONSTANT(FMOD_ERR_TAGNOTFOUND);
	BIND_ENUM_CONSTANT(FMOD_ERR_TOOMANYCHANNELS);
	BIND_ENUM_CONSTANT(FMOD_ERR_TRUNCATED);
	BIND_ENUM_CONSTANT(FMOD_ERR_UNIMPLEMENTED);
	BIND_ENUM_CONSTANT(FMOD_ERR_UNINITIALIZED);
	BIND_ENUM_CONSTANT(FMOD_ERR_UNSUPPORTED);
	BIND_ENUM_CONSTANT(FMOD_ERR_VERSION);
	BIND_ENUM_CONSTANT(FMOD_ERR_EVENT_ALREADY_LOADED);
	BIND_ENUM_CONSTANT(FMOD_ERR_EVENT_LIVEUPDATE_BUSY);
	BIND_ENUM_CONSTANT(FMOD_ERR_EVENT_LIVEUPDATE_MISMATCH);
	BIND_ENUM_CONSTANT(FMOD_ERR_EVENT_LIVEUPDATE_TIMEOUT);
	BIND_ENUM_CONSTANT(FMOD_ERR_EVENT_NOTFOUND);
	BIND_ENUM_CONSTANT(FMOD_ERR_STUDIO_UNINITIALIZED);
	BIND_ENUM_CONSTANT(FMOD_ERR_STUDIO_NOT_LOADED);
	BIND_ENUM_CONSTANT(FMOD_ERR_INVALID_STRING);
	BIND_ENUM_CONSTANT(FMOD_ERR_ALREADY_LOCKED);
	BIND_ENUM_CONSTANT(FMOD_ERR_NOT_LOCKED);
	BIND_ENUM_CONSTANT(FMOD_ERR_RECORD_DISCONNECTED);
	BIND_ENUM_CONSTANT(FMOD_ERR_TOOMANYSAMPLES);

	// Loading state
	BIND_ENUM_CONSTANT(FMOD_STUDIO_LOADING_STATE_UNLOADING);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_LOADING_STATE_UNLOADED);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_LOADING_STATE_LOADING);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_LOADING_STATE_LOADED);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_LOADING_STATE_ERROR);

	// Parameter types
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_GAME_CONTROLLED);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_DISTANCE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_EVENT_CONE_ANGLE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_EVENT_ORIENTATION);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_DIRECTION);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_ELEVATION);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_LISTENER_ORIENTATION);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_SPEED);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_SPEED_ABSOLUTE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PARAMETER_AUTOMATIC_DISTANCE_NORMALIZED);

	// User property types
	BIND_ENUM_CONSTANT(FMOD_STUDIO_USER_PROPERTY_TYPE_INTEGER);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_USER_PROPERTY_TYPE_BOOLEAN);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_USER_PROPERTY_TYPE_FLOAT);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_USER_PROPERTY_TYPE_STRING);

	// Event properties
	BIND_ENUM_CONSTANT(FMOD_STUDIO_EVENT_PROPERTY_CHANNELPRIORITY);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_DELAY);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_LOOKAHEAD);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_EVENT_PROPERTY_MINIMUM_DISTANCE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_EVENT_PROPERTY_MAXIMUM_DISTANCE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_EVENT_PROPERTY_COOLDOWN);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_EVENT_PROPERTY_MAX);

	// Playback state
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PLAYBACK_PLAYING);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PLAYBACK_SUSTAINING);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PLAYBACK_STOPPED);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PLAYBACK_STARTING);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_PLAYBACK_STOPPING);

	// Stop mode
	BIND_ENUM_CONSTANT(FMOD_STUDIO_STOP_ALLOWFADEOUT);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_STOP_IMMEDIATE);

	// Instance type
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_NONE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_SYSTEM);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_EVENTDESCRIPTION);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_EVENTINSTANCE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_PARAMETERINSTANCE);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_BUS);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_VCA);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_BANK);
	BIND_ENUM_CONSTANT(FMOD_STUDIO_INSTANCETYPE_COMMANDREPLAY);

	#pragma endregion
}
