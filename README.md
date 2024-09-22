## ESP32 IDF version 5.1.4
## Author: M G Vybhav Nag
## Email: vybhav@craftpower.cc

| Supported Targets | ESP32 | ESP32-S2 |
| ----------------- | ----- | -------- |

# _AUDIO_WS_CLIENT_
This is a simple ESP32 S2 Audio Websocket client project with the following features
1. Connect to Wifi - Wifi credentials can be changed at the beginning of main.c
2. Connect to Websocker server as a client and maintain continuous connection - Server URL can be changed in the beginning of main.c
3. Configure the Audio driver for I2S peripherals at the beginning of main.c
4. Record Audio from Mic and send it to the server in a stream of 20 seconds, then provide a break
5. If any data is received from the server, verify if it is Audio Binary data and play it on Speaker

## FAQ / Errors Solution
1. Mic is not working? - Make sure the Driver Bit rate, Sample Rate, Audio Channel, and Audio communication format are correct, and the Pin is properly connected and configured
2. Speaker is not working? - Make sure the Driver Bit rate, Sample Rate, Audio Channel, and Audio communication format are correct, and the Pin is properly connected and configured
3. Mic and Speaker are not working simultaneously - Make sure both devices have the same I2S configuration [Hardware issue]
4. WebSocket connect fail - make sure TLS is enabled in "menuconfig" and allow "potentially insecure options" and "Skip server certificate verification by default (WARNING: ONLY FOR TESTING PURPOSE, READ HELP)"
5. Device crashing on receiving audio from server / not playing audio - make sure audio sent is in 2048 sample chunks and audio format is same as supported by the speaker and I2S driver.


## How to use this
We encourage the users to use the example as a template for the new projects.
- License: It cannot be used for commercial purposes but can be used as wished for personal and educational use. - Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0).
- Contributors: M G Vybhav Nag
- Attribution: to Craft Power [URL: craftpower.cc], M G Vybhav

Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0).
Copyright © [2024] [M G Vybhav Nag] [craftpower.cc]
