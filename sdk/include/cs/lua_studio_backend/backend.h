////////////////////////////////////////////////////////////////////////////
//	Module 		: backend.h
//	Created 	: 14.04.2008
//  Modified 	: 14.04.2008
//	Author		: Dmitriy Iassenev
//	Description : script debugger backend class
////////////////////////////////////////////////////////////////////////////

#ifndef CS_LUA_STUDIO_BACKEND_H_INCLUDED
#define CS_LUA_STUDIO_BACKEND_H_INCLUDED

struct lua_State;
struct lua_Debug;

namespace cs {
namespace lua_studio {

enum icon_type {
#if 0
	icon_type_nil				= static_cast<unsigned int>(0),
	icon_type_boolean			= static_cast<unsigned int>(0),
	icon_type_number			= static_cast<unsigned int>(0),
	icon_type_string			= static_cast<unsigned int>(0),
	icon_type_table				= static_cast<unsigned int>(1),
	icon_type_function			= static_cast<unsigned int>(3),
	icon_type_thread			= static_cast<unsigned int>(0),
	icon_type_class				= static_cast<unsigned int>(2),
	icon_type_class_base		= static_cast<unsigned int>(5),
	icon_type_class_instance	= static_cast<unsigned int>(4),
	icon_type_unknown			= static_cast<unsigned int>(0),
#else // #if 0
	icon_type_nil				= static_cast<unsigned int>(0),
	icon_type_boolean			= static_cast<unsigned int>(1),
	icon_type_number			= static_cast<unsigned int>(2),
	icon_type_string			= static_cast<unsigned int>(3),
	icon_type_table				= static_cast<unsigned int>(4),
	icon_type_function			= static_cast<unsigned int>(5),
	icon_type_coroutine			= static_cast<unsigned int>(6),
	icon_type_class				= static_cast<unsigned int>(7),
	icon_type_class_base		= static_cast<unsigned int>(8),
	icon_type_class_instance	= static_cast<unsigned int>(9),
	icon_type_unknown			= static_cast<unsigned int>(10),
#endif // #if 0
}; // enum icon_type

struct DECLSPEC_NOVTABLE backend {
public:
	virtual	void	CS_LUA_STUDIO_BACKEND_CALL	type_to_string	(char* buffer, unsigned int size, lua_State* state, int index, bool& use_in_description) = 0;
	virtual	void	CS_LUA_STUDIO_BACKEND_CALL	value_to_string	(char* buffer, unsigned int size, lua_State* state, int index, icon_type& icon_type, bool full_description) = 0;
}; // struct DECLSPEC_NOVTABLE backend

struct DECLSPEC_NOVTABLE value_to_expand {
	virtual	void	CS_LUA_STUDIO_BACKEND_CALL	add_value		(char const* id, char const* type, char const* value, icon_type icon_type) = 0;
}; // struct DECLSPEC_NOVTABLE value_to_expand

} // namespace lua_studio
} // namespace cs

#endif // #ifndef CS_LUA_STUDIO_BACKEND_H_INCLUDED