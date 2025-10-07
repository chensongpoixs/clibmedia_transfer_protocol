

project(libmedia_transfer_protocol)
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
	../libwebrtc
	../deps/libyuv/include
	../deps/abseil-cpp/
	../deps/curl-7.81.0/include
	../deps/jsoncpp/source/include
)



#usrsctplib

file(GLOB libmedia_transfer_protocol_source 
	 
	*.h
	*.cpp 
	 *.cc 
	 crypto/*.h
	 crypto/*.cpp 
	 sctp/*.cpp 
	 sctp/*.h
	 rtp/*.cpp 
	 rtp/*.h 
	 rtp_rtcp/rtp_util.h
	 rtp_rtcp/rtp_util.cpp 
	 rtp_rtcp/byte_io.h
	  rtp_rtcp/rtp_header_extension_map.h
rtp_rtcp/rtp_header_extension_map.cpp 

	 
	rtp_rtcp/rtp_dependency_descriptor_extension.h
	rtp_rtcp/rtp_dependency_descriptor_extension.cpp 
	rtp_rtcp/rtp_generic_frame_descriptor_extension.h
	rtp_rtcp/rtp_generic_frame_descriptor_extension.cpp 
	rtp_rtcp/rtp_header_extensions.h
	rtp_rtcp/rtp_header_extensions.cpp 
	rtp_rtcp/rtp_video_layers_allocation_extension.h
	rtp_rtcp/rtp_video_layers_allocation_extension.cpp
	rtp_rtcp/rtp_rtcp_defines.h
	rtp_rtcp/rtp_rtcp_defines.cpp 
	rtp_rtcp/rtp_packet.h
	rtp_rtcp/rtp_packet.cpp
	rtp_rtcp/rtcp_packet/*.h 
	rtp_rtcp/rtcp_packet/*.cc 
	rtp_rtcp/rtcp_packet/*.cpp 
	 rtp_rtcp/*.cpp 
	 rtp_rtcp/*.h 
	 rtp_rtcp/*.cc 
	 #rtp_rtcp/rtcp_packet/*.cpp 
	 #rtp_rtcp/rtcp_packet/*.h 
	 #rtp_rtcp/rtcp_packet/*.cc 
	 rtp_rtcp/deprecated/*.cpp 
	 rtp_rtcp/deprecated/*.cc 
	 rtp_rtcp/deprecated/*.h 
	 congestion_controller/*.h
	 congestion_controller/*.cpp 
	 congestion_controller/*.cc 
	 
	 pacing/*.cc 
	 pacing/*.h 
	 pacing/*.cpp 
	 remote_bitrate_estimator/*.h 
	 remote_bitrate_estimator/*.cc 
	 remote_bitrate_estimator/*.cpp 
	 
	 librtsp/*.h
	 librtsp/*.cpp 
	 
) 


 
add_library(${PROJECT_NAME}   STATIC  ${libmedia_transfer_protocol_source})
 

 


#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /SAFESEH:NO")


set(AllFile ${libmedia_transfer_protocol_source} )
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



#set_property(TARGET ${PROJECT_NAME}  				PROPERTY FOLDER libice) 

#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})