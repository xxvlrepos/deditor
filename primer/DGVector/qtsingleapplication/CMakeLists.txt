cmake_minimum_required(VERSION 3.10)

#*********************************
# Название модуля/папки
set(PROJECT qtsingleapplication)

project(${PROJECT} LANGUAGES CXX)
set(LIBS)
#**********************************
# опции настраиваемые

#***********************************
#  приамбулы для QT
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#***********************************
# глобальные дефайны

#**********************************
# модули QT
set(QT_MODULE #${QT_MODULE}
    Gui
    Widgets
    Core
    Network
    )

#****************************************************
#   этот код трогать ненадо - добавление модулей

foreach(QMOD ${QT_MODULE})
    find_package(Qt5 COMPONENTS  ${QMOD}  REQUIRED)
    set(LIBS ${LIBS} Qt5::${QMOD})
endforeach()
#*****************************************************
# если исходники находятс в поддиректории - указать путь

set(SUBDIR "src")
set_property(GLOBAL PROPERTY cursubdir "${SUBDIR}")

#*******************************************
#  файлы модуля, если в подпапке, то ${SUBDIR}/

set(SOURCES #${SOURCES}
         ${SUBDIR}/qtlocalpeer.cpp
         ${SUBDIR}/qtsingleapplication.cpp
         ${SUBDIR}/qtsinglecoreapplication.cpp
    )


set(HEADERS #${HEADERS}
         ${SUBDIR}/qtlocalpeer.h
         ${SUBDIR}/qtsingleapplication.h
         ${SUBDIR}/qtsinglecoreapplication.h
         ${SUBDIR}/qtlockedfile.h
        )

  
add_library(${PROJECT} STATIC ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} PRIVATE ${LIBS})

