/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "unity.h"
#include "unity_test_runner.h"
#include "unity_test_utils_memory.h"
#include "esp_heap_caps.h"

// Some resources are lazy allocated in GPTimer driver, the threshold is left for that case
#define TEST_MEMORY_LEAK_THRESHOLD (-300)

static size_t before_free_8bit;
static size_t before_free_32bit;

void setUp(void)
{
    before_free_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    before_free_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
}

void tearDown(void)
{
    size_t after_free_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t after_free_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    unity_utils_check_leak(before_free_8bit, after_free_8bit, "8BIT", TEST_MEMORY_LEAK_THRESHOLD);
    unity_utils_check_leak(before_free_32bit, after_free_32bit, "32BIT", TEST_MEMORY_LEAK_THRESHOLD);
}

void app_main(void)
{

    //   ___ _    ___   ___   _____ ___ ___ _____
    //  | _ ) |  |   \ / __| |_   _| __/ __|_   _|
    //  | _ \ |__| |) | (__    | | | _|\__ \ | |
    //  |___/____|___/ \___|   |_| |___|___/ |_|
    //
    printf("  ___ _    ___   ___   _____ ___ ___ _____ \n");
    printf(" | _ ) |  |   \\ / __| |_   _| __/ __|_   _|\n");
    printf(" | _ \\ |__| |) | (__    | | | _|\\__ \\ | |  \n");
    printf(" |___/____|___/ \\___|   |_| |___|___/ |_|  \n");
    unity_run_menu();
}
