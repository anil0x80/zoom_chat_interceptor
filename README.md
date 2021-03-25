# zoom_chat_interceptor
This project hooks the Zoom meeting's chat for incoming messages and raises an alarm if the arbitrary specified text is found on the incoming message.

# How to use
Compile as win32 DLL, inject the DLL into zoom.exe. (Keep in mind zoom.exe has 2 instances, one of them is for the meeting, the other for the home page. It won't work if you inject into the wrong instance.) You can attach a debugger and check to debug output for messages if there is any problem. 


# How does it work?
There is a callback function which is called for every incoming message. The function is in "zVideoUI.dll", signature: "55 8B EC 56 57 8B F9 FF 15 ? ? ? ? 85 C0 74 3A"

By hooking this, we can search incoming messages, and we can check if a specific person has sent a message by setting search text as his/her name.

The code that I upload here will check and raise an alarm if anyone writes "Hello!" on chat. You can use your imagination to think everything you can do with this :) [hint: you can specify your instructor's name and if he sends a text, you will be alarmed]

As you can see, I have implemented this by hooking the UI function. A more stable method might be hooking network functions, but I coded this real quick and it works.

# Alarm
Beeps using WinApi's Beep function. This also automatically unmutes Zoom if it is muted, since this DLL is also injected to zoom, zoom needs to be unmuted for the alarm to work.

# Warning
There is a possibility of the signature of the "magic" function changing between zoom versions. The method has been working stable for months, but that does not guarantee it will work.
