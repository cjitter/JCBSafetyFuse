cmake_minimum_required(VERSION 3.20)

project(JCBSafetyFuse VERSION 1.0.1)

# Configuración C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

# Versión mínima de macOS
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Versión mínima de macOS")

# Binary Universal (Apple Silicon + Intel)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Arquitecturas de compilación para macOS")

# Descargar JUCE directamente con FetchContent
include(FetchContent)
FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
)
# Opciones de JUCE para evitar compilar extras innecesarios
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(JUCE)

# Opciones de configuración
option(JUCE_BUILD_AAX "Build AAX format (requires AAX SDK and Pro Tools Developer)" ON)
option(JUCE_AUTO_INSTALL_PLUGINS "Auto-install plugins after build" ON)
option(JCB_DISABLE_SANITIZER "Disable runtime output sanitizer (only when debugging issues)" OFF)

# SDK externo opcional (AAX) para macOS
set(JUCE_AAX_SDK_PATH "$ENV{HOME}/SDK/aax-sdk-2-8-1" CACHE PATH "Ruta al SDK de AAX")

# Definir los formatos soportados según el estado del flag
if(JUCE_BUILD_AAX)
    set(PLUGIN_FORMATS VST3 AU AAX)
else()
    set(PLUGIN_FORMATS VST3 AU)
endif()

# Crear el plugin
juce_add_plugin(JCBSafetyFuse
        # Información básica del plugin
        PLUGIN_NAME "JCBSafetyFuse"
        PRODUCT_NAME "JCBSafetyFuse"
        VERSION 1.0.1
        DESCRIPTION "Multichannel safety fuse and numeric explosion protection"

        # Información de la empresa
        COMPANY_NAME "Coeval"
        COMPANY_WEBSITE "https://github.com/cjitter"
        BUNDLE_ID "com.coeval.JCBSafetyFuse"
        PLUGIN_MANUFACTURER_CODE "CoeV"
        PLUGIN_CODE "CoE8"

        # Formatos soportados
        FORMATS ${PLUGIN_FORMATS}

        # Configuración del plugin
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE

        # Categorías por formato
        VST3_CATEGORIES Fx Dynamics
        AU_MAIN_TYPE kAudioUnitType_Effect
        AAX_CATEGORY AAX_ePlugInCategory_Dynamics
)

# BinaryData para imágenes de fondo
juce_add_binary_data(JCBSafetyFuseData SOURCES
        Assets/fondo.png
        Assets/blow.png
)

# Archivos fuente del plugin (SIN Gen)
target_sources(JCBSafetyFuse PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Includes
target_include_directories(JCBSafetyFuse PRIVATE
        Source/
)

if(JCB_DISABLE_SANITIZER)
    target_compile_definitions(JCBSafetyFuse PRIVATE JCB_DISABLE_SANITIZER=1)
endif()

# Módulos JUCE necesarios
target_link_libraries(JCBSafetyFuse PRIVATE
        JCBSafetyFuseData
        juce::juce_audio_basics
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
)

# Definiciones del compilador
target_compile_definitions(JCBSafetyFuse PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_STRICT_REFCOUNTEDPTR=1
)

# Flags del compilador para macOS (Clang)
target_compile_options(JCBSafetyFuse PRIVATE
        -Wall -Wextra -Wno-unused-parameter
)

# Instalación automática para macOS
if(JUCE_AUTO_INSTALL_PLUGINS)
    set(VST3_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/VST3")
    set(AU_DIR "$ENV{HOME}/Library/Audio/Plug-Ins/Components")
    set(AAX_DIR "/Applications/Pro Tools Developer/Plug-Ins")

    if(TARGET JCBSafetyFuse_VST3)
        add_custom_command(TARGET JCBSafetyFuse_VST3 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${VST3_DIR}"
                COMMAND ${CMAKE_COMMAND} -E remove_directory "${VST3_DIR}/JCBSafetyFuse.vst3"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_BINARY_DIR}/JCBSafetyFuse_artefacts/$<CONFIG>/VST3/JCBSafetyFuse.vst3"
                "${VST3_DIR}/JCBSafetyFuse.vst3"
        )
    endif()

    if(TARGET JCBSafetyFuse_AU)
        add_custom_command(TARGET JCBSafetyFuse_AU POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${AU_DIR}"
                COMMAND ${CMAKE_COMMAND} -E remove_directory "${AU_DIR}/JCBSafetyFuse.component"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_BINARY_DIR}/JCBSafetyFuse_artefacts/$<CONFIG>/AU/JCBSafetyFuse.component"
                "${AU_DIR}/JCBSafetyFuse.component"
        )
    endif()

    if(TARGET JCBSafetyFuse_AAX)
        add_custom_command(TARGET JCBSafetyFuse_AAX POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E remove_directory "${AAX_DIR}/JCBSafetyFuse.aaxplugin"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_BINARY_DIR}/JCBSafetyFuse_artefacts/$<CONFIG>/AAX/JCBSafetyFuse.aaxplugin"
                "${AAX_DIR}/JCBSafetyFuse.aaxplugin"
        )
    endif()
endif()

message(STATUS "JCBSafetyFuse ${PROJECT_VERSION} - Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build directory: ${CMAKE_CURRENT_BINARY_DIR}/JCBSafetyFuse_artefacts/")
