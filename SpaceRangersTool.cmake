add_executable(SpaceRangersTool 
        analyzer.cpp
        common_algo.cpp
        convert.cpp
        #db.cpp
        factory.cpp
        filefinder.cpp
        filters.cpp
        main.cpp
        model.cpp
        parser.cpp
        performance_tracker.cpp
        programargs.cpp

        analyzer.hpp
        analyzer_entities.h
        common_algo.h
        convert.h
        Entities.h
        factory.hpp
        filefinder.hpp
        filters.hpp
        model.hpp
        parser.h
        performance_tracker.hpp
        programargs.hpp
        sorters.hpp
)

target_link_libraries(SpaceRangersTool PRIVATE 
        Boost::program_options 
        )

set_target_properties(SpaceRangersTool
        PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
        #DEBUG_POSTFIX "_d"
        )