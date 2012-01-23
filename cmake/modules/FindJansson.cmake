
find_path(JANSSON_INCLUDE_DIR jansson.h)
find_library(JANSSON_LIBRARY NAMES jansson)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(jansson DEFAULT_MSG JANSSON_LIBRARY JANSSON_INCLUDE_DIR)
set(JANSSON_LIBRARIES ${JANSSON_LIBRARY})
mark_as_advanced(JANSSON_LIBRARY JANSSON_INCLUDE_DIR)

if(JANSSON_LIBRARY AND JANSSON_INCLUDE_DIR)
   set(JANSSON_FOUND TRUE)
endif()
