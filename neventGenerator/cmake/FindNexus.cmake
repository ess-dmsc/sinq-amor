find_path(NEXUS_INCLUDE_DIR NAMES nexus/napi.h)
find_library(NEXUS_LIBRARIES NAMES NeXus)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NEXUS DEFAULT_MSG
	NEXUS_INCLUDE_DIR
	NEXUS_LIBRARIES
)
