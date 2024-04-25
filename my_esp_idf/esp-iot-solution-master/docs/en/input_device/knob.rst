Knob
========

:link_to_translation:`en:[English]`

`Knob` is the component that provides the software PCNT, it can be used on chips(esp32c2, esp32c3) that do not have PCNT hardware capabilities. By using knob you can quickly use a physical encoder, such as the EC11 encoder.

Applicable Scenarios
---------------------

This is suitable for low-speed rotary knob counting scenarios where the pulse rate is less than 30 pulses per second, such as the EC11 encoder. It is suitable for scenarios where 100% accuracy of pulse counting is not required.

.. Note:: For precise or fast pulse counting, please use the `hardware PCNT function <https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/pcnt.html?highlight=pcnt>`_. The hardware PCNT is supported by ESP32, ESP32-C6, ESP32-H2, ESP32-S2, ESP32-S3 chips.

Knob Event
-----------

Each knob has the 5 events in the following table.

+------------+---------------------------------+
|   Events   |       Trigger Conditions        |
+============+=================================+
| KNOB_LEFT  | Left                            |
+------------+---------------------------------+
| KNOB_RIGHT | Right                           |
+------------+---------------------------------+
| KNOB_H_LIM | Count reaches maximum limit     |
+------------+---------------------------------+
| KNOB_L_LIM | Count reaches the minimum limit |
+------------+---------------------------------+
| KNOB_ZERO  | Count back to 0                 |
+------------+---------------------------------+

Each knob can have **Callback** usage.

- Callbacks: Each event of a knob can have a callback function registered for it, and the callback function will be called when the event is generated. This approach is efficient and real-time, and no events are lost.

.. attention:: No blocking operations such as TaskDelay in the callback function

Configuration items
--------------------

- KNOB_PERIOD_TIME_MS : Scan cycle

- KNOB_DEBOUNCE_TICKS : Number of de-shaking

- KNOB_HIGH_LIMIT : The highest number that can be counted by the knob

- KNOB_LOW_LIMIT : The lowest number that can be counted by the knob

Application Examples
---------------------

Create Knob
^^^^^^^^^^^^
.. code:: c

    // create knob
    knob_config_t cfg = {
        .default_direction =0,
        .gpio_encoder_a = GPIO_KNOB_A,
        .gpio_encoder_b = GPIO_KNOB_B,
    };
    s_knob = iot_knob_create(&cfg);
    if(NULL == s_knob) {
        ESP_LOGE(TAG, "knob create failed");
    }

Register callback function
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: c

    static void _knob_left_cb(void *arg, void *data)
    {
        ESP_LOGI(TAG, "KONB: KONB_LEFT,count_value:%"PRId32"",iot_knob_get_count_value((button_handle_t)arg));
    }
    iot_knob_register_cb(s_knob, KNOB_LEFT, _knob_left_cb, NULL);

API Reference
-----------------

.. include-build-file:: inc/iot_knob.inc
