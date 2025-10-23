#include "event_instance.h"

#include "api/fmod_studio.hpp"
#include "api/fmod_errors.h"

// Docs: https://www.fmod.com/docs/2.03/api/studio-api-eventinstance.html

/*
 * TODO:
 * - Signals for playback?
 * - Signals for timeline events
 */

Ref<EventInstance> EventInstance::create(FMOD::Studio::EventInstance* event) {
	DEV_ASSERT(event != nullptr);

	Ref r = memnew(EventInstance);
	r->init(event);
	return r;
}

EventInstance::~EventInstance() {
	if (event_description != nullptr) {
		event_description = nullptr;
	}

	if (event_instance != nullptr) {
		event_instance->release();
		event_instance = nullptr;
	}
}

void EventInstance::init(FMOD::Studio::EventInstance *event) {
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

void EventInstance::start() {
	event_instance->start();
}

void EventInstance::stop(const FMOD_STUDIO_STOP_MODE stop_mode) {
	event_instance->stop(stop_mode);
}

FMOD_STUDIO_PLAYBACK_STATE EventInstance::get_playback_state() const {
	FMOD_STUDIO_PLAYBACK_STATE s{};
	if (const auto result = event_instance->getPlaybackState(&s); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get playback state: %s", String(FMOD_ErrorString(result))));
		return FMOD_STUDIO_PLAYBACK_STOPPED;
	}
	return s;
}

FMOD_RESULT EventInstance::set_parameter(const String &name, const Variant &value, const bool ignore_seek_speed) {
	switch (value.get_type()) {
		case Variant::INT:
		case Variant::FLOAT:
			return event_instance->setParameterByName(name.utf8().get_data(), value);
		default:
			return event_instance->setParameterByNameWithLabel(name.utf8().get_data(), value.stringify().utf8().get_data(), ignore_seek_speed);
	}
}

float EventInstance::get_parameter(const String &name) const {
	float v{};
	if (const auto result = event_instance->getParameterByName(name.utf8().get_data(), &v); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get parameter value: %s", String(FMOD_ErrorString(result))));
		return 0;
	}
	return v;
}

const Vector<String>& EventInstance::get_parameters() const {
	return event_parameters;
}

Vector3 EventInstance::get_position() const {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return Vector3(0, 0, 0);
	}
	return Vector3(attr.position.x, attr.position.y, attr.position.z);
}

void EventInstance::set_position(Vector3 p_position) {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return;
	}

	attr.position.x = p_position.x;
	attr.position.y = p_position.y;
	attr.position.z = p_position.z;
	try_set_3d_attributes(&attr);
}

Vector3 EventInstance::get_velocity() const {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return Vector3(0, 0, 0);
	}

	return Vector3(attr.velocity.x, attr.velocity.y, attr.velocity.z);
}

void EventInstance::set_velocity(Vector3 p_velocity) {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return;
	}

	attr.velocity.x = p_velocity.x;
	attr.velocity.y = p_velocity.y;
	attr.velocity.z = p_velocity.z;
	try_set_3d_attributes(&attr);
}

Basis EventInstance::get_rotation() const {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return Quaternion();
	}

	// TODO: verify math
	const Vector3 fwd{attr.forward.x, attr.forward.y, attr.forward.z};
	const Vector3 up{attr.up.x, attr.up.y, attr.up.z};

	const auto b = Basis::looking_at(fwd, up);
	return b;
}

void EventInstance::set_rotation(const Basis& p_rotation) {
	FMOD_3D_ATTRIBUTES attr{};
	if (!try_get_3d_attributes(&attr)) {
		return;
	}

	const auto up = p_rotation.xform(Vector3(0, 1, 0));
	const auto fwd = p_rotation.xform(Vector3(0, 0, -1));

	attr.up = {up.x, up.y, up.z};
	attr.forward = {fwd.x, fwd.y, fwd.z};
}


float EventInstance::get_volume() const {
	float v{};
	if (const auto result = event_instance->getVolume(&v); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get volume: %s", String(FMOD_ErrorString(result))));
		return 0;
	}
	return v;
}

void EventInstance::set_volume(const float p_volume) {
	if (const auto result = event_instance->setVolume(p_volume); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to set volume: %s", String(FMOD_ErrorString(result))));
	}
}

bool EventInstance::try_get_3d_attributes(FMOD_3D_ATTRIBUTES *p_attr) const {
	if (const auto result = event_instance->get3DAttributes(p_attr); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get 3D attributes: %s", String(FMOD_ErrorString(result))));
		return false;
	}
	return true;
}

bool EventInstance::try_set_3d_attributes(const FMOD_3D_ATTRIBUTES *p_attr) {
	if (const auto result = event_instance->set3DAttributes(p_attr); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to set 3D attributes: %s", String(FMOD_ErrorString(result))));
		return false;
	}
	return true;
}

bool EventInstance::try_get_event_desc(FMOD::Studio::EventDescription **p_desc) const {
	if (const auto result = event_instance->getDescription(p_desc); result != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get event description: %s", String(FMOD_ErrorString(result))));
		return false;
	}
	return true;
}

#define BIND_GETTER_SETTER_PAIR(clazz, name) \
	ClassDB::bind_method("get_"#name, &clazz::get_##name); \
	ClassDB::bind_method(D_METHOD("set_"#name, #name), &clazz::set_##name); \

void EventInstance::_bind_methods() {
	// Playback
	ClassDB::bind_method("start", &EventInstance::start);
	ClassDB::bind_method(D_METHOD("stop", "stop_mode"), &EventInstance::stop, DEFVAL(FMOD_STUDIO_STOP_ALLOWFADEOUT));

	// Parameters
	ClassDB::bind_method(D_METHOD("get_parameter", "name"), &EventInstance::get_parameter);
	ClassDB::bind_method(D_METHOD("set_parameter", "name", "value", "ignore_seek_speed"), &EventInstance::set_parameter, DEFVAL(false));
	ClassDB::bind_method("get_parameters", &EventInstance::get_parameters);

	BIND_GETTER_SETTER_PAIR(EventInstance, volume);
	BIND_GETTER_SETTER_PAIR(EventInstance, position);
	BIND_GETTER_SETTER_PAIR(EventInstance, velocity);
	BIND_GETTER_SETTER_PAIR(EventInstance, rotation);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "volume"), "get_volume", "set_volume");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "get_position", "set_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "velocity"), "get_velocity", "set_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::BASIS, "rotation"), "get_rotation", "set_rotation");
}
