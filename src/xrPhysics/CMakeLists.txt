# Source list generated from xrPhysics.vcxproj by tools/vcxproj-sources.ps1
add_library(xrPhysics STATIC
	../xrServerEntities/PHNetState.cpp
	ActorCameraCollision.cpp
	collisiondamagereceiver.cpp
	console_vars.cpp
	dcylinder/dCylinder.cpp
	debug_output.cpp
	DisablingParams.cpp
	dRayMotions.cpp
	ElevatorState.cpp
	ExtendedGeom.cpp
	Geometry.cpp
	GeometryBits.cpp
	IActivationShape.cpp
	MathUtils.cpp
	MovementBoxDynamicActivate.cpp
	params.cpp
	PHActivationShape.cpp
	PHActorCharacter.cpp
	PHAICharacter.cpp
	PHCapture.cpp
	PHCaptureInit.cpp
	PHCharacter.cpp
	PHCollideValidator.cpp
	PHContactBodyEffector.cpp
	PHDisabling.cpp
	PHDynamicData.cpp
	PHElement.cpp
	PHElementNetState.cpp
	PHFracture.cpp
	PHGeometryOwner.cpp
	PHInterpolation.cpp
	PHIsland.cpp
	PHJoint.cpp
	PHJointDestroyInfo.cpp
	PHMoveStorage.cpp
	PHObject.cpp
	PHShell.cpp
	PHShellActivate.cpp
	PHShellNetState.cpp
	PHShellSplitter.cpp
	PHSimpleCharacter.cpp
	PHSplitedShell.cpp
	PHStaticGeomShell.cpp
	phvalide.cpp
	PHWorld.cpp
	Physics.cpp
	PhysicsExternalCommon.cpp
	PhysicsShell.cpp
	PhysicsShellAnimator.cpp
	physics_scripted.cpp
	ShellHit.cpp
	stdafx.cpp
	tri-colliderknoopc/dSortTriPrimitive.cpp
	tri-colliderknoopc/dTriBox.cpp
	tri-colliderknoopc/dTriCallideK.cpp
	tri-colliderknoopc/dTriCylinder.cpp
	tri-colliderknoopc/dTriList.cpp
	tri-colliderknoopc/dTriSphere.cpp
	xrPhysics.cpp
)

target_compile_options(xrPhysics PUBLIC "-iquote${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(xrPhysics PRIVATE XRPHYSICS_EXPORTS)
target_link_libraries(xrPhysics PUBLIC xrCore xrCDB xrAPI xrSound ode)
