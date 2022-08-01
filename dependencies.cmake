set(Boost_USE_STATIC_LIBS TRUE)
set(Boost_USE_MULTITHREADED TRUE)
find_package(Boost REQUIRED COMPONENTS 
    #boost
    program_options 
    system)