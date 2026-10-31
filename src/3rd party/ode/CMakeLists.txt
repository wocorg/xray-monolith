set(ODE_SRC ode/src)
add_library(ode STATIC
	${ODE_SRC}/array.cpp
	${ODE_SRC}/Bounder33.cpp
	${ODE_SRC}/collision_kernel.cpp
	${ODE_SRC}/collision_quadtreespace.cpp
	${ODE_SRC}/collision_space.cpp
	${ODE_SRC}/collision_std.cpp
	${ODE_SRC}/collision_transform.cpp
	${ODE_SRC}/collision_util.cpp
	${ODE_SRC}/error.cpp
	${ODE_SRC}/export-dif.cpp
	${ODE_SRC}/fastdot.c
	${ODE_SRC}/fastldlt.c
	${ODE_SRC}/fastlsolve.c
	${ODE_SRC}/fastltsolve.c
	${ODE_SRC}/joint.cpp
	${ODE_SRC}/lcp.cpp
	${ODE_SRC}/mass.cpp
	${ODE_SRC}/mat.cpp
	${ODE_SRC}/matrix.cpp
	${ODE_SRC}/memory.cpp
	${ODE_SRC}/misc.cpp
	${ODE_SRC}/obstack.cpp
	${ODE_SRC}/ode.cpp
	${ODE_SRC}/odemath.cpp
	${ODE_SRC}/quickstep.cpp
	${ODE_SRC}/rotation.cpp
	${ODE_SRC}/step.cpp
	${ODE_SRC}/stepfast.cpp
	${ODE_SRC}/StepJointInternal.cpp
	${ODE_SRC}/testing.cpp
	${ODE_SRC}/timer.cpp
	${ODE_SRC}/util.cpp
	contrib/msvc7/ode_default/de_padf_integration.cpp
)

target_include_directories(ode PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(ode PUBLIC dSINGLE PRIVATE dNODEBUG)
