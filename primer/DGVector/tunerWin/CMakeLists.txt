cmake_minimum_required(VERSION 3.10)

#*********************
# Название модуля/папки
set(PROJECT tunerWin)

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
    Widgets
    Core
    Xml
    XmlPatterns
    PrintSupport
    )
#set(USER_MODULE ${USER_MODULE}
#   )
#****************************************************
#  особые опциии!!!!
    include_directories(${PROJECT} PRIVATE ../)
    include_directories(${PROJECT} PRIVATE ../server)
    include_directories(${PROJECT} PRIVATE ../mdata)
    include_directories(${PROJECT} PRIVATE ../former)
    include_directories(${PROJECT} PRIVATE ../xmlConverter)
    include_directories(${PROJECT} PRIVATE ../helpbrowser)
    include_directories(${PROJECT} PRIVATE ../deditor)
    include_directories(${PROJECT} PRIVATE ../search)
    include_directories(${PROJECT} PRIVATE ../execut)
    include_directories(${PROJECT} PRIVATE ../com485)

#****************************************************
#   этот код трогать ненадо - добавление модулей

foreach(QMOD ${QT_MODULE})
    find_package(Qt5 COMPONENTS  ${QMOD}  REQUIRED)
    set(LIBS ${LIBS} Qt5::${QMOD})
endforeach()

#set(LIBS)
#foreach(UMOD ${USER_MODULE})
  #   add_subdirectory(${UMOD})
  #   get_property(SUBDIR GLOBAL PROPERTY cursubdir)
  #   target_include_directories(${UMOD} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/${UMOD}/${SUBDIR})
  #   include_directories(BEFORE ${UMOD}/${SUBDIR})
  #   include_directories(BEFORE ${CMAKE_BINARY_DIR}/${UMOD}/${UMOD}_autogen/include)
  #   set(LIBS ${LIBS} ${UMOD})
  #   MESSAGE( STATUS "Add MODULE:" ${UMOD} )
#endforeach()

#*****************************************************
# если исходники находятс в поддиректории - указать путь

set(SUBDIR "")
set_property(GLOBAL PROPERTY cursubdir "${SUBDIR}")

#*******************************************
#  файлы мод

set(HEADERS #${HEADERS}
   tunerwin.h
  )

set(SOURCES #${SOURCES}
   tunerwin.cpp
   )
   
 set(UI_FILES #${UI_FILES}
    tunerwin.ui
  )

get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
    set(HEADERS ${HEADERS}
        ../deditor/deditor.h
        ../search/search.h
      )
     set(UI_FILES ${UI_FILES}
      ../deditor/deditor.ui
      ../search/search.ui
     )
else()
     include_directories(BEFORE ${CMAKE_BINARY_DIR}/deditor/deditor_autogen/include)
     include_directories(BEFORE ${CMAKE_BINARY_DIR}/search/search_autogen/include)
endif()

add_library(${PROJECT} STATIC 
    ${HEADERS} 
    ${SOURCES} 
    ${UI_FILES}
    )   

target_link_libraries(${PROJECT} PRIVATE ${LIBS})
