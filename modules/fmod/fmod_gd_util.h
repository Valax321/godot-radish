#pragma once

#define DEFINE_GETTER_SETTER_PAIR(name, type) \
	type get_##name() const; \
	void set_##name(type p_##name);

#define DEFINE_GETTER_SETTER_PAIR_BYREF(name, type) \
	type get_##name() const; \
	void set_##name(const type& p_##name);

#define BIND_GETTER_SETTER_PAIR(clazz, name) \
	ClassDB::bind_method("get_"#name, &clazz::get_##name); \
	ClassDB::bind_method(D_METHOD("set_"#name, #name), &clazz::set_##name); \
