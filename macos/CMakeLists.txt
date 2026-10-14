cmake_minimum_required(VERSION 3.21)
# arm64 build of Immersive First Person for the native macOS game.
# Windows builds keep native/CMakeLists.txt. This folder is at the repo root, same place as the other Mac ports,
# so that Windows file stays untouched.

set(IFP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
find_package(Git QUIET)
set(_ifp_tag "")
if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${IFP_DIR}" describe --tags --abbrev=0 --match "v[0-9]*"
                    OUTPUT_VARIABLE _ifp_tag OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()
string(REGEX MATCH "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ifp_ver "${_ifp_tag}")
if(_ifp_ver)
    set(IFP_VER_MAJOR ${CMAKE_MATCH_1})
    set(IFP_VER_MINOR ${CMAKE_MATCH_2})
    set(IFP_VER_PATCH ${CMAKE_MATCH_3})
else()
    message(WARNING "no v<major>.<minor>.<patch> tag found, using 2.1.2")
    set(IFP_VER_MAJOR 2)
    set(IFP_VER_MINOR 1)
    set(IFP_VER_PATCH 2)
endif()
project(ImmersiveFirstPerson VERSION ${IFP_VER_MAJOR}.${IFP_VER_MINOR}.${IFP_VER_PATCH} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64")

if(NOT APPLE)
    message(FATAL_ERROR "This target builds the macOS Immersive First Person dylib")
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")

set(RED4EXT_SDK_DIR "${IFP_DIR}/../../RED4ext.SDK-macos" CACHE PATH "macOS RED4ext SDK checkout")
get_filename_component(RED4EXT_SDK_DIR "${RED4EXT_SDK_DIR}" ABSOLUTE)
set(SDK_INCLUDE "${RED4EXT_SDK_DIR}/include")
set(CODEWARE_DIR "${IFP_DIR}/../../cp2077-codeware" CACHE PATH "macOS Codeware checkout (its SDK-core address rows are reused)")
get_filename_component(CODEWARE_DIR "${CODEWARE_DIR}" ABSOLUTE)
if(NOT EXISTS "${SDK_INCLUDE}/RED4ext/Relocation-inl.hpp")
    message(FATAL_ERROR "no RED4ext SDK headers at ${SDK_INCLUDE} (set -DRED4EXT_SDK_DIR=...)")
endif()

add_compile_definitions(
    RED4EXT_PLATFORM_MACOS
    RED4EXT_HEADER_ONLY
)

get_filename_component(_ifp_root "${IFP_DIR}" ABSOLUTE)
add_compile_options("-ffile-prefix-map=${_ifp_root}=immersive_first_person"
                    "-ffile-prefix-map=${RED4EXT_SDK_DIR}=RED4ext.SDK-macos"
                    "-ffile-prefix-map=${CODEWARE_DIR}=cp2077-codeware"
                    "-ffile-prefix-map=${CMAKE_BINARY_DIR}=build")

# Generated SDK overlay: the Mac address table plus Codeware's macos/MacSdkRows.inc.
# An unknown hash goes to ifp_mac_trap_for_hash. The shared SDK checkout is not edited.
set(SDK_OVERLAY "${CMAKE_CURRENT_BINARY_DIR}/sdk-overlay")
set(SDK_RELOC "${SDK_INCLUDE}/RED4ext/Relocation-inl.hpp")
file(READ "${SDK_RELOC}" SDK_RELOC_TEXT)
set(CW_ROWS_INC "${CODEWARE_DIR}/macos/MacSdkRows.inc")
if(NOT EXISTS "${CW_ROWS_INC}")
    message(FATAL_ERROR "no Codeware Mac rows at ${CW_ROWS_INC} (set -DCODEWARE_DIR=...)")
endif()
file(READ "${CW_ROWS_INC}" CW_ROWS_TEXT)
set(SDK_ROWS_MARKER "        { 4201325776, 0x21739E8 },  // CGlobalFunction_ctor\n")
string(FIND "${SDK_RELOC_TEXT}" "${SDK_ROWS_MARKER}" SDK_ROWS_POS)
if(SDK_ROWS_POS EQUAL -1)
    message(FATAL_ERROR "the anchor row (CGlobalFunction_ctor) is not in ${SDK_RELOC}")
endif()
string(REPLACE "${SDK_ROWS_MARKER}" "${SDK_ROWS_MARKER}        // --- cp2077-codeware/macos/MacSdkRows.inc ---\n${CW_ROWS_TEXT}\n" SDK_RELOC_TEXT "${SDK_RELOC_TEXT}")
set(SDK_TAIL_START "    // Address not found or placeholder - return a non-zero sentinel value")
set(SDK_TAIL_END "    return 0;\n#else\n    if constexpr (Detail::AddressResolverOverride<uint32_t>::value)")
string(FIND "${SDK_RELOC_TEXT}" "${SDK_TAIL_START}" SDK_TAIL_A)
string(FIND "${SDK_RELOC_TEXT}" "${SDK_TAIL_END}" SDK_TAIL_B)
if(SDK_TAIL_A EQUAL -1 OR SDK_TAIL_B EQUAL -1)
    message(FATAL_ERROR "the unknown-hash tail of Resolve was not found in ${SDK_RELOC}")
endif()
string(SUBSTRING "${SDK_RELOC_TEXT}" 0 ${SDK_TAIL_A} SDK_HEAD)
math(EXPR SDK_REST_FROM "${SDK_TAIL_B} + 14")
string(SUBSTRING "${SDK_RELOC_TEXT}" ${SDK_REST_FROM} -1 SDK_REST)
set(SDK_RELOC_TEXT "${SDK_HEAD}    // Immersive First Person: unknown hash logs once and returns a stub. Never returns 0.\n    return ifp_mac_trap_for_hash(aHash);\n${SDK_REST}")
set(SDK_DECL_MARKER "extern \"C\" uintptr_t tweakxl_class_vslot(uint32_t aByteOffset);\n")
string(FIND "${SDK_RELOC_TEXT}" "${SDK_DECL_MARKER}" SDK_DECL_POS)
if(SDK_DECL_POS EQUAL -1)
    message(FATAL_ERROR "the extern declaration anchor was not found in ${SDK_RELOC}")
endif()
string(REPLACE "${SDK_DECL_MARKER}" "${SDK_DECL_MARKER}extern \"C\" uintptr_t ifp_mac_trap_for_hash(uint32_t aHash);\n" SDK_RELOC_TEXT "${SDK_RELOC_TEXT}")
file(WRITE "${SDK_OVERLAY}/RED4ext/Relocation-inl.hpp" "${SDK_RELOC_TEXT}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SDK_RELOC}" "${CW_ROWS_INC}")

set(IFP_SOURCES
    "${IFP_DIR}/native/src/Main.cpp"
    MacTrap.cpp
    MacVault.cpp
)

add_library(ImmersiveFirstPerson SHARED ${IFP_SOURCES})
set_target_properties(ImmersiveFirstPerson PROPERTIES OUTPUT_NAME "ImmersiveFirstPerson" PREFIX "" SUFFIX ".dylib")

target_include_directories(ImmersiveFirstPerson PRIVATE
    "${SDK_OVERLAY}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${SDK_INCLUDE}"
)

target_compile_options(ImmersiveFirstPerson PRIVATE
    -fexceptions
    -fms-extensions
    -Wno-deprecated-declarations
    -Wno-microsoft-template
    -Wno-invalid-offsetof
    -fvisibility=hidden
    -fvisibility-inlines-hidden
)

add_custom_command(TARGET ImmersiveFirstPerson POST_BUILD
    COMMAND /bin/bash "${CMAKE_CURRENT_SOURCE_DIR}/fix-install-names.sh" "$<TARGET_FILE:ImmersiveFirstPerson>"
    COMMAND /bin/bash "${CMAKE_CURRENT_SOURCE_DIR}/check-undefined.sh" "$<TARGET_FILE:ImmersiveFirstPerson>"
    VERBATIM
)

message(STATUS "Immersive First Person ${PROJECT_VERSION} configured for macOS")
