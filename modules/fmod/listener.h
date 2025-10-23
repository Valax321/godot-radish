#pragma once

#include "scene/3d/node_3d.h"

#include "fmod_gd_util.h"
#include "api/fmod_common.h"
#include "scene/2d/node_2d.h"

struct FMOD_3D_ATTRIBUTES;

class FMODListener3D : public Node3D {
	GDCLASS(FMODListener3D, Node3D);
public:
	FMODListener3D();
	~FMODListener3D() override;

	DEFINE_GETTER_SETTER_PAIR(id, int32_t);
	DEFINE_GETTER_SETTER_PAIR(weight, float);

private:
	int32_t id{};

	void _notification(int p_what);

	void set_transform_3d(const Transform3D& p_xform, Vector3 velocity, Vector3 p_attenuation_position);
	bool try_set_3d_attributes(const FMOD_3D_ATTRIBUTES* attr, const FMOD_VECTOR* attenuation_position = nullptr);

	static void _bind_methods();
};

class FMODListener2D : public Node2D {
	GDCLASS(FMODListener2D, Node2D);
public:
	FMODListener2D();
	~FMODListener2D() override;

	DEFINE_GETTER_SETTER_PAIR(id, int32_t);
	DEFINE_GETTER_SETTER_PAIR(weight, float);

private:
	int32_t id{};

	void _notification(int p_what);

	void set_transform_2d(const Transform2D& p_xform, Vector2 velocity, Vector2 p_attenuation_position);
	bool try_set_3d_attributes(const FMOD_3D_ATTRIBUTES* attr, const FMOD_VECTOR* attenuation_position = nullptr);

	static void _bind_methods();
};
