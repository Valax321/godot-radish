#pragma once

#include "core/object/ref_counted.h"

#include "fmod_enums.h"
#include "fmod_gd_util.h"

namespace FMOD::Studio {
	class EventInstance;
	class EventDescription;
}

class FMODEventInstance final : public RefCounted {
	GDCLASS(FMODEventInstance, RefCounted);

public:
	static Ref<FMODEventInstance> create(FMOD::Studio::EventInstance* event);
	~FMODEventInstance() override;

	void init(FMOD::Studio::EventInstance* event);

	void start();
	void stop(FMOD_STUDIO_STOP_MODE stop_mode = FMOD_STUDIO_STOP_ALLOWFADEOUT);
	FMOD_STUDIO_PLAYBACK_STATE get_playback_state() const;
	DEFINE_GETTER_SETTER_PAIR(paused, bool);
	DEFINE_GETTER_SETTER_PAIR(volume, float);

	FMOD_RESULT set_parameter(const String& name, const Variant &value, bool ignore_seek_speed = false);
	float get_parameter(const String& name) const;

	const Vector<String>& get_parameters() const;

	DEFINE_GETTER_SETTER_PAIR(position, Vector3);
	DEFINE_GETTER_SETTER_PAIR(velocity, Vector3);
	DEFINE_GETTER_SETTER_PAIR_BYREF(rotation, Basis);
	void set_transform_3d(const Transform3D& p_xform, Vector3 p_velocity);
	void set_transform_2d(const Transform2D& p_xform, Vector2 p_velocity);

private:
	static auto _bind_methods() -> void;

	Vector<String> event_parameters;

	bool try_get_3d_attributes(FMOD_3D_ATTRIBUTES* p_attr) const;
	bool try_set_3d_attributes(const FMOD_3D_ATTRIBUTES* p_attr);
	bool try_get_event_desc(FMOD::Studio::EventDescription** p_desc) const;

	FMOD::Studio::EventInstance* event_instance{nullptr};
	FMOD::Studio::EventDescription* event_description{nullptr};
};
