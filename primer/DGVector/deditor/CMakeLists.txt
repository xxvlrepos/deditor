cmake_minimum_required(VERSION 3.10)

#*********************
# Название модуля/папки
set(PROJECT deditor)

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
    Gui
    Widgets
    Xml
    XmlPatterns
    )
#**********************************
# модули пользователя из подпапок
#set(USER_MODULE ${USER_MODULE}
#    hexspinbox
#    )
#****************************************************
#  особые опциии!!!!
include_directories(${PROJECT} PRIVATE ../)
include_directories(${PROJECT} PRIVATE ../hexspinbox)
include_directories(${PROJECT} PRIVATE ../server)
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
   deditor.h
   domattr.h
   domitem.h
   dommodel.h
   xmlschema.h
  )

set(SOURCES #${SOURCES}
   deditor.cpp
   domattr.cpp
   domitem.cpp
   dommodel.cpp
   xmlschema.cpp
 )
 
 set(UI_FILES #${UI_FILES}
    deditor.ui
  )

add_library(${PROJECT} STATIC ${HEADERS} ${SOURCES} ${UI_FILES})

target_link_libraries(${PROJECT} PRIVATE ${LIBS})
