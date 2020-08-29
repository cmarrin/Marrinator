# Marrinator

Marrinator is a set of libraries and source code to make it possible to run programs on both Mac and ESP8266 (and eventyally ESP32 and Raspberry PI Zero).

## Why do this?

ESP8266 is a powerful embedded system with WiFi which is incredibly inexpensive. Anything you can do with Arduino, you can do with ESP8266. And it is usually cheaper than, for instance, and Arduino Uno. And you can even get Arduino compatible ESP6266 boards so you can take advantage of the vast array of hardware in the Arduino format.

But like all embedded devices, it's often difficult to debug programs written for it. So I made a library so you can build and run the same code on Mac and the ESP. There are platform specific pieces, but they are small and isolated. So often you can get a program running on Mac and it will run without a problem on the ESP.

There is an ESP8266 SDK for Arduino. That's nice, but it's pretty memory intensive and uses the NONOS version of the ESP SDK. So you're limited to single threaded execution and you can't take advantage of some nice memory saving techniques. My library sits on top of the ESP RTOS SDK, which uses FreeRTOS. A simplistic program built on Marrinator will have around 50K of RAM left, which on the Arduino SDK you're lucky if you get 40K. There are several reasons for this, which I'll go into soon.

## Memory Use

The ESP8266 has very little RAM available. It has 80KB of "user ram" which is available to user programs, but is also used for some system purposes. But it also has another 80KB of system RAM used for various purposes. With the Arduino SDK you only use user RAM and all constant strings use this memory. There are techniques you can use to get around this, like putting strings in Flash. But there caveats and you have to be careful doing it.

The ESP RTOS SDK does a couple of clever things. First, it puts every constant data structure into Flash unless you tell it otherwise. That means string and other data types as well. The ESP restricts read access to Flash to 32 bit boundaries. So if you put, say an array of bytes in Flash you have to carefully read the data 4 bytes at a time and pick out the byte you need. the ESP RTOS SDK handles all this automatically. A "non-aligned" access to Flash will fail. The SDK sees this, picks out the data you need, and gives it to you. This is no doubt slower, but that's rarely an issue. The ESP RTOS is also clever in its use of system RAM. The 80KB of system RAM is normally used as caches for executing code and storing system data. But when linking it can determine how much is needed for those purposes and make the rest available to the user. Typically you can get an extra 16KB this way, making the total available 96KB, which gives you more overhead for you programs.

## Building

Mac uses xcode. Simply open the mac/Marrinator.xcodeproj file and build the m8rsim target to get an executable that opens a window showing you a console and a button that flashes a heartbeart, simulating the LED on the WemosD1 Mini, and then runs a sample native task.

The system is designed to run on the the Wemos D1 Mini, although it's not hard to get it running on other models. Follow these steps:

1) Install the [ESP toolchain](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/macos-setup.html)
2) Install the [ESP RTOS SDK](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html#get-esp8266-rtos-sdk)
3) Add a path to in you environment: `export IDF_PATH=/Users/<your username>/esp/ESP8266_RTOS_SDK`
4) Add a path for your serial port: `export ESPPORT=/dev/cu.usbserial-2430`

The port above is an example. Connect your Wemos via USB and look in `/dev`. There should be a `cu.usbserial-xxxx` file in there. Use that. If there is more than one you probably have something else connected.

Once that is installed you can go to the esp directory and type:

~~~~
make -j40 flash monitor
~~~~

Once you've successfully loaded the Wemos will try to connect to Wifi, which will not work because it needs the SSID and password. These will be setup with a WebServer, which is not yet complete. After connecting to Wifi (or fails to do so) it will run the task, which prints a message and starts blinking the LED. It blinks at 1 second intervals until the Wifi connects, then it switches to 3 second intervals.

### More Later...
