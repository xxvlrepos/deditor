cmake_minimum_required(VERSION 3.10)

#*********************************
# Название модуля/папки
set(PROJECT qtservice)

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
set(QT_MODULE ${QT_MODULE}
    Core
    )

if(WIN32)
   find_package(user32)
   set(LIBS ${LIBS} user32)
else()
    set(QT_MODULE #${QT_MODULE}
        Network
        )
endif()
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

set(HEADERS #${HEADERS}
         ${SUBDIR}/qtservice.h
         ${SUBDIR}/qtservice_p.h
        )
set(SOURCES #${SOURCES}
         ${SUBDIR}/qtservice.cpp
    )

if(WIN32)
    set(SOURCES ${SOURCES}
             ${SUBDIR}/qtservice_win.cpp
        )
else()
    set(HEADERS ${HEADERS}
             ${SUBDIR}/qtunixsocket.h
             ${SUBDIR}/qtunixserversocket.h
            )
    set(SOURCES ${SOURCES}
             ${SUBDIR}/qtservice_unix.cpp
             ${SUBDIR}/qtunixsocket.cpp
             ${SUBDIR}/qtunixserversocket.cpp
        )
endif()

  
add_library(${PROJECT} STATIC ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} PRIVATE ${LIBS})

