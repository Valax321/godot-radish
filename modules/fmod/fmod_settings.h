#pragma once

#include "core/config/project_settings.h"
#include "core/variant/typed_array.h"

class FMODProjectSettings {
public:
	static void register_settings();

	static Vector<StringName> get_autoload_banks();
	static Vector<StringName> get_locales();
	static int32_t get_max_channels();
	static String get_banks_path();
	static StringName get_platform_name();
};
