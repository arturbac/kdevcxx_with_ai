
find_package(ECM "5.68.0" CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)


find_package(Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)
find_package(KF5
    REQUIRED COMPONENTS
        CoreAddons # Core addons on top of QtCore
        I18n # For localization
        TextEditor # The editor component
        WidgetsAddons
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(plugin_common)
add_subdirectory(kate_with_ai)

find_package(KDevPlatform 5.2.40)
if(KDevPlatform_FOUND)
  add_subdirectory(kdevcxx_with_ai)
endif()
