## Millennium Manager - mm_manager


#### Manager for Nortel Millennium Payphones


## Overview

The Nortel Millennium payphones utilize a Manager to facilitate installation, reporting, and call cost rating.  This Millennium payphone calls into the Manager using a 1200-baud phone modem.


## Compatibility

`mm_manager` runs on Linux and MacOS X.



The `mm_manager` has been tested with a Nortel Multi-Pay (coin, credit card) Terminal with both V1.0 Control PCP (Through-hole, 1.20 firmware) and V1.3 Control PCP (Surface-mount, 2.20 firmware.)  It may work with other versions, please let me know.


## Items Needed



1. Nortel Millennium Multi-Pay Terminal running firmware v1.20 or v2.20.  **_Some Millennium Terminals purchased from online phone stores may have been re-programmed with “demo” firmware that does not need a Manager_**.  If you have one of these phones, you’ll have to program the phone back to stock v1.20 or v2.20 firmware.
2. 24VDC @500mA Power Supply for Millennium Terminal
3. Two phone lines (one for `mm_manager`, one for Millennium terminal.)  They can be real POTS lines, or lines from your own PBX, but the Millennium should be able to dial the manager with a 1- to 15-digit number.
4. 1200-baud or faster modem that supports [Bell 212A](https://en.wikipedia.org/wiki/Bell_212A) modulation.  I like the [Lenovo 56K USB](https://www.ebay.com/i/264145034802) modems, but any 56K modem with Conexant chipset should work.
5. T-Key such as the [Jonard JIC-719A](https://jonard.com/jic-719a-t-key-tool?v=248) to open the payphone.  I don’t recommend the flat, stamped T-keys as they are prone to bending.
6. Keys for your terminal’s upper and lower locks, if locks are present.
7. `mm_manager` software and a Linux machine (Raspberry Pi works great) or MacOS machine.  This machine should be left on 24x7 so the terminal can call in when needed.


## mm_manager Installation and Usage

Simply download the source files and example tables from GitHub.  Then type “make” to compile `mm_manager`, and a few utilities.


#### Usage:


```
usage: mm_manager [-vhm] [-b <baudrate] [-f <filename>] [-l <logfile>] [-a <access_code>] [-n <ncc_number>]
	-v verbose 	multiple v's increase verbosity.
	-f <filename> 	modem device or file
	-h 			this help.
	-l <logfile> 	log bytes transmitted to and received from
            the terminal. Useful for debugging.
	-m 			use serial modem (specify device with -f)
-b <baudrate>	Modem baud rate, in bps.  Defaults to 19200.
	-n <Primary NCC Number> [-n <Secondary NCC Number>] - specify
            primary and optionally secondary NCC number.
```



## Millennium Terminal Hardware Installation


#### Phone Line

The Nortel Millennium payphones require a standard POTS line, with answer supervision in the form of polarity reversal to indicate that the far end has answered the call.  This is required for the Millennium to know when to collect or refund coins.  Most SIP ATAs and Cisco Voice Routers can be configured for polarity reversal.  If your phone line does not support polarity reversal, an answer supervision detection module is available from Nortel.

If you are using VoIP, make sure to use the u-law PCM codec, disable silence suppression, disable comfort noise generation, and disable echo cancellation.  This is required to condition the line for modem operation.  You may also need to increase the jitter buffer size, and use a fixed jitter buffer, rather than adaptive.


#### Power Supply

The Nortel Millennium terminal requires 24VDC to supply power to the phone.  Only limited functionality is provided for emergency service when this power is not present.


#### Other Terminal Installation Notes

The Millennium Terminal is an advanced payphone that contains a multitude of sensors to determine if the phone is installed and operating properly.  This includes sensors to make sure that a coin box is installed, and also a sensor to ensure the coin vault door is in place.  If your Millennium does not have a coin box, please obtain one.  The coin boxes are standard Western Electric / Northern Electric Single Slot, readily available.  In a pinch, the coin box sensor switch can be taped in the closed position.  The same goes for the coin vault door.  The coin vault door must be in place and locked.  If your phone is missing the coin vault lock, as is quite common for these phones purchased on the second-hand market, please obtain the correct lock, or tape the switch closed.

If these sensors are not happy, the phone will alarm, and will go “Out of Service.”


## Millennium Terminal Provisioning

Provisioning the Millennium Terminal is accomplished through the craft access menu on the terminal itself.  For this, you will need the terminal’s access code, and a PIN.  The default Access Code is 2727378 (CRASERV).

With the upper housing of the phone locked, take the handset off the cradle and replace it.  Then key in 2727378 (CRASERV.)  You will be prompted for a PIN, use anything above 50000, like 55555.  Unlock the upper housing with a T-Key when prompted.  You do not need to open the upper housing.

Start `mm_manager`:


```
./mm_manager -m -n 18005551234 -f /dev/ttyACM0 -vv -l install.bytes
```


Where:

	18005551234 should be replaced by the phone number of your Manager (1-15 digits).

	/dev/ttyACM0 should be replaced by your modem device.

	install.bytes is a log file that will contain all of the data received and sent by the manager.

Follow on-screen prompts on the Terminal to install.

Key in this Terminal’s telephone number (10-digits) NPA-NXX-XXXX.

Key in this Terminal’s 10-digit serial number (1234567890 is fine.)

Key in the Manager’s phone number (I use 1-800-555-1234, which I intercept in the Asterisk dialplan and send to the modem connected to the computer running `mm_manager`.)




## Configuration Tables

Communication with the Nortel Millennium involves sending and receiving tables.  Tables are numbered 1 through 155, and contain configuration information sent to the terminal or query / status information received from the terminal.  Tables are of fixed size, depending on the type of table.

Each packet response from the Manager includes the Terminal’s phone number, followed by a byte containing the Table ID of the current table being sent by the Manager.  Multiple tables may be packed into a single packet (the maximum packet table payload length is 245 bytes.)

In the case of large table download (> 245 bytes) to the Millennium terminal, the table payload is split into 245-byte chunks, but the Table ID is only included in the first packet.

The following are some of the tables used by the Millennium terminal.  They should be customized (using a hex editor) for your specific phone.  Some parameters (like NCC numbers and Access Code) can be changed via command-line parameters to `mm_manager`.

Some of the important tables for configuring a Millennium terminal include:


<table>
  <tr>
   <td><strong><em>Table ID (DEC)</em></strong>
   </td>
   <td><strong><em>Table ID (Hex)</em></strong>
   </td>
   <td><strong><em>Description</em></strong>
   </td>
   <td><strong><em>Length</em></strong>
   </td>
   <td><strong><em>Notes</em></strong>
   </td>
  </tr>
  <tr>
   <td>36
   </td>
   <td>0x24
   </td>
   <td>Time Sync Request
   </td>
   <td>0
   </td>
   <td>TTMSYNC
   </td>
  </tr>
  <tr>
   <td>20
   </td>
   <td>0x14
   </td>
   <td>Set Date / Time
   </td>
   <td>7
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>44
   </td>
   <td>0x2c
   </td>
   <td>Attention Request Table Update
   </td>
   <td>1
   </td>
   <td>TUPLOAD / TTBLREQ
   </td>
  </tr>
  <tr>
   <td>38
   </td>
   <td>0x26
   </td>
   <td>Cash Box Status Message
   </td>
   <td>62
   </td>
   <td>TCASHST (Terminal Cash Box Status)
<p>
TCOLLCT
   </td>
  </tr>
  <tr>
   <td>18
   </td>
   <td>0x12
   </td>
   <td>Terminal Table Data Update
   </td>
   <td>0
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>151
   </td>
   <td>97
   </td>
   <td>Set Based Rating International Table
   </td>
   <td>603
   </td>
   <td>RATE and RATEINT: Almost all 00’s
   </td>
  </tr>
  <tr>
   <td>14
   </td>
   <td>0x0e
   </td>
   <td>Table Update ACK
   </td>
   <td>1
   </td>
   <td>Terminal -> Manager
   </td>
  </tr>
  <tr>
   <td>150
   </td>
   <td>0x96
   </td>
   <td>Set Based Rating NPA Table
   </td>
   <td>400
   </td>
   <td>See RATENPA, 200-999, 4-bits each
   </td>
  </tr>
  <tr>
   <td>138
   </td>
   <td>0x8a
   </td>
   <td>Double Compressed LCD Tables
   </td>
   <td>202
   </td>
   <td>LCD (Local Call Determination)
   </td>
  </tr>
  <tr>
   <td>137
   </td>
   <td>0x89
   </td>
   <td>Double Compressed LCD Tables
   </td>
   <td>202
   </td>
   <td>LCD (Local Call Determination)
   </td>
  </tr>
  <tr>
   <td>136
   </td>
   <td>0x88
   </td>
   <td>Double Compressed LCD Tables
   </td>
   <td>202
   </td>
   <td>LCD (Local Call Determination)
   </td>
  </tr>
  <tr>
   <td>135
   </td>
   <td>0x87
   </td>
   <td>Expanded Carrier Table (33 Entries)
   </td>
   <td>1108
   </td>
   <td>CARRIER
<p>
9 + (33 x 33) + 10
   </td>
  </tr>
  <tr>
   <td>134
   </td>
   <td>0x86
   </td>
   <td>Expanded Card Table (32 Entries)
   </td>
   <td>1152
   </td>
   <td>HOTRNG 36 x 32
   </td>
  </tr>
  <tr>
   <td>93
   </td>
   <td>0x5d
   </td>
   <td>Smart Card Parameters Table
   </td>
   <td>224
   </td>
   <td>Hex data
   </td>
  </tr>
  <tr>
   <td>92B
   </td>
   <td>0x5c
   </td>
   <td>180 Number Call Screening List P2-277
   </td>
   <td>3060
   </td>
   <td>(17 x 180) Flags:
<p>
0x00=unused
<p>
0x01=Toll Free
<p>
0x41=Information
<p>
0x81=Operator
   </td>
  </tr>
  <tr>
   <td>
   </td>
   <td>
   </td>
   <td>Japanese VFD Text
   </td>
   <td>8000
   </td>
   <td>Japanese: V1.3 Only
<p>
0x7A001 offset in U2 ROM.
   </td>
  </tr>
  <tr>
   <td>86
   </td>
   <td>0x56
   </td>
   <td>Expanded Visual Prompts Language B
   </td>
   <td>8000
   </td>
   <td>Spanish: V1.3 Only
<p>
Qty 400: 20-line VFD strings
<p>
0x77001 offset in U2 ROM.
   </td>
  </tr>
  <tr>
   <td>
   </td>
   <td>
   </td>
   <td>French VFD Text
   </td>
   <td>8000
   </td>
   <td>French: V1.3 Only
<p>
0x74001 offset in U2 ROM.
   </td>
  </tr>
  <tr>
   <td>85
   </td>
   <td>0x55
   </td>
   <td>Expanded Visual Prompts Language A
   </td>
   <td>8000
   </td>
   <td>English: V1.3 Only
<p>
Qty 400: 20-line VFD strings
<p>
0x71B5A offset in U2 ROM.
   </td>
  </tr>
  <tr>
   <td>73
   </td>
   <td>0x49
   </td>
   <td>Rate Table
   </td>
   <td>1191
   </td>
   <td>RATE
   </td>
  </tr>
  <tr>
   <td>72
   </td>
   <td>0x48
   </td>
   <td>Spare Table
   </td>
   <td>1000
   </td>
   <td>Almost all 00, except one block of hex data. NOTE: This is a required table for V1.3 PCP, but not used on V1.0.
   </td>
  </tr>
  <tr>
   <td>62
   </td>
   <td>0x3e
   </td>
   <td>Numbering Plan Table
   </td>
   <td>352
   </td>
   <td>More hex data
   </td>
  </tr>
  <tr>
   <td>60
   </td>
   <td>0x3c
   </td>
   <td>Terminal SW Version
   </td>
   <td>27
   </td>
   <td>TSWVERS
   </td>
  </tr>
  <tr>
   <td>58
   </td>
   <td>0x3a
   </td>
   <td>Service Level Table
   </td>
   <td>25
   </td>
   <td>All 00’s.
   </td>
  </tr>
  <tr>
   <td>55
   </td>
   <td>0x37
   </td>
   <td>Enhanced Repertory Dialer List
   </td>
   <td>570
   </td>
   <td>RDLIST 10 rows of 57 bytes
   </td>
  </tr>
  <tr>
   <td>50
   </td>
   <td>0x32
   </td>
   <td>Coin Validation Table
   </td>
   <td>104
   </td>
   <td>COINVL
   </td>
  </tr>
  <tr>
   <td>35
   </td>
   <td>0x23
   </td>
   <td>Time/Call-In Parameters
   </td>
   <td>20
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>34
   </td>
   <td>0x22
   </td>
   <td>Call / Carrier Statistics Parameters
   </td>
   <td>18
   </td>
   <td>CALLST
   </td>
  </tr>
  <tr>
   <td>33
   </td>
   <td>0x21
   </td>
   <td>Modem Configuration Parameters
   </td>
   <td>35
   </td>
   <td>MODEM (Modem Parameters)
   </td>
  </tr>
  <tr>
   <td>32
   </td>
   <td>0x20
   </td>
   <td>Communication Statistics and Configuration Parameters
   </td>
   <td>32
   </td>
   <td>COMMST
   </td>
  </tr>
  <tr>
   <td>31
   </td>
   <td>0x1f
   </td>
   <td>Installation/Servicing Parameters
   </td>
   <td>36
   </td>
   <td>INSTSV Access Code In first 7 nibbles.
   </td>
  </tr>
  <tr>
   <td>30
   </td>
   <td>0x1e
   </td>
   <td>User Interface Parameters Universal
   </td>
   <td>67
   </td>
   <td>USERPRM (User Interface Parameters)
   </td>
  </tr>
  <tr>
   <td>29
   </td>
   <td>0x1d
   </td>
   <td>Advertising Prompts
<p>
seq, text, duration(2), effects
   </td>
   <td>480
   </td>
   <td>ADMESS
<p>
Text info about rates for local and anywhere in the US Calls. 20 entries.
   </td>
  </tr>
  <tr>
   <td>26
   </td>
   <td>0x1a
   </td>
   <td>Feature Configuration – Universal
   </td>
   <td>71
   </td>
   <td>FEATRU
   </td>
  </tr>
  <tr>
   <td>21
   </td>
   <td>0x15
   </td>
   <td>Terminal Access Parameters
   </td>
   <td>47
   </td>
   <td>TERM Contains our number and  primary/sec NCC#, call in start date, time, interval, CDR threshold.
   </td>
  </tr>
  <tr>
   <td>13
   </td>
   <td>0xd
   </td>
   <td>End of Data Message
   </td>
   <td>0
   </td>
   <td>Manager sends no data.
   </td>
  </tr>
  <tr>
   <td>10
   </td>
   <td>0xa
   </td>
   <td>Terminal Status
   </td>
   <td>10
   </td>
   <td>5 byte serial number, 5 byte TSTATUS register.
   </td>
  </tr>
</table>





### Generating NPA and LCD tables

Python3 scripts are included to generate NPA and LCD tables automatically, using spreadsheets available from the [North American Numbering Plan Administrator](https://nationalnanpa.com/).  For Canada, spreadsheets are available from the [Canadian Numbering Administrator](http://www.cnac.ca/).  These scripts require the Python3 Pandas library to be installed.

To generate an NPA table that classifies US and Canadian numbers as domestic:


```
./generate_npa.py --countries US CANADA
```


To generate LCD tables for rate centers in San Jose, California, USA:


```
./generate_lcd.py --state CA --npa 408 --ratecenters "SNJS NORTH" "SNJS WEST" "SNJS SOUTH" CAMPBELL SARATOGA SUNNYVALE "LOS GATOS"
```


To generate LCD tables for Ottawa, Canada:


```
./generate_lcd.py --country CA --npa 613 --ratecenters Ottawa-Hull
```



## Low-Level Protocol

The low-level protocol sent over the modem is a stream of bytes framed within START and END bytes.


<table>
  <tr>
   <td>START
   </td>
   <td>FLAG Byte
   </td>
   <td>LEN (includes END)
   </td>
   <td>DATA
   </td>
   <td>CRC-16
   </td>
   <td>END
   </td>
  </tr>
  <tr>
   <td>02
   </td>
   <td>1-byte
   </td>
   <td>1-byte
   </td>
   <td>N bytes...
   </td>
   <td>2-bytes *
   </td>
   <td>03
   </td>
  </tr>
</table>


*CRC-16 IBM with A001 polynomial [per this site](https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/).  Includes all bytes including START through the DATA.

FLAG Bits:


<table>
  <tr>
   <td>7
   </td>
   <td>6
   </td>
   <td>5
   </td>
   <td>4
   </td>
   <td>3
   </td>
   <td>2
   </td>
   <td>1
   </td>
   <td>0
   </td>
  </tr>
  <tr>
   <td>?
   </td>
   <td>?
   </td>
   <td>DISCONNECT
   </td>
   <td>?
   </td>
   <td>ACK
   </td>
   <td>RETRY
   </td>
   <td colspan="2" >Sequence
   </td>
  </tr>
</table>


Separate sequence numbers are counted for each of tx_seq and rx_seq.  ACKs should always be sent with the same sequence as the last received Rx packet.  Tx seq is incremented after every successful packet transmission, and reset when the terminal disconnects.


## Debugging

`mm_manager` can log all bytes sent to or received from a Millennium terminal.  This can be used to record a transcript of the session, that can be “played back” to `mm_manager` by specifying this file to the -f option, without supplying -m (modem.)  This allows quick iteration when debugging and testing `mm_manager`, as a real Millennium terminal is not needed.

One useful trick is to parse the transcript with `mm_manager`, and save it to a file.  Then the code can be modified and improved and tested by re-running the transcript through `mm_manager` and comparing it with the previous run using a tool such as `tkdiff`.


## Filing Bug Reports

If you find a bug, please provide the following information when you report the bug on GitHub:



1. Please make sure you run `mm_manager` with the `-l &lt;filename>` option to generate the session transcript of all the data sent to and from the manager.  Please attach this file to your bug report.
2. Please provide information about the type of Millennium phone you have and what ROM version it’s running.  Some of this information is displayed by `mm_manager` after it connects to the phone.
3. Please provide details about the operating system you are using, what kind of modem you are using, and if you are using a serial modem, what type of serial port you are using (built-in PC serial port, USB serial port, etc.)
4. Please provide details about the phone lines you are using: Are they VoIP, POTS, going through your own PBX, or going over the PSTN?  Analog and TDM switches are preferable to VoIP, if at all possible.
5. Steps to reproduce the issue.


## Known Issues



1. `mm_manager` can run under the [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/about) available on Windows 10.  Note, however, that there may be some issue with modem initialization.  To work around this, connect to your modem using a program like [Putty](https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html) first, to make sure it is working, then quit Putty and run `mm_manager`.
2. Sometimes, `mm_manager` doesn’t properly detect a disconnect, and subsequently does not reset its tx_seq.  If the Terminal calls in after this, it will disconnect after receiving a packet with the incorrect tx_seq.  Eventually, it will get back in sync.
3. Lots of tables are not well understood.  Please help figure more out if you can.


## Further Work Needed (Feel free to help!)



1. Improve modem robustness.
2. Port modem code to Win32 to support Windows XP/Windows 7.
3. Lots of other things, please file bugs as you find them.


## References

[Nortel Millennium on Wikipedia](https://en.wikipedia.org/wiki/Nortel_payphones#Millennium)

[Chaos Computer Club Muenchen](https://wiki.muc.ccc.de/millennium:start)

[muccc / millennium on GitHub](https://github.com/muccc/millennium/)

[Description of Millennium Tables](https://wiki.muc.ccc.de/millennium:dlog:start)

[YouTube Video of Millennium and mm_manager Installation](https://youtu.be/A_g4DSWtDx4)

[millenniium-panel on GitHub](https://github.com/pc-coholic/millennium-panel)

[Millennium Database Design Report MSR 2.1](https://github.com/muccc/millennium/blob/master/documentation/manager/A0xxxxxx_00_02.pdf)

[Millennium Multi-Pay Terminal Installation, Operation, and Maintenance Guide](https://github.com/muccc/millennium/blob/master/documentation/nortel_millennium_text.pdf)
