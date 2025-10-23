#pragma once

#include "core/object/object.h"
#include "core/variant/typed_dictionary.h"
#include "core/variant/variant.h"
#include "core/object/class_db.h"

#include "fmod_enums.h"
#include "event_instance.h"

namespace FMOD::Studio {
	class System;
	class Bank;
}

class FMODManager final : public Object {
	GDCLASS(FMODManager, Object);
public:
	static FMODManager* get_singleton();
	FMODManager();
	~FMODManager() override;

	void init();
	void hook_process_signal();
	void unhook_process_signal();

	#pragma region Banks

	bool load_bank(const StringName& p_name, bool non_blocking);
	bool unload_bank(const StringName& p_name);
	FMOD_STUDIO_LOADING_STATE get_bank_loading_state(const StringName& p_name) const;

	#pragma endregion

	#pragma region Events

	Ref<EventInstance> create_instance(const String& path_or_guid) const;

	#pragma endregion

private:
	static void _bind_methods();

	void process_frame();

	inline static FMODManager* singleton_instance;
	FMOD::Studio::System* system = nullptr;
	HashMap<StringName, FMOD::Studio::Bank*> banks{};
	bool hooked_process_frame{false};
};
