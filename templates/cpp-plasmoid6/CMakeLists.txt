cmake_minimum_required(VERSION 3.16)

project(lingmo-%{APPNAMELC})

set(QT_MIN_VERSION "6.4.0")
set(KF6_MIN_VERSION "5.93.0")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Lingmo
    I18n
)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    Core
)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
