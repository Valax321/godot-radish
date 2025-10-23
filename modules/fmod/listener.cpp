#include "listener.h"
#include "fmod_manager.h"

#include "api/fmod_studio.hpp"
#include "api/fmod_errors.h"

#pragma region 3D Listener

FMODListener3D::FMODListener3D() {
	set_notify_transform(true);
}

FMODListener3D::~FMODListener3D() {
	auto* system = FMODManager::get_singleton()->get_system();
	if (system == nullptr) {
		return;
	}

	// Remove this listener from weighting contribution.
	system->setListenerWeight(id, 0);
}

int32_t FMODListener3D::get_id() const {
	return id;
}

void FMODListener3D::set_id(const int32_t p_id) {
	id = p_id;
}

float FMODListener3D::get_weight() const {
	auto* system = FMODManager::get_singleton()->get_system();
	if (system == nullptr) {
		return 0;
	}

	float w{};
	if (const auto r = system->getListenerWeight(id, &w); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get listener weight: %s", FMOD_ErrorString(r)));
		return 0;
	}
	return w;
}

void FMODListener3D::set_weight(float p_weight) {
	auto* system = FMODManager::get_singleton()->get_system();
	ERR_FAIL_NULL(system);

	if (const auto r = system->setListenerWeight(id, CLAMP(p_weight, 0.0f, 1.0f)); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to set listener weight: %s", FMOD_ErrorString(r)));
	}
}

void FMODListener3D::_notification(int p_what) {
	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		auto xform = get_transform();
		//TODO: calculate velocity somehow...
		set_transform_3d(xform, Vector3(), xform.origin);
	}
}

void FMODListener3D::set_transform_3d(const Transform3D& p_xform, Vector3 velocity, Vector3 p_attenuation_position) {

	FMOD_3D_ATTRIBUTES attr;
	attr.position = {p_xform.origin.x, p_xform.origin.y, p_xform.origin.z};
	const auto up = p_xform.basis.xform(Vector3(0, 1, 0));
	const auto fwd = p_xform.basis.xform(Vector3(0, 0, -1));
	attr.forward = { fwd.x, fwd.y, fwd.z };
	attr.up = { up.x, up.y, up.z };
	attr.velocity = { velocity.x, velocity.y, velocity.z };

	const FMOD_VECTOR attn{ p_attenuation_position.x, p_attenuation_position.y, p_attenuation_position.z };
	try_set_3d_attributes(&attr, &attn);
}

bool FMODListener3D::try_set_3d_attributes(const FMOD_3D_ATTRIBUTES *attr, const FMOD_VECTOR* attenuation_position) {
	auto* system = FMODManager::get_singleton()->get_system();
	if (system == nullptr) {
		return false;
	}

	if (const auto r = system->setListenerAttributes(id, attr, attenuation_position); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get listener attributes: %s", FMOD_ErrorString(r)));
		return false;
	}
	return true;
}


void FMODListener3D::_bind_methods() {
	BIND_GETTER_SETTER_PAIR(FMODListener3D, weight);
	BIND_GETTER_SETTER_PAIR(FMODListener3D, id);

	ADD_GROUP("Listener", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "id", PROPERTY_HINT_RANGE, vformat("0,%d", FMOD_MAX_LISTENERS - 1)), "get_id", "set_id");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "weight", PROPERTY_HINT_RANGE, "0,1"), "get_weight", "set_weight");
}

#pragma endregion

#pragma region 2D Listener

FMODListener2D::FMODListener2D() {
	set_notify_transform(true);
}

FMODListener2D::~FMODListener2D() {
	auto* system = FMODManager::get_singleton()->get_system();
	if (system == nullptr) {
		return;
	}

	// Remove this listener from weighting contribution.
	system->setListenerWeight(id, 0);
}

void FMODListener2D::_notification(int p_what) {
	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		auto xform = get_transform();
		//TODO: calculate velocity somehow...
		set_transform_2d(xform, Vector2(), xform.get_origin());
	}
}

int32_t FMODListener2D::get_id() const {
	return id;
}

void FMODListener2D::set_id(const int32_t p_id) {
	id = p_id;
}

float FMODListener2D::get_weight() const {
	auto* system = FMODManager::get_singleton()->get_system();
	if (system == nullptr) {
		return 0;
	}

	float w{};
	if (const auto r = system->getListenerWeight(id, &w); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get listener weight: %s", FMOD_ErrorString(r)));
		return 0;
	}
	return w;
}

void FMODListener2D::set_weight(float p_weight) {
	auto* system = FMODManager::get_singleton()->get_system();
	ERR_FAIL_NULL(system);

	if (const auto r = system->setListenerWeight(id, CLAMP(p_weight, 0.0f, 1.0f)); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to set listener weight: %s", FMOD_ErrorString(r)));
	}
}

void FMODListener2D::set_transform_2d(const Transform2D &p_xform, Vector2 velocity, Vector2 p_attenuation_position) {
	FMOD_3D_ATTRIBUTES attr;

	const auto pos = p_xform.get_origin();
	const auto fwd = p_xform.basis_xform(Vector2(1, 0));
	const auto up = p_xform.basis_xform(Vector2(0, 1));

	attr.position = { pos.x, pos.y, 0 };
	attr.velocity = { velocity.x, velocity.y, 0 };
	attr.forward = { fwd.x, fwd.y, 0 };
	attr.up = { up.x, up.y, 0 };
	const FMOD_VECTOR attn { p_attenuation_position.x, p_attenuation_position.y, 0 };

	try_set_3d_attributes(&attr, &attn);
}

bool FMODListener2D::try_set_3d_attributes(const FMOD_3D_ATTRIBUTES *attr, const FMOD_VECTOR *attenuation_position) {
	auto* system = FMODManager::get_singleton()->get_system();
	if (system == nullptr) {
		return false;
	}

	if (const auto r = system->setListenerAttributes(id, attr, attenuation_position); r != FMOD_OK) {
		WARN_PRINT(vformat("Failed to get listener attributes: %s", FMOD_ErrorString(r)));
		return false;
	}
	return true;
}

void FMODListener2D::_bind_methods() {
	BIND_GETTER_SETTER_PAIR(FMODListener2D, id);
	BIND_GETTER_SETTER_PAIR(FMODListener2D, weight);

	ADD_GROUP("Listener", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "id", PROPERTY_HINT_RANGE, vformat("0,%d", FMOD_MAX_LISTENERS - 1)), "get_id", "set_id");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "weight", PROPERTY_HINT_RANGE, "0,1"), "get_weight", "set_weight");
}

#pragma endregion
