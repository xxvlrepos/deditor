cmake_minimum_required(VERSION 3.10)

#*********************
# Название модуля/папки
set(PROJECT mdata)

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
    Core
    )
    
#****************************************************
#  особые опциии!!!!
include_directories(${PROJECT} PRIVATE ../server)

#****************************************************
#   этот код трогать ненадо - добавление модулей

foreach(QMOD ${QT_MODULE})
    find_package(Qt5 COMPONENTS  ${QMOD}  REQUIRED)
    set(LIBS ${LIBS} Qt5::${QMOD})
endforeach()
#*****************************************************
# если исходники находятс в поддиректории - указать путь

set(SUBDIR "")
set_property(GLOBAL PROPERTY cursubdir "${SUBDIR}")

#*******************************************
#  файлы модуля, если в подпапке, то ${SUBDIR}/


set(HEADERS #${HEADERS}
   mdata.h
  )

set(SOURCES #${SOURCES}
   mdata.cpp
 )

add_library(${PROJECT} STATIC ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT} PRIVATE ${LIBS})
