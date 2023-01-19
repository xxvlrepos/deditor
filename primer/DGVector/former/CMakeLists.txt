cmake_minimum_required(VERSION 3.10)

#*********************
# Название модуля/папки
set(PROJECT former)

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
    Widgets
    PrintSupport
    )
#**********************************
# модули пользователя из подпапок
#set(USER_MODULE ${USER_MODULE}
#    hexspinbox
#    )
#****************************************************
#  особые опциии!!!!
set_property(SOURCE report.h PROPERTY SKIP_AUTOUIC ON)
include_directories(${PROJECT} PRIVATE ../server)
include_directories(${PROJECT} PRIVATE ../mdata)
include_directories(${PROJECT} PRIVATE ../parser)
#****************************************************
#   этот код трогать ненадо - добавление модулей

foreach(QMOD ${QT_MODULE})
    find_package(Qt5 COMPONENTS  ${QMOD}  REQUIRED)
    set(LIBS ${LIBS} Qt5::${QMOD})
endforeach()

#foreach(UMOD ${USER_MODULE})
#     add_subdirectory(${UMOD})
#     get_property(SUBDIR GLOBAL PROPERTY cursubdir)
#     target_include_directories(${UMOD} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${UMOD}/${SUBDIR})
#     include_directories(BEFORE ${UMOD}/${SUBDIR})
#     include_directories(BEFORE ${CMAKE_BINARY_DIR}/${UMOD}/${UMOD}_autogen/include)
#     set(LIBS ${LIBS} ${UMOD})
#     MESSAGE( STATUS "Add MODULE:" ${UMOD} )
#endforeach()
#*****************************************************
# если исходники находятс в поддиректории - указать путь

set(SUBDIR "")
set_property(GLOBAL PROPERTY cursubdir "${SUBDIR}")

#*******************************************
#  файлы модуля, если в подпапке, то ${SUBDIR}/


set(HEADERS #${HEADERS}
   former.h
   info.h
   report.h
   ui_report.h
  )

set(SOURCES #${SOURCES}
   former.cpp
   info.cpp
   report.cpp
 )
 
# set(UI_FILES #${UI_FILES}
#    
#  )

add_library(${PROJECT} STATIC 
    ${HEADERS} 
    ${SOURCES} 
#   ${UI_FILES}
    )

target_link_libraries(${PROJECT} PRIVATE ${LIBS})
