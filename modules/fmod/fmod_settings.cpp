#include "fmod_settings.h"

void FMODProjectSettings::register_settings() {
	GLOBAL_DEF_RST(PropertyInfo(
		Variant::ARRAY,
		"fmod/project/autoload_banks",
		PROPERTY_HINT_ARRAY_TYPE, vformat("%d/%d:*.bank", Variant::STRING, PROPERTY_HINT_FILE)
		), Array());

	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "fmod/runtime/max_channels", PROPERTY_HINT_RANGE, U"1,128"), 32);
}

Vector<String> FMODProjectSettings::get_autoload_banks() {
	return GLOBAL_GET("fmod/project/autoload_banks");
}

int32_t FMODProjectSettings::get_max_channels() {
	return GLOBAL_GET("fmod/runtime/max_channels");
}
