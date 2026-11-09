if(NOT TARGET evolution)
    ariadne_add_submodule(evolution)
endif()
if(NOT TARGET smt)
    ariadne_add_submodule(smt)
endif()
if(NOT TARGET sat)
    ariadne_add_submodule(sat)
endif()

# Ariadne Testing is used only by the test executables.
if(NOT TARGET ariadne-testing)
    ariadne_add_submodule(testing)
endif()

# Benchmarks are development-only, not a production Reasoning dependency.
if(NOT TARGET ariadne-benchmarking)
    ariadne_add_submodule(benchmarking)
endif()
