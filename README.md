## ESP32 IDF version 5.1.4
## Author: M G Vybhav Nag
## Email: vybhav@craftpower.cc

| Supported Targets | ESP32 | ESP32-S2 |
| ----------------- | ----- | -------- |

# _AUDIO_WS_CLIENT_
This is a simple ESP32 S2 Audio Websocket client project with following features
1. Connect to Wifi - Wifi credetials can be changed in beginning of main.c
2. Connect to Websocker server as client and maintain continuous connection - Server URL can be changed in beginning of main.c
3. Configure Audio driver for I2S pheriperals in beginning of main.c
4. Record Audio from Mic and send to server in stream of 20 seconds, then provide a break
5. If any data is recieved from server, verify if its Audio Binary data and play it on Speaker

## FAQ / Errors Solution
1. Mic is not working ? - Make sure Driver Bit rate, Sample Rate, Audio channel, Audio communication format are correct, and Pin is properly connected and configured
2. Speaker is not working ? - Make sure Driver Bit rate, Sample Rate, Audio channel, Audio communication format are correct, and Pin is properly connected and configured
3. Mic and Speaker are not working simultaneously - Make sure both devices have same I2S configuration [Hardware issue]
4. Websocket connect fail - make sure TLS is enabled in "menuconfig" and allow Allow "potentially insecure options" and "Skip server certificate verification by default (WARNING: ONLY FOR TESTING PURPOSE, READ HELP)"
5. Device crashing on recieving audio from server / not playing audio - make sure audio sent is in 2048 sample chunks and audio format is same as supported by speaker and I2S driver.


## How to use this
We encourage the users to use the example as a template for the new projects.
License : cannot for Commertial purpose, but can be used as wished for personal use. - Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0).
Contributors : M G Vybhav Nag
Attribution : to Craft Power [URL: craftpower.cc], M G Vybhav

Creative Commons Attribution-NonCommercial 4.0 International License (CC BY-NC 4.0).
Copyright © [2024] [M G Vybhav Nag] [craftpower.cc]

Permission is hereby granted to any person obtaining a copy of this software to use, copy, modify, and/or distribute the software, for personal or educational purposes, subject to the following conditions:

Non-Commercial Use: The software may not be used for commercial purposes. This means that the software, and any derivative works, cannot be sold, rented, leased, or used in a business or any other commercial enterprise or a product commercially sold.

Attribution: Appropriate credit must be given to the original author, including a link to the source repository or website.

Modification: You are free to modify the software for personal use, but modifications must retain this license and the non-commercial restriction.

Redistribution: You may redistribute this software as long as the original license, attribution, and non-commercial restriction remain intact.

Liability: The software is provided "as is", without warranty of any kind, express or implied. The author shall not be liable for any claim, damages, or other liability arising from the use of the software.