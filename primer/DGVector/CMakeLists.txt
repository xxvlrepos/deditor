cmake_minimum_required(VERSION 3.10)

#*********************
# Название проекта
set(PROJECT DGVector)

project(${PROJECT} LANGUAGES CXX)
set(LIBS)
#**********************************
# опции настраиваемые


#**********************************
#  приамбулы для QT
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#***********************************
# глобальные дефайны
if(WIN32)
    add_definitions(-D_WIN_)
endif()

#**********************************
# модули QT
set(QT_MODULE #${QT_MODULE}
    Widgets
    Xml
    XmlPatterns
    PrintSupport
    Sql
    )

#**********************************
# модули пользователя из подпапок
set(USER_MODULE
    server
    execut
    mdata
    libmaia
    com485
    former
    helpbrowser  
    parser
    qtsingleapplication
    deditor
    search
    xmlConverter
    tunerWin   
    hexspinbox
    )
#****************************************************
#  особые опциии!!!!1)
include_directories(${PROJECT} PRIVATE ./server/opcua62541)
include_directories(${PROJECT} PRIVATE ./server/opcua62541/open62541)
include_directories(${PROJECT} PRIVATE ./server/opcua62541/qopen62541)
include_directories(${PROJECT} PRIVATE ./server/opcua62541/qopen62541/QtOpcUa)
#**********************************
# модули и дефайны по условию
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
    set(EXE_PATH "WINGCC_5")
else()
    set(EXE_PATH "MVC2015")
endif()

#***************************************************************************
#   этот код трогать ненадо - добавление модулей и библиотек автоматическое

foreach(QMOD ${QT_MODULE})
    find_package(Qt5 COMPONENTS  ${QMOD}  REQUIRED)
    set(LIBS ${LIBS} Qt5::${QMOD})
  # MESSAGE( STATUS "Add QT MODULE:" ${QMOD} )
endforeach()

foreach(UMOD ${USER_MODULE})
     add_subdirectory(${UMOD})
     get_property(SUBDIR GLOBAL PROPERTY cursubdir)
     target_include_directories(${UMOD} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${UMOD}/${SUBDIR})

     include_directories(BEFORE ${UMOD}/${SUBDIR})
     include_directories(BEFORE ${CMAKE_BINARY_DIR}/${UMOD}/${UMOD}_autogen/include)

     set(LIBS ${LIBS} ${UMOD})
     MESSAGE( STATUS "Add MODULE:" ${UMOD} )
endforeach()

#**************************************************
# подпроект
set(EXT_MODULE
    qtservice
    dgserver
    )
foreach(EXT ${EXT_MODULE})
     add_subdirectory(${EXT})
     get_property(SUBDIR GLOBAL PROPERTY cursubdir)
     target_include_directories(${EXT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${EXT}/${SUBDIR})
     MESSAGE( STATUS "Ext MODULE:" ${EXT} )
endforeach()

#*******************************************************************************
#  файлы головного проекта

set(HEADERS #${HEADERS}
    dgvector.h 
   )
  

set(SOURCES #${SOURCES}
    main.cpp
    dgvector.cpp
   )

#set(UI_FILES #${UI_FILES}   )
  
set(RESOURCES #${RESOURCES}
     dgvector.qrc
    )  
    
set(LANGUAGE_TS_FILES #${LANGUAGE_TS_FILES}
    resource/dgvector_ru.ts
    )


add_executable(${PROJECT}
    ${HEADERS}
    ${SOURCES}
    ${UI_FILES}
    ${RESOURCES}
    ${LANGUAGE_TS_FILES}
    )
target_link_libraries(${PROJECT} PRIVATE ${LIBS})

#копируем после сборки в папку
if(WIN32)
    add_custom_command(
        TARGET ${PROJECT} POST_BUILD
        COMMAND  ${CMAKE_COMMAND} -E copy
                 ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}.exe
                 ${CMAKE_SOURCE_DIR}/windows/${EXE_PATH}/dgvector.exe
     )
endif()


