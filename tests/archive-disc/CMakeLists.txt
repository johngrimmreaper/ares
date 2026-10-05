add_executable(archive-disc-test archive-disc.cpp generate-fixtures.sh)

target_compile_definitions(archive-disc-test PRIVATE MIA_LIBRARY)
target_include_directories(archive-disc-test PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/thirdparty)
target_link_libraries(archive-disc-test PRIVATE ares::mia ares::ares ares::nall tzxfile)

set(CONSOLE TRUE)
ares_configure_executable(archive-disc-test)
set_target_properties(archive-disc-test PROPERTIES FOLDER tests PREFIX "")
target_enable_subproject(archive-disc-test "archive disc container test harness")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES archive-disc.cpp generate-fixtures.sh)
