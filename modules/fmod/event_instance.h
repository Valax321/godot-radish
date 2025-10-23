#pragma once

#include "core/object/ref_counted.h"

#include "fmod_enums.h"

namespace FMOD::Studio {
	class EventInstance;
	class EventDescription;
}

#define DEFINE_GETTER_SETTER_PAIR(name, type) \
	type get_##name() const; \
	void set_##name(type p_##name);

#define DEFINE_GETTER_SETTER_PAIR_BYREF(name, type) \
type get_##name() const; \
void set_##name(const type& p_##name);

class EventInstance final : public RefCounted {
	GDCLASS(EventInstance, RefCounted);

public:
	static Ref<EventInstance> create(FMOD::Studio::EventInstance* event);
	~EventInstance() override;

	void init(FMOD::Studio::EventInstance* event);

	void start();
	void stop(FMOD_STUDIO_STOP_MODE stop_mode = FMOD_STUDIO_STOP_ALLOWFADEOUT);
	FMOD_STUDIO_PLAYBACK_STATE get_playback_state() const;

	FMOD_RESULT set_parameter(const String& name, const Variant &value, bool ignore_seek_speed = false);
	float get_parameter(const String& name) const;

	const Vector<String>& get_parameters() const;

	DEFINE_GETTER_SETTER_PAIR(position, Vector3);
	DEFINE_GETTER_SETTER_PAIR(velocity, Vector3);
	DEFINE_GETTER_SETTER_PAIR_BYREF(rotation, Basis);
	DEFINE_GETTER_SETTER_PAIR(volume, float);

private:
	static auto _bind_methods() -> void;

	Vector<String> event_parameters;

	bool try_get_3d_attributes(FMOD_3D_ATTRIBUTES* p_attr) const;
	bool try_set_3d_attributes(const FMOD_3D_ATTRIBUTES* p_attr);
	bool try_get_event_desc(FMOD::Studio::EventDescription** p_desc) const;

	FMOD::Studio::EventInstance* event_instance{nullptr};
	FMOD::Studio::EventDescription* event_description{nullptr};
};
