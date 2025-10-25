

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

 
 add_definitions(
 
	-DTEST_HTTP_FLV=1
 )

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
	 Dnsapi.lib   
			delayimp.lib
			ws2_32.lib
			 Qwave.lib
			 crypt32.lib
			iphlpapi.lib
			secur32.lib
			 #
			# oatpp
			# oatpp-sqlite
			# oatpp-swagger
			# sqlite
			crypt32.lib
			iphlpapi.lib
			secur32.lib
			 # absl 
			 abseil_dll
			# Meida_LIB
		#absl_cordz_functions
		#absl_cordz_handle
		#absl_cordz_info
		#absl_cordz_sample_token
		#absl_flags
		#absl_flags_commandlineflag_internal
		#absl_flags_internal
		#absl_flags_marshalling
		#absl_flags_parse
		#absl_flags_private_handle_accessor
		#absl_flags_program_name
		#absl_flags_reflection
		#absl_flags_usage
		#absl_flags_usage_internal
		#absl_low_level_hash
		#absl_random_internal_distribution_test_util
		api 
			audio_device
			audio_processing
			desktop_capture
			video_capture
			video_coding
			audio_coding

			call
			common_audio
			common_video
			libwebrtc 
			logging 
			media
			p2p 
			pc 
			rtc_base
		 
				  
				  iphlpapi.lib
		#crypto32.lib 
		ws2_32   
		crypt32
		Shell32.lib
		 advapi32
              iphlpapi
              psapi
              shell32
              user32
              userenv
              ws2_32
			  mswsock.lib
			  #  webrtc  timeGetTime  
			  winmm.lib
			  #ucrt.lib
			  #  libuv.lib  µÄÒÀÀµ¿â
			  dbghelp.lib

			  libmedia_transfer_protocol

			  libcrypto
		libssl
				 


				###########################

				 
				 #AAC
				 faad
				 opus
				 libsrtp2
				 x264
		 
		 crypt32.lib
		iphlpapi.lib
       secur32.lib
		 

		libice
		libp2p_peerconnection 

		libmedia_transfer_protocol
		libsrtp2
		libmedia_codec
		#libx264.lib
		x264

		
		#libx264.lib
		#libx264d.lib
	#libx264.dll.lib
	#libx264.lib

	 


	d3d11.lib
      dxgi.lib
	jpeg-static
		

		#curl 
		jsoncpp_static
	       dmoguids.lib
          wmcodecdspuuid.lib
        amstrmid.lib
         msdmo.lib
        oleaut32.lib
		opus
		
libcross_platform_collection_render
	 
		ole32.lib
        oleaut32.lib
        strmiids.lib
        user32.lib
		 
		abseil_dll
		absl_cordz_functions
		absl_cordz_handle
		absl_cordz_info
		absl_cordz_sample_token
		absl_flags
		absl_flags_commandlineflag_internal
		absl_flags_internal
		absl_flags_marshalling
		absl_flags_parse
		absl_flags_private_handle_accessor
		absl_flags_program_name
		absl_flags_reflection
		absl_flags_usage
		absl_flags_usage_internal
		absl_low_level_hash
		absl_random_internal_distribution_test_util
		jpeg-static
		

		#curl 
		jsoncpp_static
		#libcurl
		# curl 
		Ws2_32.lib
		Wldap32.lib
		# api
		 

		d3d11.lib
      dxgi.lib
		
 

			Imm32 
			comctl32
			ws2_32.lib
			winmm.lib


			libyuv
		libcrypto.lib 
		libssl.lib 
		#
		##libredis.lib
		#libzmalloc.lib
		iphlpapi.lib
		#crypto32.lib 
		ws2_32   
		crypt32
		Shell32.lib
		 advapi32
              iphlpapi
              psapi
              shell32
              user32
              userenv
              ws2_32
			  mswsock.lib
			  #  webrtc  timeGetTime  
			  winmm.lib
			  #ucrt.lib
			  #  libuv.lib  µÄÒÀÀµ¿â
			  dbghelp.lib

		 

		libice
		libp2p_peerconnection 

		libmedia_transfer_protocol
		libsrtp2
		libmedia_codec
		#libx264.lib
		x264

		
		#libx264.lib
		#libx264d.lib
	#libx264.dll.lib
	#libx264.lib

 			
	



	       dmoguids.lib
          wmcodecdspuuid.lib
        amstrmid.lib
         msdmo.lib
        oleaut32.lib
		opus
		 
		libvpx.lib
		libvpx_asm.lib

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