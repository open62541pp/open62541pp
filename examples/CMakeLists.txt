# find all example source files
file(GLOB examples_sources "*.cpp")
foreach(source ${examples_sources})
    # extract filename
    get_filename_component(name ${source} NAME_WE)
    # use open62541pp_example_ prefix for target to prevent naming collisions
    set(target_name "open62541pp_examples_${name}")
    add_executable(${target_name} ${source})
    target_link_libraries(${target_name} open62541pp::open62541pp open62541pp_project_options)
    set_target_properties(${target_name} PROPERTIES OUTPUT_NAME ${name})
endforeach(source ${sandbox_sources})