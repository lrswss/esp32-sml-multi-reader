# Read and publish data from multiple smart meters with an ESP32 microcontroller

## Motivation

As part of a small project, data from several smart meters in a large meter cabinet (all
equipped with a [SML](https://wiki.volkszaehler.org/software/sml#geraete_mit_sml-schnittstelle)
IR interface) had to be read more or less simultaneously and published via MQTT. To avoid
installing an [ESP8266 with Tasmota](https://tasmota.github.io/docs/Smart-Meter-Interface/) 
and IR reading head for each electricity meter, this firmware was developed for an ESP32
microcontroller that can read SML messages from up to eight smart meters simultaneously
using RTOS tasks.

## Features

- based on [sml_parser](https://github.com/olliiiver/sml_parser) library
- read data from up to 6 smart meters simultaneously
- publish readings with timestamp to MQTT broker
- optional MQTT authentication
- TLS support

## Hardware components

- ESP32 development board (e.g. Wemos LolinD32)
- multiple [SML IR reading heads](https://wiki.volkszaehler.org/hardware/controllers/ir-schreib-lesekopf)
- 3D printed [IR reading head housing](https://www.thingiverse.com/thing:3378332) with 27mm neodymium ring magnet
- 5V USB power supply

## Upload firmware

To compile the firmware for the ESP32 controller just download [Visual
Studio Code](https://code.visualstudio.com/) and install the [PlatformIO
add-on](https://platformio.org/install/ide?install=vscode). Open the project
directory and adjust the settings in `include/config.h` to your needs.

Since the software was developed on a MacBook you might need to adjust the [upload port
settings](https://docs.platformio.org/en/stable/projectconf/section_env_upload.html)
in the section `[common]` in `platformio.ini`. For further firmware updates
use the OTA option in the web interface.

## Contributing

Pull requests are welcome! For major changes, please open an issue first to discuss
what you would like to change.

## License

Copyright (c) 2023 Lars Wessels  
This software was published under the Apache License 2.0.  
Please check the [license file](LICENSE).
