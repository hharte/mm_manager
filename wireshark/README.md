# Debugging Millennium Manager with Wireshark


# Overview

Wireshark can be used to view packet captures from the Millennium.  Packet captures can be obtained in the following ways:



1. Capture a transcript of all packets sent and received by mm_manager using the `-p filename.pcap` command line option.  The .pcap file can be loaded directly into Wireshark.
2. Convert a dialog byte stream created using mm_manager -l transcript.dlog using the mm_dlog2pcap utility.
3. Use the -u option to mm_manager to send all packets sent and received by mm_manger to UDP port 27273 on the loopback (127.0.0.1) interface, and use Wireshark to capture the packets in real-time.


# Installation

Download and install the latest version (Version 3.6.5 as of this writing) of Wireshark from [https://www.wireshark.org/](https://www.wireshark.org/).

When installing on Windows, make sure to install the Npcap library.  See the [Loopback article](https://wiki.wireshark.org/CaptureSetup/Loopback) for details.

After installing Wireshark, you need to install the Millennium dissector ([millennium.lua](https://github.com/hharte/mm_manager/blob/master/wireshark/millennium.lua)) into Wireshark’s plugins directory.  The plugins directory is different depending on the operating system.


## Dissector Plugin Installation


### For Windows

Create the plugins directory and copy `millennium.lua` into `%APPDATA%\wireshark\plugins` using the Windows Command Prompt as follows:

mkdir %APPDATA%\wireshark\plugins

copy wireshark\millennium.lua %APPDATA%\wireshark\plugins


### For Linux and MacOS

Create the plugins directory and copy `millennium.lua` into `~/.local/lib/wireshark/plugins `using a Terminal window:


```
mkdir -p ~/.local/lib/wireshark/plugins
cp wireshark/millennium.lua ~/.local/lib/wireshark/plugins
```



## Installing the Millennium Profile

Start Wireshark.  Import the [Millennium Wireshark Profile](https://github.com/hharte/mm_manager/blob/master/wireshark/millennium_ws_profile.zip) by right-clicking on the lower right corner of the Wireshark window where it says “Default Profile” and choose “Import” > “from ZIP file” and select the [millennium_ws_profile.zip](https://github.com/hharte/mm_manager/blob/master/wireshark/millennium_ws_profile.zip) file.


# Capturing Millennium Packets


## Capturing in Real Time

When starting a capture, select the Loopback Interface for capture and specify the following capture filter (Capture … using this filter:)


```
port 27273
```


After starting the capture, start mm_manager with the -u option, and wait for the terminal to interact with mm_manager.  You should see packets in Wireshark’s capture window in real-time.  Note that these packets are encapsulated in the UDP protocol to facilitate sending over the loopback interface, so there are a considerable amount of headers added to the packet.


## Collecting a Packet Capture using mm_manager

mm_manager can save all packets directly to a .pcap file while it is running.  The resulting .pcap file can be loaded directly into Wireshark.  To have mm_manager write all packets to a .pcap file, specify `-p filename.pcap` on the command line when invoking mm_manager.


## Converting a Dialog Transcript to a .pcap File

Dialog transcripts that have been saved using mm_manager’s -l option can be converted to a .pcap file using the mm_dlog2pcap utility:


```
mm_dlog2pcap transcript.dlog transcript.pcap
```


The resulting .pcap file can be loaded with Wireshark.


# References

[Capturing on the Loopback Inteface](https://wiki.wireshark.org/CaptureSetup/Loopback) 

[Wireshark Plugin Folders](https://www.wireshark.org/docs/wsug_html_chunked/ChPluginFolders.html)
