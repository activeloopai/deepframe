include(ExternalProject)  

set(DEFAULT_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(ffmpeg_BUILD_INSTALL_PREFIX ${DEFAULT_PARENT_DIR}/.ext)
set(ffmpeg_URL "https://github.com/FFmpeg/FFmpeg/archive/refs/tags/n7.1.tar.gz")
set(ffmpeg_SOURCE_DIR ${DEFAULT_PARENT_DIR}/.ext/ffmpeg)
set(ffmpeg_SHA256_HASH 7ddad2d992bd250a6c56053c26029f7e728bebf0f37f80cf3f8a0e6ec706431a)
set(ffmpeg_INSTALL_DIR "${DEFAULT_PARENT_DIR}/.ext")

set(ffmpeg_PKG_CONFIG_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/pkgconfig:${DEFAULT_PARENT_DIR}/.ext/lib/pkgconfig")

macro(check_lib_existence LIB_NAME)
    find_path(${LIB_NAME}_EXISTS
        NAMES ${LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
        PATHS ${ffmpeg_INSTALL_DIR}/lib
        NO_DEFAULT_PATH
    )
endmacro()

set(FFMPEG_LIBRARIES_LIST libavformat libavcodec libavfilter libavutil libswresample libswscale)

macro(build_ffmpeg_once)
    set(ffmpeg_NEED_BUILD 0)
    if (NOT EXISTS ${ffmpeg_INSTALL_DIR}/lib)
        set(ffmpeg_NEED_BUILD 1)
    endif()
    foreach(LIB_NAME ${FFMPEG_LIBRARIES_LIST})
        check_lib_existence(${LIB_NAME})
        if (${LIB_NAME}_EXISTS STREQUAL ${LIB_NAME}_EXISTS-NOTFOUND)
            set(ffmpeg_NEED_BUILD 1)
        endif()
    endforeach()

    if (NOT TARGET ffmpeg_ep AND ${ffmpeg_NEED_BUILD})
        message(STATUS "Building FFmpeg from source")

        set(EXTRA_ARGUMENTS)
        if(APPLE)
            list(APPEND EXTRA_ARGUMENTS "--target-os=darwin")
            if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL arm64)
                list(APPEND EXTRA_ARGUMENTS "--arch=arm64")
            elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL x86_64)
                list(APPEND EXTRA_ARGUMENTS "--arch=x86_64")
            endif()
        elseif(UNIX)
            list(APPEND EXTRA_ARGUMENTS "--target-os=linux")
            if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL aarch64)
                list(APPEND EXTRA_ARGUMENTS "--arch=arm64")
            elseif(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL x86_64)
                list(APPEND EXTRA_ARGUMENTS "--arch=x86_64")
            endif()
        endif()

        set(FFMPEG_EP_DEPENDS)

        message(STATUS "Building FFmpeg with extra arguments: ${EXTRA_ARGUMENTS}")

        ExternalProject_Add(ffmpeg_ep
            SOURCE_DIR ${ffmpeg_SOURCE_DIR}
            URL ${ffmpeg_URL}
            URL_HASH SHA256=${ffmpeg_SHA256_HASH}
            CONFIGURE_COMMAND
                ${CMAKE_COMMAND} -E env PKG_CONFIG_PATH=${ffmpeg_PKG_CONFIG_PATH} ${ffmpeg_SOURCE_DIR}/configure
                    --quiet
                    --prefix=${ffmpeg_INSTALL_DIR}
                    --disable-programs
                    --disable-logging
                    --disable-shared 
                    --enable-static 
                    --disable-doc
                    --disable-htmlpages
                    --disable-manpages
                    --disable-podpages
                    --disable-txtpages
                    --enable-ffmpeg 
                    --disable-ffplay
                    --disable-ffprobe 
                    --disable-x86asm
                    --disable-avdevice 
                    --enable-avcodec 
                    --enable-avformat 
                    --enable-swscale 
                    --enable-pic
                    --disable-asm
                    --disable-debug
                    --disable-programs
                    --disable-libdrm
                    --enable-openssl
                    ${EXTRA_ARGUMENTS}
            BUILD_IN_SOURCE TRUE
            PATCH_COMMAND 
                patch -p1 < ${DEFAULT_PARENT_DIR}/cmake/modules/url_max_length_fix.patch
            BUILD_COMMAND $(MAKE)
            INSTALL_COMMAND $(MAKE) install
            DEPENDS ${FFMPEG_EP_DEPENDS}
            ${EP_LOG_OPTIONS}
        )
    endif()

endmacro()

build_ffmpeg_once()

macro(ensure_ffmpeg)
    include_directories(${ffmpeg_BUILD_INSTALL_PREFIX}/include)
    set(FFMPEG_LIBRARIES_LIST)
    list(APPEND FFMPEG_LIBRARIES_LIST ${ffmpeg_INSTALL_DIR}/lib/libavformat${CMAKE_STATIC_LIBRARY_SUFFIX})
    list(APPEND FFMPEG_LIBRARIES_LIST ${ffmpeg_INSTALL_DIR}/lib/libavcodec${CMAKE_STATIC_LIBRARY_SUFFIX})
    list(APPEND FFMPEG_LIBRARIES_LIST ${ffmpeg_INSTALL_DIR}/lib/libavfilter${CMAKE_STATIC_LIBRARY_SUFFIX})
    list(APPEND FFMPEG_LIBRARIES_LIST ${ffmpeg_INSTALL_DIR}/lib/libavutil${CMAKE_STATIC_LIBRARY_SUFFIX})
    list(APPEND FFMPEG_LIBRARIES_LIST ${ffmpeg_INSTALL_DIR}/lib/libswresample${CMAKE_STATIC_LIBRARY_SUFFIX})
    list(APPEND FFMPEG_LIBRARIES_LIST ${ffmpeg_INSTALL_DIR}/lib/libswscale${CMAKE_STATIC_LIBRARY_SUFFIX})


    #TODO come up with proper solution here
    add_library(FFmpeg::FFmpeg INTERFACE IMPORTED GLOBAL)
    #target_link_libraries(FFmpeg::FFmpeg INTERFACE
    #    ${ffmpeg_INSTALL_DIR}/lib/libavcodec${CMAKE_STATIC_LIBRARY_SUFFIX}
    #    ${ffmpeg_INSTALL_DIR}/lib/libavformat${CMAKE_STATIC_LIBRARY_SUFFIX}
    #    ${ffmpeg_INSTALL_DIR}/lib/libavfilter${CMAKE_STATIC_LIBRARY_SUFFIX}
    #    ${ffmpeg_INSTALL_DIR}/lib/libavutil${CMAKE_STATIC_LIBRARY_SUFFIX}
    #    ${ffmpeg_INSTALL_DIR}/lib/libswresample${CMAKE_STATIC_LIBRARY_SUFFIX}
    #    ${ffmpeg_INSTALL_DIR}/lib/libswscale${CMAKE_STATIC_LIBRARY_SUFFIX}
    #)

    if (APPLE)
        set_target_properties(FFmpeg::FFmpeg
                      PROPERTIES INTERFACE_LINK_LIBRARIES
                                 "-framework VideoToolbox -framework CoreFoundation -framework CoreMedia -framework CoreVideo -framework CoreServices")
    endif()
endmacro()