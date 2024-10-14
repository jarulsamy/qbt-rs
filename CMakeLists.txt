# CMakeList.txt : CMake project for QBT-Organizer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("QBT-Organizer")

find_package(ftxui CONFIG REQUIRED)
find_package(Boost COMPONENTS program_options log_setup log json date_time spirit phoenix REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (QBT-Organizer 
	"main.cpp"  
	 
	"debug.cpp" 
	"debug.h" 
	"logger.cpp" 
	"logger.h" 
	"qbt/qbt.cpp" 
	"qbt/qbt.h" 
	"qbt/torrent.cpp" 
	"qbt/torrent.h" 
	"qbt/torrentList.h" 
	"qbt/web.cpp"
	"qbt/web.h" 
	"qbt/core.h"
 "tui/tui.h" "tui/tui.cpp" "qbt/shell.h" "qbt/shell.cpp" "fs/fs.h" "fs/core.h" "fs/fs.cpp" "qbt/torrentContents.h")

target_link_libraries(QBT-Organizer PRIVATE Boost::date_time)
target_link_libraries(QBT-Organizer PRIVATE Boost::json)
target_link_libraries(QBT-Organizer PRIVATE Boost::log)
target_link_libraries(QBT-Organizer PRIVATE Boost::log_setup)
target_link_libraries(QBT-Organizer PRIVATE Boost::program_options)
target_link_libraries(QBT-Organizer PRIVATE Boost::spirit)
target_link_libraries(QBT-Organizer PRIVATE Boost::phoenix)
target_link_libraries(QBT-Organizer PRIVATE CURL::libcurl)
target_link_libraries(QBT-Organizer PRIVATE fmt::fmt)
target_link_libraries(QBT-Organizer PRIVATE ftxui::dom ftxui::screen ftxui::component)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET QBT-Organizer PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
