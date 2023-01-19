cmake_minimum_required(VERSION 3.10)

#*********************
# Название модуля/папки
set(PROJECT dgserver)

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
# локальные дефайны
if(WIN32)
    add_definitions(-D_WIN_)
endif()

#**********************************
# модули QT
set(QT_MODULE #${QT_MODULE}
    Core
    Xml
    XmlPatterns
    Sql
    )
#set(USER_MODULE ${USER_MODULE}
#   )
#****************************************************
#  особые опциии!!!!
#    include_directories(${PROJECT} PRIVATE ../)

    set(LIBS #${LIBS}
       qtservice
       server
       libmaia
       execut
       com485
       mdata
       qtsingleapplication
       )

#****************************************************
#   этот код трогать ненадо - добавление модулей

foreach(QMOD ${QT_MODULE})
    find_package(Qt5 COMPONENTS  ${QMOD}  REQUIRED)
    set(LIBS ${LIBS} Qt5::${QMOD})
endforeach()


#*****************************************************
# если исходники находятс в поддиректории - указать путь

#set(SUBDIR "")
#set_property(GLOBAL PROPERTY cursubdir "${SUBDIR}")

#*******************************************
#  файлы мод

set(HEADERS #${HEADERS}
   dgserver.h
  )

set(SOURCES #${SOURCES}
    main.cpp
    dgserver.cpp
   )
   

add_executable(${PROJECT}
    ${HEADERS} 
    ${SOURCES} 
    )   

target_link_libraries(${PROJECT} PRIVATE ${LIBS})

#копируем после сборки в папку
if(WIN32)
     add_custom_command(
        TARGET ${PROJECT} POST_BUILD
        COMMAND  ${CMAKE_COMMAND} -E copy
                 ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}.exe
                 ${CMAKE_SOURCE_DIR}/windows/${EXE_PATH}/${PROJECT}.exe
     )
endif()
