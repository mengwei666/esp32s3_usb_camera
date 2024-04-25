/*
 * SPDX-FileCopyrightText: 2019-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "ble_ota.h"

#define OTA_RINGBUF_SIZE                    8192
#define OTA_TASK_SIZE                       8192

static const char *TAG = "ESP_BLE_OTA";

#if CONFIG_EXAMPLE_USE_PRE_ENC_OTA
extern const char rsa_private_pem_start[] asm("_binary_private_pem_start");
extern const char rsa_private_pem_end[]   asm("_binary_private_pem_end");
esp_decrypt_handle_t decrypt_handle;
#endif

#ifdef CONFIG_EXAMPLE_USE_PROTOCOMM
#include "manager.h"
#include "scheme_ble.h"
#include "esp_netif.h"

#if CONFIG_EXAMPLE_OTA_SEC2_DEV_MODE
static const char sec2_salt[] = {
    0x03, 0x6e, 0xe0, 0xc7, 0xbc, 0xb9, 0xed, 0xa8, 0x4c, 0x9e, 0xac, 0x97, 0xd9, 0x3d, 0xec, 0xf4
};

static const char sec2_verifier[] = {
    0x7c, 0x7c, 0x85, 0x47, 0x65, 0x08, 0x94, 0x6d, 0xd6, 0x36, 0xaf, 0x37, 0xd7, 0xe8, 0x91, 0x43,
    0x78, 0xcf, 0xfd, 0x61, 0x6c, 0x59, 0xd2, 0xf8, 0x39, 0x08, 0x12, 0x72, 0x38, 0xde, 0x9e, 0x24,
    0xa4, 0x70, 0x26, 0x1c, 0xdf, 0xa9, 0x03, 0xc2, 0xb2, 0x70, 0xe7, 0xb1, 0x32, 0x24, 0xda, 0x11,
    0x1d, 0x97, 0x18, 0xdc, 0x60, 0x72, 0x08, 0xcc, 0x9a, 0xc9, 0x0c, 0x48, 0x27, 0xe2, 0xae, 0x89,
    0xaa, 0x16, 0x25, 0xb8, 0x04, 0xd2, 0x1a, 0x9b, 0x3a, 0x8f, 0x37, 0xf6, 0xe4, 0x3a, 0x71, 0x2e,
    0xe1, 0x27, 0x86, 0x6e, 0xad, 0xce, 0x28, 0xff, 0x54, 0x46, 0x60, 0x1f, 0xb9, 0x96, 0x87, 0xdc,
    0x57, 0x40, 0xa7, 0xd4, 0x6c, 0xc9, 0x77, 0x54, 0xdc, 0x16, 0x82, 0xf0, 0xed, 0x35, 0x6a, 0xc4,
    0x70, 0xad, 0x3d, 0x90, 0xb5, 0x81, 0x94, 0x70, 0xd7, 0xbc, 0x65, 0xb2, 0xd5, 0x18, 0xe0, 0x2e,
    0xc3, 0xa5, 0xf9, 0x68, 0xdd, 0x64, 0x7b, 0xb8, 0xb7, 0x3c, 0x9c, 0xfc, 0x00, 0xd8, 0x71, 0x7e,
    0xb7, 0x9a, 0x7c, 0xb1, 0xb7, 0xc2, 0xc3, 0x18, 0x34, 0x29, 0x32, 0x43, 0x3e, 0x00, 0x99, 0xe9,
    0x82, 0x94, 0xe3, 0xd8, 0x2a, 0xb0, 0x96, 0x29, 0xb7, 0xdf, 0x0e, 0x5f, 0x08, 0x33, 0x40, 0x76,
    0x52, 0x91, 0x32, 0x00, 0x9f, 0x97, 0x2c, 0x89, 0x6c, 0x39, 0x1e, 0xc8, 0x28, 0x05, 0x44, 0x17,
    0x3f, 0x68, 0x02, 0x8a, 0x9f, 0x44, 0x61, 0xd1, 0xf5, 0xa1, 0x7e, 0x5a, 0x70, 0xd2, 0xc7, 0x23,
    0x81, 0xcb, 0x38, 0x68, 0xe4, 0x2c, 0x20, 0xbc, 0x40, 0x57, 0x76, 0x17, 0xbd, 0x08, 0xb8, 0x96,
    0xbc, 0x26, 0xeb, 0x32, 0x46, 0x69, 0x35, 0x05, 0x8c, 0x15, 0x70, 0xd9, 0x1b, 0xe9, 0xbe, 0xcc,
    0xa9, 0x38, 0xa6, 0x67, 0xf0, 0xad, 0x50, 0x13, 0x19, 0x72, 0x64, 0xbf, 0x52, 0xc2, 0x34, 0xe2,
    0x1b, 0x11, 0x79, 0x74, 0x72, 0xbd, 0x34, 0x5b, 0xb1, 0xe2, 0xfd, 0x66, 0x73, 0xfe, 0x71, 0x64,
    0x74, 0xd0, 0x4e, 0xbc, 0x51, 0x24, 0x19, 0x40, 0x87, 0x0e, 0x92, 0x40, 0xe6, 0x21, 0xe7, 0x2d,
    0x4e, 0x37, 0x76, 0x2f, 0x2e, 0xe2, 0x68, 0xc7, 0x89, 0xe8, 0x32, 0x13, 0x42, 0x06, 0x84, 0x84,
    0x53, 0x4a, 0xb3, 0x0c, 0x1b, 0x4c, 0x8d, 0x1c, 0x51, 0x97, 0x19, 0xab, 0xae, 0x77, 0xff, 0xdb,
    0xec, 0xf0, 0x10, 0x95, 0x34, 0x33, 0x6b, 0xcb, 0x3e, 0x84, 0x0f, 0xb9, 0xd8, 0x5f, 0xb8, 0xa0,
    0xb8, 0x55, 0x53, 0x3e, 0x70, 0xf7, 0x18, 0xf5, 0xce, 0x7b, 0x4e, 0xbf, 0x27, 0xce, 0xce, 0xa8,
    0xb3, 0xbe, 0x40, 0xc5, 0xc5, 0x32, 0x29, 0x3e, 0x71, 0x64, 0x9e, 0xde, 0x8c, 0xf6, 0x75, 0xa1,
    0xe6, 0xf6, 0x53, 0xc8, 0x31, 0xa8, 0x78, 0xde, 0x50, 0x40, 0xf7, 0x62, 0xde, 0x36, 0xb2, 0xba
};
#endif

static esp_err_t
example_get_sec2_salt(const char **salt, uint16_t *salt_len)
{
#if CONFIG_EXAMPLE_OTA_SEC2_DEV_MODE
    ESP_LOGI(TAG, "Development mode: using hard coded salt");
    *salt = sec2_salt;
    *salt_len = sizeof(sec2_salt);
    return ESP_OK;
#elif CONFIG_EXAMPLE_OTA_SEC2_PROD_MODE
    ESP_LOGE(TAG, "Not implemented!");
    return ESP_FAIL;
#endif
    return ESP_FAIL;
}

static esp_err_t
example_get_sec2_verifier(const char **verifier, uint16_t *verifier_len)
{
#if CONFIG_EXAMPLE_OTA_SEC2_DEV_MODE
    ESP_LOGI(TAG, "Development mode: using hard coded verifier");
    *verifier = sec2_verifier;
    *verifier_len = sizeof(sec2_verifier);
    return ESP_OK;
#elif CONFIG_EXAMPLE_OTA_SEC2_PROD_MODE
    /* This code needs to be updated with appropriate implementation to provide verifier */
    ESP_LOGE(TAG, "Not implemented!");
    return ESP_FAIL;
#endif
    return ESP_FAIL;
}

/* Event handler for catching system events */
static void
event_handler(void *arg, esp_event_base_t event_base,
              int32_t event_id, void *event_data)
{
    if (event_base == ESP_BLE_OTA_EVENT) {
        switch (event_id) {
        case OTA_FILE_RCV:
            ESP_LOGD(TAG, "File received in appln layer :");
            ota_recv_fw_cb(event_data, 4096);
            break;
        default:
            break;
        }
    } else if (event_base == PROTOCOMM_TRANSPORT_BLE_EVENT) {
        switch (event_id) {
        case PROTOCOMM_TRANSPORT_BLE_CONNECTED:
            ESP_LOGI(TAG, "BLE transport: Connected!");
            break;
        case PROTOCOMM_TRANSPORT_BLE_DISCONNECTED:
            ESP_LOGI(TAG, "BLE transport: Disconnected!");
            break;
        default:
            break;
        }
    }
}

static void
get_device_service_name(char *service_name, size_t size)
{
    char *svc_name = "OTA_123456";
    strlcpy(service_name, svc_name, size);
}

#endif /* CONFIG_EXAMPLE_USE_PROTOCOMM */

static RingbufHandle_t s_ringbuf = NULL;

bool
ble_ota_ringbuf_init(uint32_t ringbuf_size)
{
    s_ringbuf = xRingbufferCreate(ringbuf_size, RINGBUF_TYPE_BYTEBUF);
    if (s_ringbuf == NULL) {
        return false;
    }

    return true;
}

size_t
write_to_ringbuf(const uint8_t *data, size_t size)
{
    BaseType_t done = xRingbufferSend(s_ringbuf, (void *)data, size, (TickType_t)portMAX_DELAY);
    if (done) {
        return size;
    } else {
        return 0;
    }
}

void
ota_task(void *arg)
{
    esp_partition_t *partition_ptr = NULL;
    esp_partition_t partition;
    const esp_partition_t *next_partition = NULL;
    esp_ota_handle_t out_handle = 0;

    uint32_t recv_len = 0;
    uint8_t *data = NULL;
    size_t item_size = 0;
    ESP_LOGI(TAG, "ota_task start");
    // search ota partition
    partition_ptr = (esp_partition_t *)esp_ota_get_boot_partition();
    if (partition_ptr == NULL) {
        ESP_LOGE(TAG, "boot partition NULL!\r\n");
        goto OTA_ERROR;
    }
    if (partition_ptr->type != ESP_PARTITION_TYPE_APP) {
        ESP_LOGE(TAG, "esp_current_partition->type != ESP_PARTITION_TYPE_APP\r\n");
        goto OTA_ERROR;
    }

    if (partition_ptr->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY) {
        partition.subtype = ESP_PARTITION_SUBTYPE_APP_OTA_0;
    } else {
        next_partition = esp_ota_get_next_update_partition(partition_ptr);
        if (next_partition) {
            partition.subtype = next_partition->subtype;
        } else {
            partition.subtype = ESP_PARTITION_SUBTYPE_APP_OTA_0;
        }
    }
    partition.type = ESP_PARTITION_TYPE_APP;

    partition_ptr = (esp_partition_t *)esp_partition_find_first(partition.type, partition.subtype, NULL);
    if (partition_ptr == NULL) {
        ESP_LOGE(TAG, "partition NULL!\r\n");
        goto OTA_ERROR;
    }

    memcpy(&partition, partition_ptr, sizeof(esp_partition_t));
    if (esp_ota_begin(&partition, OTA_SIZE_UNKNOWN, &out_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed!\r\n");
        goto OTA_ERROR;
    }

    ESP_LOGI(TAG, "wait for data from ringbuf! fw_len = %u", esp_ble_ota_get_fw_length());
    /*deal with all receive packet*/
    for (;;) {
        data = (uint8_t *)xRingbufferReceive(s_ringbuf, &item_size, (TickType_t)portMAX_DELAY);

        ESP_LOGI(TAG, "recv: %u, recv_total:%"PRIu32"\n", item_size, recv_len + item_size);
        if (item_size != 0) {
            if (esp_ota_write(out_handle, (const void *)data, item_size) != ESP_OK) {
                ESP_LOGE(TAG, "esp_ota_write failed!\r\n");
                goto OTA_ERROR;
            }
            recv_len += item_size;
            vRingbufferReturnItem(s_ringbuf, (void *)data);

            if (recv_len >= esp_ble_ota_get_fw_length()) {
                break;
            }
        }
    }

    if (esp_ota_end(out_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!\r\n");
        goto OTA_ERROR;
    }

    if (esp_ota_set_boot_partition(&partition) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed!\r\n");
        goto OTA_ERROR;
    }

    esp_restart();

OTA_ERROR:
    ESP_LOGE(TAG, "OTA failed");
    vTaskDelete(NULL);
}

void
ota_recv_fw_cb(uint8_t *buf, uint32_t length)
{
    write_to_ringbuf(buf, length);
}

static void
ota_task_init(void)
{
    xTaskCreate(&ota_task, "ota_task", OTA_TASK_SIZE, NULL, 5, NULL);
    return;
}

void
app_main(void)
{
    esp_err_t ret;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (!ble_ota_ringbuf_init(OTA_RINGBUF_SIZE)) {
        ESP_LOGE(TAG, "%s init ringbuf fail", __func__);
        return;
    }

#if CONFIG_EXAMPLE_USE_PROTOCOMM
    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(ESP_BLE_OTA_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(PROTOCOMM_TRANSPORT_BLE_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    /* Configuration for the ota manager */
    esp_ble_ota_config_t config = {
        .scheme = esp_ble_ota_scheme_ble,

        .scheme_event_handler = ESP_BLE_OTA_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    /* Initialize ota manager with the
     * configuration parameters set above */
    ESP_ERROR_CHECK(esp_ble_ota_init(config));

    char service_name[12];
    get_device_service_name(service_name, sizeof(service_name));
    esp_ble_ota_security_t security = ESP_BLE_OTA_SECURITY_2;

#ifdef CONFIG_EXAMPLE_OTA_SECURITY_VERSION_1
    security = ESP_BLE_OTA_SECURITY_1;

    /* Do we want a proof-of-possession (ignored if Security 0 is selected):
     *      - this should be a string with length > 0
     *      - NULL if not used
     */
    const char *pop = "abcd1234";

    /* This is the structure for passing security parameters
     * for the protocomm security 1.
     */
    esp_ble_ota_security1_params_t *sec_params = pop;

#elif CONFIG_EXAMPLE_OTA_SECURITY_VERSION_2
    /* The username must be the same one, which has been used in the generation of salt and verifier */

    esp_ble_ota_security2_params_t sec2_params = {};

    ESP_ERROR_CHECK(example_get_sec2_salt(&sec2_params.salt, &sec2_params.salt_len));
    ESP_ERROR_CHECK(example_get_sec2_verifier(&sec2_params.verifier, &sec2_params.verifier_len));

    esp_ble_ota_security2_params_t *sec_params = &sec2_params;
#endif
    const char *service_key = NULL;

    ESP_ERROR_CHECK(esp_ble_ota_start(security, (const void *) sec_params, service_name, service_key));
#else
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if (esp_ble_ota_host_init() != ESP_OK) {
        ESP_LOGE(TAG, "%s initialoze ble host fail: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

#if CONFIG_EXAMPLE_USE_PRE_ENC_OTA
    esp_decrypt_cfg_t cfg = {};
    cfg.rsa_pub_key = rsa_private_pem_start;
    cfg.rsa_pub_key_len = rsa_private_pem_end - rsa_private_pem_start;
    decrypt_handle = esp_encrypted_img_decrypt_start(&cfg);
    if (!decrypt_handle) {
        ESP_LOGE(TAG, "OTA upgrade failed");
        vTaskDelete(NULL);
    }

    esp_ble_ota_recv_fw_data_callback(ota_recv_fw_cb, decrypt_handle);
#else
    esp_ble_ota_recv_fw_data_callback(ota_recv_fw_cb);
#endif /* CONFIG_EXAMPLE_USE_PRE_ENC_OTA */

#endif /* CONFIG_EXAMPLE_USE_PROTOCOMM */
    ota_task_init();
}
