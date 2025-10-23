#include "fmod_settings.h"

void FMODProjectSettings::register_settings() {
	GLOBAL_DEF_RST(PropertyInfo(Variant::ARRAY,"fmod/project/autoload_banks",PROPERTY_HINT_ARRAY_TYPE, U"StringName"), Array());
	GLOBAL_DEF_RST(PropertyInfo(Variant::ARRAY, "fmod/project/locales", PROPERTY_HINT_ARRAY_TYPE, U"StringName"), Array());
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "fmod/runtime/max_channels", PROPERTY_HINT_RANGE, U"1,128"), 32);
	GLOBAL_DEF_RST(PropertyInfo(Variant::STRING, "fmod/runtime/banks_directory"), U"banks");
	GLOBAL_DEF_RST(PropertyInfo(Variant::STRING_NAME, "fmod/runtime/banks_platform_name"), U"pc");
}

Vector<StringName> FMODProjectSettings::get_autoload_banks() {
	return GLOBAL_GET("fmod/project/autoload_banks");
}

Vector<StringName> FMODProjectSettings::get_locales() {
	return GLOBAL_GET("fmod/project/locales");
}

int32_t FMODProjectSettings::get_max_channels() {
	return GLOBAL_GET("fmod/runtime/max_channels");
}

String FMODProjectSettings::get_banks_path() {
	return GLOBAL_GET("fmod/runtime/banks_directory");
}

StringName FMODProjectSettings::get_platform_name() {
	return GLOBAL_GET("fmod/runtime/banks_platform_name");
}
