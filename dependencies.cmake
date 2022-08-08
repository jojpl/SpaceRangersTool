#cmake -G "CodeBlocks - MinGW Makefiles" -DBoost_ROOT=C:\src\boost_1_73_0 -DBoost_ARCHITECTURE=-x32

set(Boost_USE_STATIC_LIBS TRUE)
set(Boost_USE_MULTITHREADED TRUE)
find_package(Boost REQUIRED COMPONENTS 
    program_options
	filesystem
	)