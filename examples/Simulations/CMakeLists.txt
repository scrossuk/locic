find_package(CSFML 2.0 COMPONENTS graphics window system)

if(NOT CSFML_FOUND)
	message(WARNING "Failed to find CSFML so can't build Simulations example.")
	return()
endif()

file(COPY Data DESTINATION .)

loci_get_3rdparty_api_files(SFML_API_FILES SFML)

loci_module(Simulations_Module simulations.bc ""
	main.loci
	Random.loci
	Simulation.loci
	SimulationList.loci
	Support.loci
	Vector.loci
	Checkpoint/Particle.loci
	Checkpoint/RandomSimulation.loci
	Checkpoint/Target.loci
	Flocking/Boid.loci
	Flocking/Field.loci
	Flocking/FollowingSimulation.loci
	Flocking/Obstacle.loci
	Flocking/System.loci
	Gravity/CentralOrbitSimulation.loci
	Gravity/Particle.loci
	Gravity/System.loci
	${SFML_API_FILES}
	${STD_CONTAINER_API_FILES}
	${STD_STRING_API_FILES}
)

loci_link(Simulations simulations "-lcsfml-graphics -lcsfml-window -lcsfml-system"
	simulations.bc
	${STD_CONTAINER_BINARY_FILES}
	${STD_STRING_BINARY_FILES}
	${STD_MEMORY_BINARY_FILES}
)

add_dependencies(Simulations std.container std.memory std.string)
