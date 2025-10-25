

project(libflv_test)
cmake_minimum_required(VERSION 2.8)
MESSAGE(STATUS  "####################################")
MESSAGE(STATUS  "#############${PROJECT_NAME}###############")
MESSAGE(STATUS  "####################################")

#add_definitions(
				#	-DWIN32
				#	-D_DEBUG
				##	-D_WINDOWS
				#	-D_UNICODE
				#	-DUNICODE
				#	-D_AFXDLL
#)

 


include_directories(
	../
	../../
	../../../
	../../../libwebrtc
	../../../deps/libyuv/include
	../../../deps/abseil-cpp/
	../../../deps/curl-7.81.0/include
	../../../deps/jsoncpp/source/include
	${DEPS_DIR}/ffmpeg/include
)



#usrsctplib

file(GLOB   libflv_test_source 
	 
	*.h
	*.cpp 
	 *.cc 
  
	
	 
) 


 
#add_library(aac_decder  STATIC  ${audio_aac_source})
 
add_executable(${PROJECT_NAME}     ${libflv_test_source}      )
add_dependencies(${PROJECT_NAME} libmedia_transfer_protocol)
 add_dependencies(${PROJECT_NAME}   libmedia_codec)
# add_dependencies(${PROJECT_NAME} libsip)

set(ALL_LIB 
 
    faad
	
	libmedia_codec
	 libmedia_transfer_protocol
	)

target_link_libraries(${PROJECT_NAME}    ${ALL_LIB} )

#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /SAFESEH:NO")


set(AllFile ${libflv_test_source} )
foreach(fileItem ${AllFile})       
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${fileItem}" DIRECTORY)
	
    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")
	#message(status   "---> fileItem = ${fileItem}     GROUP ---->  ${GROUP}")
    # Group into "Source Files" and "Header Files"
    set(GROUP "${GROUP}")
    source_group("${GROUP}" FILES "${fileItem}")
endforeach()



set_property(TARGET ${PROJECT_NAME}  				PROPERTY FOLDER libmedia/media_transfer_protocol_test) 

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})