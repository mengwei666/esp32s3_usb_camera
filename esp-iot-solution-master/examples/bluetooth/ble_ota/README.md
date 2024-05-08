# ESP BLE OTA Demo description

``ble ota demo`` is based on the ``ble ota component``, it receives firmware via BLE and writes it to flash, sector by sector, until the upgrade is complete.

## 1. Services definition

The component contains two services:

- `DIS Service`: Displays software and hardware version information
- `OTA Service`: It is used for OTA upgrade and contains 4 characteristics, as shown in the following table:

|  Characteristics   | UUID  |  Prop   | description  |
|  ----  | ----  |  ----  | ----  |
|  RECV_FW_CHAR | 0x8020 | Write, notify  | Firmware received, send ACK |
|  PROGRESS_BAR_CHAR  | 0x8021 | Read, notify  | Read the progress bar and report the progress bar |
|  COMMAND_CHAR  | 0x8022 | Write, notify  | Send the command and ACK |
|  CUSTOMER_CHAR  | 0x8023 | Write, notify  | User-defined data to send and receive |

## 2. Data transmission

### 2.1 Command package format

|  unit   | Command_ID  |  PayLoad   | CRC16  |
|  ----  | ----  |  ----  | ----  |
|  Byte | Byte: 0 ~ 1 | Byte: 2 ~ 17  | Byte: 18 ~ 19 |

Command_ID:

- 0x0001: Start OTA, Payload bytes(2 to 5), indicates the length of the firmware. Other Payload is set to 0 by default. CRC16 calculates bytes(0 to 17).
- 0x0002: Stop OTA, and the remaining Payload will be set to 0. CRC16 calculates bytes(0 to 17).
- 0x0003: The Payload bytes(2 or 3) is the payload of the Command_ID for which the response will be sent. Payload bytes(4 to 5) is a response to the command. 0x0000 indicates accept, 0x0001 indicates reject. Other payloads are set to 0. CRC16 computes bytes(0 to 17).

### 2.2 Firmware package format

The format of the firmware package sent by the client is as follows:

|  unit   | Sector_Index  |  Packet_Seq   | PayLoad  |
|  ----  | ----  |  ----  | ----  |
|  Byte | Byte: 0 ~ 1 | Byte: 2  | Byte: 3 ~ (MTU_size - 4) |

- Sector_Index：Indicates the number of sectors, sector number increases from 0, cannot jump, must be send 4K data and then start transmit the next sector, otherwise it will immediately send the error ACK for request retransmission.
- Packet_Seq：If Packet_Seq is 0xFF, it indicates that this is the last packet of the sector, and the last 2 bytes of Payload is the CRC16 value of 4K data for the entire sector, the remaining bytes will set to 0x0. Server will check the total length and CRC of the data from the client, reply the correct ACK, and then start receive the next sector of firmware data.

The format of the reply packet is as follows:

|  unit   | Sector_Index  |  ACK_Status   | CRC6  |
|  ----  | ----  |  ----  | ----  |
|  Byte | Byte: 0 ~ 1 | Byte: 2 ~ 3  | Byte: 18 ~ 19 |

ACK_Status:

- 0x0000: Success
- 0x0001: CRC error
- 0x0002: Sector_Index error, bytes(4 ~ 5) indicates the desired Sector_Index
- 0x0003：Payload length error

## 3.  Sample APP

[APP](https://github.com/EspressifApps/esp-ble-ota-android/releases/tag/rc)

## 4. Sample code

[ESP BLE OTA](https://github.com/espressif/esp-iot-solution/tree/master/examples/bluetooth/ble_ota)

### 4.1 OTA update ESP32-H2

To construct the BLE OTA demo for the ESP32-H2 device, ensure you're using ESP-IDF version 5.0 or later. Currently, the ESP32-H2 doesn't support NimBLE OTA updates, so omit the instructions in section 5. Maintain the default configuration for the Bluetooth framework, where `Bluedroid - Dual-mode` is selected. To ensure compatibility with the sample app mentioned in section 3, deactivate BLE 5.0 features by navigating to:

1. `Component config` > `Bluetooth` > `Bluedroid Options` and deselect `Enable BLE 5.0 features`.
2. `Component config` > `Bluetooth` > `Controller Options` and deselect `Enable BLE 5 feature`.

## 5. NimBLE OTA

#### Configure the Project

`idf.py menuconfig`

- Component config → Bluetooth → Bluetooth → Host → NimBLE - BLE only

Note: For maximum throughput, set maximum MTU using

- Component config → Bluetooth → NimBLE Options → Preferred MTU size in octets as 517

### 5.1 OTA with Protocomm

#### Configure the Project

`idf.py menuconfig`

- Example Configuration → Type of OTA → Use protocomm layer for security
- Component config → OTA Manager → Type of OTA → Enable protocomm level security

### 5.2 Pre encrypted OTA

- For using pre encrypted OTA, the OTA file needs to be encrypted using [private key](../ble_ota/rsa_key/private.pem)
- For more info regarding encrypting file refer : [esp_encrypted_img](https://github.com/espressif/idf-extra-components/tree/master/esp_encrypted_img)

#### Configure the Project

`idf.py menuconfig`

- If using ESP32S3 set Component config → Bluetooth → Bluetooth → NimBLE Options → NimBLE Host task stack size as 8192 
- Example Configuration → Type of OTA → Use Pre-Encrypted OTA
- Component config → OTA Manager → Type of OTA → Enable pre encrypted OTA
