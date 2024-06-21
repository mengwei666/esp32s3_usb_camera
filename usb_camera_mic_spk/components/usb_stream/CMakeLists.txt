idf_component_register(SRCS "usb_stream.c" "descriptor.c" "usb_host_helpers.c"
                    INCLUDE_DIRS "include" "${IDF_PATH}/components/usb/include" "esp_camera"
                    PRIV_INCLUDE_DIRS "${IDF_PATH}/components/usb/private_include" "private_include"
                    REQUIRES usb esp_ringbuf)

                    # idf_component_register(SRCS main.c
                    # INCLUDE_DIRS "." "esp_camera")

# 定义版本宏
target_compile_definitions(${COMPONENT_LIB} PRIVATE 
USB_STREAM_VER_MAJOR=1
USB_STREAM_VER_MINOR=3
USB_STREAM_VER_PATCH=2)

# include(package_manager)
# cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})