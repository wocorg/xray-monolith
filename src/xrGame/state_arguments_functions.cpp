#include "stdafx.h"
#include "state_arguments_functions.h"

namespace award_system
{
	//definitions of static members of functions_cf

	template <typename T>
	ge_function<T> functions_cf<T>::ge_function;
	template <typename T>
	le_function<T> functions_cf<T>::le_function;

	// the instantiations in use
	template class functions_cf<float>;
	template class functions_cf<u32>;
} //namespace award_system
