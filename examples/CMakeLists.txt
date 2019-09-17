cmake_minimum_required(VERSION 3.10)

project(open62541pp-examples)

# find all cpp source files in sandbox
file(GLOB examples_sources "*.cpp")
foreach(source ${examples_sources})
    # extract filename
    get_filename_component(name ${source} NAME_WE)
    # create executeable
    add_executable(${name} ${source})
    # link to core library
    target_link_libraries(${name} open62541pp::open62541pp project_options)
endforeach(source ${sandbox_sources})