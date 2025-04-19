# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "ARender_autogen"
  "CMakeFiles\\ARender_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\ARender_autogen.dir\\ParseCache.txt"
  )
endif()
