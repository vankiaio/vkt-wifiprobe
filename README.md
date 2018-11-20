Cloud storage sector is (FLASH_MAX_SECTOR - 12)

The address is:
```
256kBytes( 2Mbits) Flash --- 0x034000
512kBytes( 4Mbits) Flash --- 0x074000
  1MBytes( 8Mbits) Flash --- 0x0F4000
  2MBytes(16Mbits) Flash --- 0x1F4000
  4MBytes(32Mbits) Flash --- 0x3F4000
```
The address with bootloader:
download:
```
Flash size 8Mbit:            512KB+512KB
boot_v1.2+.bin               0x00000
user1.1024.new.2.bin         0x01000
esp_init_data_default.bin    0xfc000 (optional)
blank.bin                    0x7e000 & 0xfe000

Flash size 16Mbit:           512KB+512KB
boot_v1.2+.bin               0x00000
user1.1024.new.2.bin         0x01000
esp_init_data_default.bin    0x1fc000 (optional)
blank.bin                    0x7e000 & 0x1fe000

Flash size 16Mbit-C1:        1024KB+1024KB
boot_v1.2+.bin               0x00000
user1.2048.new.5.bin         0x01000
esp_init_data_default.bin    0x1fc000 (optional)
blank.bin                    0xfe000 & 0x1fe000

Flash size 32Mbit:           512KB+512KB
boot_v1.2+.bin               0x00000
user1.1024.new.2.bin         0x01000
esp_init_data_default.bin    0x3fc000 (optional)
blank.bin                    0x7e000 & 0x3fe000

Flash size 32Mbit-C1:        1024KB+1024KB
boot_v1.2+.bin               0x00000
user1.2048.new.5.bin         0x01000
esp_init_data_default.bin    0x3fc000 (optional)
blank.bin                    0xfe000 & 0x3fe000
```

The address without bootloader
download
```
eagle.flash.bin              0x00000
eagle.irom0text.bin          0x40000
```
blank.bin
```
Flash size 4Mbit:            0x3e000 & 0x7e000
Flash size 8Mbit:            0x7e000 & 0xfe000
Flash size 16Mbit:           0x7e000 & 0x1fe000
Flash size 16Mbit-C1:        0xfe000 & 0x1fe000
Flash size 32Mbit:           0x7e000 & 0x3fe000
Flash size 32Mbit-C1:        0xfe000 & 0x3fe000
```
esp_init_data_default.bin    (optional)	
```
Flash size 4Mbit:            0x7c000
Flash size 8Mbit:            0xfc000
Flash size 16Mbit:           0x1fc000
Flash size 16Mbit-C1:        0x1fc000
Flash size 32Mbit:           0x3fc000
Flash size 32Mbit-C1:        0x3fc000
```

Clean the cloud data storage:
```
make dataclean
```

Clean the Wi-Fi configuration:
```
make wificlean
```

Erase the flash chip:
```
make erase
```

Now this SDK uses 4MBytes(32Mbits) Flash as default, so burn address is:
```
Flash size 32Mbit-C1:        1024KB+1024KB
boot_v1.2+.bin               0x00000
user1.2048.new.5.bin         0x01000
esp_init_data_default.bin    0x3fc000 (optional)
blank.bin                    0xfe000 & 0x3fe000
```
