#pragma once

#include "core/config/project_settings.h"
#include "core/variant/typed_array.h"

class FMODProjectSettings {
public:
	static void register_settings();

	static Vector<String> get_autoload_banks();
	static int32_t get_max_channels();
};
