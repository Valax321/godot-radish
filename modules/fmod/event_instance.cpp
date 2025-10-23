#include "event_instance.h"

#include "api/fmod_studio.hpp"
#include "api/fmod_errors.h"

// Docs: https://www.fmod.com/docs/2.03/api/studio-api-eventinstance.html

/*
 * TODO:
 * - Signals for playback?
 * - Signals for timeline events
 */

Ref<FMODEventInstance> FMODEventInstance::create(FMOD::Studio::EventInstance* event) {
	DEV_ASSERT(event != nullptr);

	Ref r = memnew(FMODEventInstance);
	r->init(event);
	return r;
}

FMODEventInstance::~FMODEventInstance() {
	if (event_description != nullptr) {
		event_description = nullptr;
	}

	if (event_instance != nullptr) {
		event_instance->release();
		event_instance = nullptr;
	}
}

void FMODEventInstance::init(FMOD::Studio::EventInstance *event) {
	event_instance = event;
	if (try_get_event_desc(&event_description)) {
		int32_t cnt{0};
		event_description->getParameterDescriptionCount(&cnt);
		for (auto i = 0; i < cnt; ++i) {
			FMOD_STUDIO_PARAMETER_DESCRIPTION desc{};
			if (const auto r = event_description->getParameterDescriptionByIndex(i, &desc); r == FMOD_OK) {
				event_parameters.append(desc.name);
			} else {
				WARN_PRINT(vformat("Failed to get parameter description: %s", FMOD_ErrorString(r)));
			}
		}
	}
}

void FMODEventInstance::start() {
	event_instance->start();
}

void FMODEventInstance::stop(const FMOD_STUDIO_STOP_MODE stop_mode) {
	event_instance->stop(stop_mode);
}

FMOD_STUDIO_PLAYBACK_STATE FMODEventInstance::get_playback_state() const {
	FMOD_STUDIO_PLAYBACK_STATE s{};
	if (const auto result = event_instance->getPlaybackState(&s); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get playback state: %s", FMOD_ErrorString(result)));
		return FMOD_STUDIO_PLAYBACK_STOPPED;
	}
	return s;
}

bool FMODEventInstance::get_paused() const {
	bool p{};
	if (const auto r = event_instance->getPaused(&p); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get paused state: %s", FMOD_ErrorString(r)));
		return false;
	}
	return p;
}

void FMODEventInstance::set_paused(const bool p_paused) {
	if (const auto r = event_instance->setPaused(p_paused); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to set paused state: %s", FMOD_ErrorString(r)));
	}
}

FMOD_RESULT FMODEventInstance::set_parameter(const String &name, const Variant &value, const bool ignore_seek_speed) {
	switch (value.get_type()) {
		case Variant::INT:
		case Variant::FLOAT:
			return event_instance->setParameterByName(name.utf8().get_data(), value);
		default:
			return event_instance->setParameterByNameWithLabel(name.utf8().get_data(), value.stringify().utf8().get_data(), ignore_seek_speed);
	}
}

float FMODEventInstance::get_parameter(const String &name) const {
	float v{};
	if (const auto result = event_instance->getParameterByName(name.utf8().get_data(), &v); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get parameter value: %s", FMOD_ErrorString(result)));
		return 0;
	}
	return v;
}

const Vector<String>& FMODEventInstance::get_parameters() const {
	return event_parameters;
}

Vector3 FMODEventInstance::get_position() const {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return Vector3(0, 0, 0);
	}
	return Vector3(attr.position.x, attr.position.y, attr.position.z);
}

void FMODEventInstance::set_position(Vector3 p_position) {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return;
	}

	attr.position.x = p_position.x;
	attr.position.y = p_position.y;
	attr.position.z = p_position.z;
	try_set_3d_attributes(&attr);
}

Vector3 FMODEventInstance::get_velocity() const {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return Vector3(0, 0, 0);
	}

	return Vector3(attr.velocity.x, attr.velocity.y, attr.velocity.z);
}

void FMODEventInstance::set_velocity(Vector3 p_velocity) {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return;
	}

	attr.velocity.x = p_velocity.x;
	attr.velocity.y = p_velocity.y;
	attr.velocity.z = p_velocity.z;
	try_set_3d_attributes(&attr);
}

Basis FMODEventInstance::get_rotation() const {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return Basis();
	}

	// TODO: verify math
	const Vector3 fwd{attr.forward.x, attr.forward.y, attr.forward.z};
	const Vector3 up{attr.up.x, attr.up.y, attr.up.z};

	const auto b = Basis::looking_at(fwd, up);
	return b;
}

void FMODEventInstance::set_rotation(const Basis& p_rotation) {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return;
	}

	const auto up = p_rotation.xform(Vector3(0, 1, 0));
	const auto fwd = p_rotation.xform(Vector3(0, 0, -1));

	attr.up = {up.x, up.y, up.z};
	attr.forward = {fwd.x, fwd.y, fwd.z};
}


float FMODEventInstance::get_volume() const {
	float v{};
	if (const auto result = event_instance->getVolume(&v); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get volume: %s", FMOD_ErrorString(result)));
		return 0;
	}
	return v;
}

void FMODEventInstance::set_volume(const float p_volume) {
	if (const auto result = event_instance->setVolume(p_volume); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to set volume: %s", FMOD_ErrorString(result)));
	}
}

void FMODEventInstance::set_transform_3d(const Transform3D &p_xform, Vector3 p_velocity) {
	const auto pos = p_xform.origin;
	const auto up = p_xform.basis.xform(Vector3(0, 1, 0));
	const auto fwd = p_xform.basis.xform(Vector3(0, 0, -1));

	FMOD_3D_ATTRIBUTES attr;
	attr.position = { pos.x, pos.y, pos.z };
	attr.velocity = { p_velocity.x, p_velocity.y, p_velocity.z };
	attr.up = { up.x, up.y, up.z };
	attr.forward = { fwd.x, fwd.y, fwd.z };
	try_set_3d_attributes(&attr);
}

void FMODEventInstance::set_transform_2d(const Transform2D &p_xform, const Vector2 p_velocity) {
	const auto pos = p_xform.get_origin();
	const auto up = p_xform.basis_xform(Vector2(0, 1));
	const auto fwd = p_xform.basis_xform(Vector2(0, 1));

	FMOD_3D_ATTRIBUTES attr;
	attr.position = { pos.x, pos.y, 0 };
	attr.velocity = { p_velocity.x, p_velocity.y, 0};
	attr.up = { up.x, up.y, 0};
	attr.forward = { fwd.x, fwd.y, 0 };
	try_set_3d_attributes(&attr);
}

bool FMODEventInstance::try_get_3d_attributes(FMOD_3D_ATTRIBUTES *p_attr) const {
	if (const auto result = event_instance->get3DAttributes(p_attr); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get 3D attributes: %s", FMOD_ErrorString(result)));
		return false;
	}
	return true;
}

bool FMODEventInstance::try_set_3d_attributes(const FMOD_3D_ATTRIBUTES *p_attr) {
	if (const auto result = event_instance->set3DAttributes(p_attr); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to set 3D attributes: %s", FMOD_ErrorString(result)));
		return false;
	}
	return true;
}

bool FMODEventInstance::try_get_event_desc(FMOD::Studio::EventDescription **p_desc) const {
	if (const auto result = event_instance->getDescription(p_desc); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get event description: %s", FMOD_ErrorString(result)));
		return false;
	}
	return true;
}

void FMODEventInstance::_bind_methods() {
	// Playback
	ClassDB::bind_method("start", &FMODEventInstance::start);
	ClassDB::bind_method(D_METHOD("stop", "stop_mode"), &FMODEventInstance::stop, DEFVAL(FMOD_STUDIO_STOP_ALLOWFADEOUT));

	// Parameters
	ClassDB::bind_method(D_METHOD("get_parameter", "name"), &FMODEventInstance::get_parameter);
	ClassDB::bind_method(D_METHOD("set_parameter", "name", "value", "ignore_seek_speed"), &FMODEventInstance::set_parameter, DEFVAL(false));
	ClassDB::bind_method("get_parameters", &FMODEventInstance::get_parameters);

	BIND_GETTER_SETTER_PAIR(FMODEventInstance, volume);
	BIND_GETTER_SETTER_PAIR(FMODEventInstance, position);
	BIND_GETTER_SETTER_PAIR(FMODEventInstance, velocity);
	BIND_GETTER_SETTER_PAIR(FMODEventInstance, rotation);
	BIND_GETTER_SETTER_PAIR(FMODEventInstance, paused);

	ClassDB::bind_method(D_METHOD("set_transform_3d", "xform", "velocity"), &FMODEventInstance::set_transform_3d);
	ClassDB::bind_method(D_METHOD("set_transform_2d", "xform", "velocity"), &FMODEventInstance::set_transform_2d);

#if 0
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "volume"), "get_volume", "set_volume");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "get_position", "set_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "velocity"), "get_velocity", "set_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::BASIS, "rotation"), "get_rotation", "set_rotation");
#endif
}
