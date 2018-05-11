# AIS NB Cellular tower logger

This mini-project is for test using `DEVIO NB-SHIELD I` and AIS NB-IoT network by logging cellular information with the GPS coordinate.

You can read the full article about this mini-project at [Article "รีวิว DEVIO NB-Shield I และ Mini-project: Cellular Tower logger
"](https://www.itpcc.net/tip-and-technic/review-devio-nb-shield-i-and-mini-project-cellular-tower-logger/).

## Device_code

This is a code for programming device to transmite data (including Cellular tower ID and GPS coordinate) to the server.

### Hardware

- Arduino Uno
- [DEVIO NB-SHIELD I](https://www.blognone.com/node/100025)
- [GlobalTop FGPMMOPA6C GPS Standalone Module](https://cdn-shop.adafruit.com/datasheets/GlobalTop-FGPMMOPA6C-Datasheet-V0A-Preliminary.pdf) or GPS Standalone Module with NMEA sentence Serial interface.

#### Pinout

| GPS | Arduino   |
|:---:|:---------:|
| GND | GND       |
| VCC | 5V        |
| TxD | 11 (MOSI) |
| RxD | 12 (MISO) |

For `DEVIO NB-SHIELD I`, you can just stack up with Arduino Uno or Other pin-compatible microcontroller board.

Please avoid using pin `8` and `9`, because they were used by `DEVIO NB-SHIELD I` via [AltSoftSerial](https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html) library to interface with Arduino.

However, if you really want to change `TxD` and `RxD` port, please check `NeoSWSerial` document to check the support pins. And, don't forget pin number in the line #21.

### Setup

- Install Arduino
- Install following library:
	* [AIS_NB_BC95](https://github.com/itpcc/AIS_NB_BC95). (This is forked version from `AIS-DeviceInnovation` to enable get Cellular tower information.)
	* [NeoSWSerial](https://github.com/SlashDevin/NeoSWSerial). (This is for GPS serial interface)
- Change the following variables in `Device_code.ino`:
	* `<UDP_SERVER_IP>` Your UDP Public-accessible server IP address.
	* `<UDP_SERVER_PORT>` Your UDP Public-accessible server port.
- Compile and upload the code to your Arduino Uno.

You can monitor/debug via USB Serial at 9600 baud.

## Server

For a simeple UDP server, you can use `UDPserver.py` as a server script to receive data from the device and save data to Firebase's firestore.

`processFile.py` is for process raw data from Firebase's firestore to processed data for displaying on the map.

### Installation

- Install Python 3.4+
- Run the following command: 
```bash
pip install asyncio firebase_admin
```
- [Get Firebase's firestore credential JSON file.](https://firebase.google.com/docs/firestore/quickstart) and rename to `firebase-credential.json` and place at the same folder that `UDPserver.py` placed.
- If you don't want to use port 8005 as your UDP port, set `PORT` environment variable.

### Usage

#### UDP server
```bash
python UDPServer.py
```

#### Processing to JSON file
```bash
python processFile.py | tee <JSON File name>.json
```

## Display

Just a static file to display a map from the observation. :/

### Installation

- Get Google map API key and change `YOUR_API_KEY` to your API key.
- Place `result.json` (JSON file  processed with `processFile.py`) at the same folder that `processFile.py` live
- Just open `processFile.py`

# Disclaimer

This mini-project is an independent project. Neither funded nor cooperate with ADVANCED INFO SERVICE PLC.
