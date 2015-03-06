file(COPY Data DESTINATION .)

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

