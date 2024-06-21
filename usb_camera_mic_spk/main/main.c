#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "usb_stream.h"

// #include "app_wifi.h"
// #include "app_httpd.h"
#include "esp_camera.h"

/****************** configure the example working mode *******************************/
#define ENABLE_UVC_CAMERA_FUNCTION        1        /* enable uvc function */
#define ENABLE_UAC_MIC_SPK_FUNCTION       0        /* enable uac mic+spk function */
#define ENABLE_UVC_FRAME_RESOLUTION_ANY   1        /* Using any resolution found from the camera */
#define ENABLE_UVC_WIFI_XFER              1        /* transfer uvc frame to wifi http */

#define BIT0_FRAME_START     (0x01 << 0)
#define BIT1_NEW_FRAME_START (0x01 << 1)
#define BIT2_NEW_FRAME_END   (0x01 << 2)
#define BIT3_SPK_START       (0x01 << 3)
#define BIT4_SPK_RESET       (0x01 << 4)

#define DEMO_UVC_FRAME_WIDTH        FRAME_RESOLUTION_ANY
#define DEMO_UVC_FRAME_HEIGHT       FRAME_RESOLUTION_ANY
#define DEMO_UVC_XFER_BUFFER_SIZE (55 * 1024)

static const char *TAG = "uvc_mic_spk_demo";
static EventGroupHandle_t s_evt_handle;
static camera_fb_t s_fb = {0};

camera_fb_t *esp_camera_fb_get()
{
	xEventGroupSetBits(s_evt_handle, BIT0_FRAME_START);
	xEventGroupWaitBits(s_evt_handle, BIT1_NEW_FRAME_START, true, true, portMAX_DELAY);
	return &s_fb;
}

void esp_camera_fb_return(camera_fb_t *fb)
{
	xEventGroupSetBits(s_evt_handle, BIT2_NEW_FRAME_END);
	return;
}

static void camera_frame_cb(uvc_frame_t *frame, void *ptr)
{
	ESP_LOGI(TAG, "uvc callback! frame_format = %d, seq = %"PRIu32", width = %"PRIu32", height = %"PRIu32", length = %u, ptr = %d",
						frame->frame_format, frame->sequence, frame->width, frame->height, frame->data_bytes, (int) ptr);

	if (!(xEventGroupGetBits(s_evt_handle) & BIT0_FRAME_START)) 
	{
		return;
	}

	switch (frame->frame_format) 
	{
		case UVC_FRAME_FORMAT_MJPEG:
			s_fb.buf = frame->data;
			s_fb.len = frame->data_bytes;
			s_fb.width = frame->width;
			s_fb.height = frame->height;
			s_fb.buf = frame->data;
			s_fb.format = PIXFORMAT_JPEG;
			s_fb.timestamp.tv_sec = frame->sequence;
			xEventGroupSetBits(s_evt_handle, BIT1_NEW_FRAME_START);
			ESP_LOGV(TAG, "send frame = %"PRIu32"", frame->sequence);
			xEventGroupWaitBits(s_evt_handle, BIT2_NEW_FRAME_END, true, true, portMAX_DELAY);
			ESP_LOGV(TAG, "send frame done = %"PRIu32"", frame->sequence);
			break;
		default:
			ESP_LOGW(TAG, "Format not supported");
			assert(0);
			break;
	}
}

static void stream_state_changed_cb(usb_stream_state_t event, void *arg)
{
	switch (event) 
	{
		case STREAM_CONNECTED: 
		{
			size_t frame_size = 0;
			size_t frame_index = 0;
			uvc_frame_size_list_get(NULL, &frame_size, &frame_index);
			if (frame_size) 
			{
				ESP_LOGI(TAG, "UVC: get frame list size = %u, current = %u", frame_size, frame_index);
				uvc_frame_size_t *uvc_frame_list = (uvc_frame_size_t *)malloc(frame_size * sizeof(uvc_frame_size_t));
				uvc_frame_size_list_get(uvc_frame_list, NULL, NULL);
				for (size_t i = 0; i < frame_size; i++) 
				{
					ESP_LOGI(TAG, "\tframe[%u] = %ux%u", i, uvc_frame_list[i].width, uvc_frame_list[i].height);
				}
				free(uvc_frame_list);
			} 
			else 
			{
				ESP_LOGW(TAG, "UVC: get frame list size = %u", frame_size);
			}
			ESP_LOGI(TAG, "Device connected");
			break;
		}
		case STREAM_DISCONNECTED:
			ESP_LOGI(TAG, "Device disconnected");
			break;
		default:
			ESP_LOGE(TAG, "Unknown event");
			break;
	}
}

void app_main(void)
{
	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set("httpd_txrx", ESP_LOG_INFO);

	esp_err_t ret = ESP_FAIL; //初始化为一个表示操作失败的错误码

	s_evt_handle = xEventGroupCreate(); //创建一个事件组
	if (s_evt_handle == NULL) 
	{
		// 处理创建失败的情况，例如内存不足
		ESP_LOGE(TAG, "line-%u event group create failed", __LINE__); //__LINE__ 是一个预定义的宏，它的值是当前源代码行号
		assert(0); //导致程序终止并打印一条错误信息
	}

	uint8_t *xfer_buffer_a = (uint8_t *)malloc(DEMO_UVC_XFER_BUFFER_SIZE);
	assert(xfer_buffer_a != NULL);
	uint8_t *xfer_buffer_b = (uint8_t *)malloc(DEMO_UVC_XFER_BUFFER_SIZE);
	assert(xfer_buffer_b != NULL);
	uint8_t *frame_buffer = (uint8_t *)malloc(DEMO_UVC_XFER_BUFFER_SIZE);
	assert(frame_buffer != NULL);

	uvc_config_t uvc_config = {
		.frame_width = DEMO_UVC_FRAME_WIDTH,
		.frame_height = DEMO_UVC_FRAME_HEIGHT,
		.frame_interval = FPS2INTERVAL(15),
		.xfer_buffer_size = DEMO_UVC_XFER_BUFFER_SIZE,
		.xfer_buffer_a = xfer_buffer_a,
		.xfer_buffer_b = xfer_buffer_b,
		.frame_buffer_size = DEMO_UVC_XFER_BUFFER_SIZE,
		.frame_buffer = frame_buffer,
		.frame_cb = &camera_frame_cb,
		.frame_cb_arg = NULL,
	};
	ret = uvc_streaming_config(&uvc_config);
	if (ret != ESP_OK) 
	{
		ESP_LOGE(TAG, "uvc streaming config failed");
	}
	ESP_LOGI(TAG,"--------------------------#1-------------------------");
	// printf("--------------------------#1-------------------------");


	//ESP_ERROR_CHECK 是一个宏，用于简化错误检查和处理。它会检查传入的函数的返回值是否表示错误，如果是错误，它会打印错误信息并中止程序
	ESP_ERROR_CHECK(usb_streaming_state_register(&stream_state_changed_cb, NULL)); //注册一个回调函数 stream_state_changed_cb，用于处理 USB 流媒体状态的变化
	ESP_LOGI(TAG,"--------------------------#2.1-------------------------");
	ESP_ERROR_CHECK(usb_streaming_start()); //启动 USB 流媒体
	ESP_LOGI(TAG,"--------------------------#2.2-------------------------");
	ESP_ERROR_CHECK(usb_streaming_connect_wait(portMAX_DELAY)); //等待 USB 设备连接完成。在调用这个函数后，程序会阻塞，直到 USB 设备成功连接或超时
	xEventGroupWaitBits(s_evt_handle, BIT3_SPK_START, false, false, portMAX_DELAY); //等待事件组中的特定位（BIT3_SPK_START）被设置。这个调用会阻塞，直到 BIT3_SPK_START 位被设置。
	ESP_LOGI(TAG, "Event BIT3_SPK_START set, continuing execution");
	ESP_LOGI(TAG,"--------------------------#2-------------------------");

}
