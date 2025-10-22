#pragma once

#include "core/object/object.h"
#include "core/os/thread.h"
#include "core/os/mutex.h"
#include "core/templates/list.h"
#include "core/variant/typed_dictionary.h"
#include "core/variant/variant.h"
#include "core/object/class_db.h"

namespace FMOD::Studio {
	class System;
	class Bank;
}

class FMODManager final : public Object {
	GDCLASS(FMODManager, Object);
public:
	static FMODManager* get_singleton() { return singleton_instance; }
	FMODManager();
	~FMODManager() override;

	void init();

	bool load_bank(const String &p_path);

protected:
	static void _bind_methods();

	FMOD::Studio::System* system = nullptr;

private:
	inline static FMODManager* singleton_instance;
	HashMap<String, FMOD::Studio::Bank*> banks{};
};
