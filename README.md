# Millennium Manager - mm_manager


### Manager for Nortel Millennium Payphones

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/550811fd0fa140279b4edc2c1044860b)](https://www.codacy.com/gh/hharte/mm_manager/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=hharte/mm_manager&amp;utm_campaign=Badge_Grade)

[![Coverity](https://scan.coverity.com/projects/20341/badge.svg)](https://scan.coverity.com/projects/hharte-mm_manager)


# Overview

The Nortel Millennium payphones utilize a Manager to facilitate installation, reporting, call cost rating, and credit card processing.  This Millennium payphone calls into the Manager using a 1200-baud phone modem.  Unlike some other COCOT payphones, the Manager **_never calls the payphone_**.


# Compatibility

`mm_manager` runs on Windows, Linux, and MacOS X.

The `mm_manager` has been tested with a Nortel Multi-Pay (coin, credit card) Terminal with both V1.0 Control PCP (Through-hole, 1.20 firmware) and V1.1 Control PCP (Surface-mount, 2.11 firmware.)  It partially works with other terminal types and firmware versions, see table for limitations:


<table>
  <tr>
   <td><strong><em>Firmware Version</em></strong>
   </td>
   <td><strong><em>MTR</em></strong>
   </td>
   <td><strong><em>Terminal Type</em></strong>
   </td>
   <td><strong><em>Languages</em></strong>
   </td>
   <td><strong><em>Notes</em></strong>
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/NQA1X01/NT_FW_2.12_U2_Rev2_CPC_STM29F040B_32PLCC">NQA1X01</a>
   </td>
   <td>2.20
   </td>
   <td>MP/MC
   </td>
   <td>English / Spanish
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/tree/master/firmware/NOK1X03">NOK1X03</a>
   </td>
   <td>2.13
   </td>
   <td>MP/MC/TTY
   </td>
   <td>
   </td>
   <td>?
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/tree/master/firmware/NLA1X05">NLA1X05</a>
   </td>
   <td>2.12
   </td>
   <td>
   </td>
   <td>
   </td>
   <td>?
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/tree/master/firmware/NKA1X02">NKA1X02</a>
   </td>
   <td>2.11
   </td>
   <td>MP/MC
   </td>
   <td>English / Spanish
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/armeniki/Nortel-Millennium/tree/master/Firmware/MTR_2.11/Multi_Pay">NKA1X05</a>
   </td>
   <td>2.11
   </td>
   <td>MP/MC
   </td>
   <td>
   </td>
   <td>?
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/tree/master/firmware/NCA1X03">NCA1X03</a>
   </td>
   <td>2.0
   </td>
   <td>MP/MC/DJ
   </td>
   <td>
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/NPA1S01/dump2/NT_FW_1.20_STM27C2001_32DIP">NPA1S01</a>
   </td>
   <td>1.20
   </td>
   <td>MP/MC
   </td>
   <td>English / Spanish
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/NNK1F05/U5_MTR1.13%20Keyboard.BIN">NNK1F05</a>
   </td>
   <td>1.13
   </td>
   <td>MP/MC/TTY
   </td>
   <td>English / French
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/NBA1F02/U5_NBA1F02.BIN">NBA1F02</a>
   </td>
   <td>1.9
   </td>
   <td>MP/MC
   </td>
   <td>English / French
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/tree/71ccd39f3004b8e54ed6fa1af009516a9753a5fa/firmware/NNA1F02">NNA1F02</a>
   </td>
   <td>1.9
   </td>
   <td>MP/MC/TTY
   </td>
   <td>English / French
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/NBJ1S03/NBJ1S03.bin">NBJ1S03</a>
   </td>
   <td>1.9
   </td>
   <td>Desk
   </td>
   <td>English / Spanish
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td>NBE1J01
   </td>
   <td>1.9
   </td>
   <td>Card-only
   </td>
   <td>English / Japanese
   </td>
   <td>Provisions but can’t rate calls.
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/tree/master/firmware/M06CAA17">06CAA17</a>
   </td>
   <td>1.7
   </td>
   <td>MP/MC
   </td>
   <td>
   </td>
   <td>Appears working except for chip cards, but Mandatory Table Alarm is present.
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/06JAA03/U5_06JAA03.BIN">06JAA03</a>
   </td>
   <td>1.7
   </td>
   <td>MP/MC
   </td>
   <td>English / Japanese
   </td>
   <td>Fully working except for chip cards
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/12AAV08/U5.BIN">12AAV08</a>
   </td>
   <td>1.7
   </td>
   <td>MP/MC
   </td>
   <td>
   </td>
   <td>?
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/armeniki/Nortel-Millennium/blob/master/Firmware/MTR_1.7/Multi_Pay/NAA1S05.bin">NAA1S05</a>
   </td>
   <td>1.7
   </td>
   <td>MP/MC
   </td>
   <td>
   </td>
   <td>?
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/NPE1S01/NT_FW_1.20_NPE1S01_V1.0_M27C2001_DIP32_Coin_Basic.BIN">NPE1S01</a>
   </td>
   <td>1.20
   </td>
   <td>Coin Basic
   </td>
   <td>
   </td>
   <td>Fully working
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/armeniki/Nortel-Millennium/blob/master/Firmware/MTR_1.7/Coin/NAD4K02.bin">NAD4K02</a>
   </td>
   <td>1.7
   </td>
   <td>Coin Basic
   </td>
   <td>
   </td>
   <td>?
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/armeniki/Nortel-Millennium/blob/master/Firmware/MTR_1.7/Coin_NoDisplay/NAD4S02.bin">NAD4S02</a>
   </td>
   <td>1.7
   </td>
   <td>Coin Basic
   </td>
   <td>
   </td>
   <td>?
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/armeniki/Nortel-Millennium/blob/master/Firmware/MTR_1.7/Desk_Set/NAJ2S05.bin">NAJ2S05</a>
   </td>
   <td>1.7
   </td>
   <td>Desk
   </td>
   <td>
   </td>
   <td>Fully working except for chip cards
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/tree/master/firmware/M06CBBX1">06CBBX1</a>
   </td>
   <td>1.7
   </td>
   <td>Card-only
   </td>
   <td>English / Spanish
   </td>
   <td>Fully working except for chip cards
   </td>
  </tr>
  <tr>
   <td><a href="https://github.com/muccc/millennium/blob/master/firmware/PBAXS05/TEST.BIN">PBAXS05</a>
   </td>
   <td>1.7
   </td>
   <td>MP/MC/International
   </td>
   <td>
   </td>
   <td>Not working
   </td>
  </tr>
</table>



# Items Needed



1. Nortel Millennium Terminal running stock firmware (v1.20 or v2.20 is best, but other versions may work.)  **_Some Millennium Terminals purchased from online phone stores may have been re-programmed with “demo” firmware that does not need a Manager_**.  If you have one of these phones, you’ll have to program the phone back to [stock firmware](https://github.com/muccc/millennium/tree/master/firmware).
2. 24VDC @500mA Power Supply for Millennium Terminal
3. [4-pin plug-in screw terminal block](https://www.amazon.com/gp/product/B07SWSLR61) to connect 24VDC power and Tip/Ring to the terminal.
4. Two phone lines (one for `mm_manager`, one for the Millennium terminal.)  They can be real POTS lines, or lines from your own PBX, but the Millennium should be able to dial the manager with a 1- to 15-digit number.
5. 1200-baud or faster modem that supports [Bell 212A](https://en.wikipedia.org/wiki/Bell_212A) modulation.  I like the [Lenovo 56K USB](https://www.ebay.com/sch/i.html?&_nkw=lenovo+56k+usb+modem) modems, but any 56K modem with Conexant chipset should work.  Note for Linux users - Conexant soft-modems have never had a fully open-source driver, and the out-of-tree driver has not been maintained since kernel v2.4 - something like the US Robotics 5637 or another "real" modem where the device actually is responsible for encoding and decoding the data works well.
6. `mm_manager` software and a Windows, Linux machine (Raspberry Pi works great) or MacOS machine.  This machine should be available 24x7 so the terminal can call in when needed.
7. **_All terminals except desk:_** T-Key such as the [Jonard JIC-719A](https://jonard.com/jic-719a-t-key-tool?v=248) to open the payphone.  I don’t recommend the flat, stamped T-keys as they are prone to bending.
8. **_All terminals except desk:_** Keys for your terminal’s upper and lower locks, if locks are present.
9. **_Desk Terminal Only:_** A “Key Card” to enter the craft access menu.  This can be any credit card– best to use an expired one.  Specify the first 10-digits of the credit card number to `mm_manager` with the `-k` option.  The default key card number is 4012888888.  You can also program a magnetic stripe card with that number.


# mm_manager Installation and Usage

Simply download the source files and example tables from GitHub.


## Linux and MacOS

From the shell, type:


```
cmake .
make
```


to compile `mm_manager`, and several utilities.


## Windows

Compile `mm_manager` with Microsoft Visual Studio 2019 or later with CMake, using “Open Folder.”


### Usage:


```
usage: mm_manager [-vhmq] [-f <filename>] [-i "modem init string"] [-l <logfile>] [-p <pcapfile>] [-a <access_code>] [-k <key_code>] [-n <ncc_number>] [-d <default_table_dir] [-t <term_table_dir>] [-u <port>]
        -a <access_code> - Craft 7-digit access code (default: CRASERV)
        -b <baudrate> - Modem baud rate, in bps.  Defaults to 19200.
        -c - Always download complete table set.
        -d <default_table_dir> - default table directory.
        -e <error_inject_type> - Inject error on SIGBRK.
        -f <filename> modem device or file
        -h this help.
        -i "modem init string" - Modem initialization string.
        -k <key_code> - Desk Terminal 10-digit key card code (default: 4012888888)
        -l <logfile> - log bytes transmitted to and received from the terminal.  Useful for debugging.
        -m use serial modem (specify device with -f)
        -n <Primary NCC Number> [-n <Secondary NCC Number>] - specify primary and optionally secondary NCC number.
        -p <pcapfile> - Save packets in a .pcap file.
        -q - Don't display sign-on banner.
        -r - Rating test mode: Amount charged determined by last 4 digits of dialed number.
        -s - Download only minimum required tables to terminal.
        -t <term_table_dir> - terminal-specific table directory.
        -u <port> - Send packets as UDP to <port>.
        -v verbose (multiple v's increase verbosity.
        -w - don't monitor the modem for carrier loss.
```



# Millennium Terminal Hardware Installation


### Phone Line

The Nortel Millennium payphones require a standard POTS line, with answer supervision in the form of polarity reversal to indicate that the far end has answered the call.  This is required for the Millennium to know when to collect or refund coins.  Most SIP ATAs and Cisco Voice Routers can be configured for polarity reversal.  If your phone line does not support polarity reversal, an answer supervision detection module is available from Nortel.

If you are using VoIP, make sure to use the u-law PCM codec, disable silence suppression, disable comfort noise generation, and disable echo cancellation.  This is required to condition the line for modem operation.  You may also need to increase the jitter buffer size, and use a fixed jitter buffer, rather than adaptive.


### Power Supply

The Nortel Millennium terminal requires 24VDC at 500mA to supply power to the phone.  Only limited functionality is provided for emergency service when this power is not present.


### Other Terminal Installation Notes

The Millennium Terminal is an advanced payphone that contains a multitude of sensors to determine if the phone is installed and operating properly.  This includes sensors to make sure that a coin box is installed, and also a sensor to ensure the coin vault door is in place.  If your Millennium does not have a coin box, please obtain one.  The coin boxes are standard Western Electric / Northern Electric Single Slot, readily available.  In a pinch, the coin box sensor switch can be taped in the closed position.  The same goes for the coin vault door.  The coin vault door must be in place and locked.  If your phone is missing the coin vault lock, as is quite common for these phones purchased on the second-hand market, please obtain the correct lock, or tape the switch closed.

If these sensors are not happy, the phone will alarm, and will go “Out of Service.”


# Millennium Terminal Provisioning

Provisioning the Millennium Terminal is accomplished through the craft access menu on the terminal itself.  For this, you will need the terminal’s access code, and a PIN.  The default Access Code is 2727378 (CRASERV).

With the upper housing of the phone locked, take the handset off the cradle and replace it.  	Then key in 2727378 (CRASERV.)  You will be prompted for a PIN, use anything above 50000, like 55555.  Unlock the upper housing with a T-Key when prompted.  You do not need to open the upper housing.

Generate NPA and LCD tables (see below,) unless you want to use the default tables included with mm_manager.

Start `mm_manager`:


```
./mm_manager -m -n 18005551234 -f /dev/ttyACM0 -vv -l install.dlog -p install.pcap
```


Where:

	18005551234 should be replaced by the phone number of your Manager (1-15 digits).

	`/dev/ttyACM0` should be replaced by your modem device.  For Windows, it will be something like `\\.\COMx` where ‘`x`’ is the COM port number of your modem.

	install.dlog is a log file that contains all of the data received and sent by the manager.

	install.pcap is a packet capture that can be loaded into [Wireshark](https://github.com/hharte/mm_manager/tree/master/wireshark) for debugging.

Follow the Terminal’s on-screen prompts to install.

Key in this Terminal’s telephone number (10-digits) NPA-NXX-XXXX.

Key in this Terminal’s 10-digit serial number (1234567890 is fine.)

Key in the Manager’s phone number (I use 1-800-555-1234, which I intercept in the Asterisk dialplan and send to the modem connected to the computer running `mm_manager`.)


# Configuration Tables

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
   <td>10
   </td>
   <td>0xa
   </td>
   <td>Terminal Status
   </td>
   <td>10
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_term_status">TSTATUS</a> - pp. 2-636
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
   <td>18
   </td>
   <td>0x12
   </td>
   <td>Terminal Table Data Update
   </td>
   <td>0
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_table_upd">TERMDAT</a> - pp. 2-528
   </td>
  </tr>
  <tr>
   <td>20
   </td>
   <td>0x14
   </td>
   <td>Date/Time Synchronization
   </td>
   <td>7
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_time_sync">Time Sync</a>
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_ncc_term_parms">TERM</a> - pp. 2-492: Contains our number and  primary/sec NCC#, call in start date, time, interval, CDR threshold.
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_fconfig_opts">FEATRU</a> - pp. 2-151
   </td>
  </tr>
  <tr>
   <td>29
   </td>
   <td>0x1d
   </td>
   <td>Advertising Prompts
   </td>
   <td>480
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_advert_prompts">ADMESS</a> - pp. 2-5
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_user_if_parms">USERPRM</a> - pp. 2-669
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_install_parms">INSTSV</a> - pp. 2-235
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_comm_stat_parms">COMMST</a> - pp. 2-86
   </td>
  </tr>
  <tr>
   <td>33
   </td>
   <td>0x21
   </td>
   <td>Modem Parameters
   </td>
   <td>35
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_modem_parms">MODEM</a> - pp. 2-279
   </td>
  </tr>
  <tr>
   <td>34
   </td>
   <td>0x22
   </td>
   <td>Call Statistics Parameters
   </td>
   <td>18
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_call_stats">CALLST</a> - pp. 2-36
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_call_in_parms">TERM</a> table, pp. 2-517
   </td>
  </tr>
  <tr>
   <td>36
   </td>
   <td>0x24
   </td>
   <td>Time Synchronization Request
   </td>
   <td>0
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_time_sync_req">TTMSYNC</a>
   </td>
  </tr>
  <tr>
   <td>38
   </td>
   <td>0x26
   </td>
   <td>Terminal Cash Box Status
   </td>
   <td>62
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_cash_box_status">TCASHST</a> - pp. 2-420
   </td>
  </tr>
  <tr>
   <td>44
   </td>
   <td>0x2c
   </td>
   <td>Terminal Upload Attention
   </td>
   <td>1
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_atn_req_tab_upd">TUPLOAD</a> - pp. 2-656
<p>
TTBLREQ - pp. 2-651
   </td>
  </tr>
  <tr>
   <td>50
   </td>
   <td>0x32
   </td>
   <td>Coin Validation Parameters
   </td>
   <td>104
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_coin_val_table">COINVL</a> - pp. 2-79
   </td>
  </tr>
  <tr>
   <td>51
   </td>
   <td>0x33
   </td>
   <td>Cash Box
<p>
Collection
   </td>
   <td>70
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_cash_box_collection">TCOLLCT</a> - pp. 2-
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_rep_dial_list">RDLIST</a> - pp. 2-329
   </td>
  </tr>
  <tr>
   <td>58
   </td>
   <td>0x3a
   </td>
   <td>Service Level
   </td>
   <td>25
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_limserv_data">SERVLEV</a> - pp. 2-349
   </td>
  </tr>
  <tr>
   <td>60
   </td>
   <td>0x3c
   </td>
   <td>Terminal Software Version
   </td>
   <td>27
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_sw_version">TSWVERS</a> - pp. 2-647
   </td>
  </tr>
  <tr>
   <td>62
   </td>
   <td>0x3e
   </td>
   <td>Dialing Rules
   </td>
   <td>352
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_num_plan_table">DIALRL</a> - pp. 2-107
<p>
32 bytes: Fixed Format Data
<p>
180 bytes: Numbering Plan Rules
<p>
140 bytes: Display Formatting
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_spare_table">SPARE</a> - pp. 2-356 NOTE: This is a required table for V1.3 PCP, but not used on V1.0.
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_rate_table">RATE</a> - pp. 2-321
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
   <td>92B
   </td>
   <td>0x5c
   </td>
   <td>180 Number Call Screening List
   </td>
   <td>3060
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_call_screen_list">CALLSCRN</a> (Call Screening List) pp. 2-33, and FREE (Free Call) pp. 2-187
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_scard_parm_table">SMCARD</a> - pp. 2-351
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_card_table">CARD</a> - pp. 2-57
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_carrier_table">CARRIER</a> - pp. 2-69
   </td>
  </tr>
  <tr>
   <td>136-149
   </td>
   <td>0x88-0x95
   </td>
   <td>Double Compressed LCD Tables 1-14
   </td>
   <td>202
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_npa_nxx_table_1">LCD</a> (Local Call Determination) - pp. 2-248
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
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_npa_sbr_table">RATENPA</a> - pp. 2-327
   </td>
  </tr>
  <tr>
   <td>151
   </td>
   <td>97
   </td>
   <td>Set Based Rating - International Table
   </td>
   <td>603
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_intl_sbr_table">RATEINT</a> - pp. 2-326
   </td>
  </tr>
  <tr>
   <td>154,155
   </td>
   <td>0x9a,0x9b
   </td>
   <td>Double Compressed LCD Tables 15,16
   </td>
   <td>202
   </td>
   <td><a href="https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_npa_nxx_table_1">LCD</a> (Local Call Determination) - pp. 2-248
   </td>
  </tr>
</table>




* Page number references are to [Millennium Database Design Report MSR 2.1](https://github.com/muccc/millennium/blob/master/documentation/manager/A0xxxxxx_00_02.pdf).


## Generating NPA and LCD tables

Python3 scripts are included to generate NPA and LCD tables automatically, using spreadsheets available from the [North American Numbering Plan Administrator](https://nationalnanpa.com/).  For Canada, spreadsheets are available from the [Canadian Numbering Administrator](http://www.cnac.ca/).  These scripts require the Python3 libraries: pandas, requests, and xmltodict to be installed.

To generate an NPA table that classifies US and Canadian numbers as domestic:


```
./generate_npa.py --countries US CANADA
```


The `generate_lcd_lata.py` script is used to generate LCD tables for a given NPA-NXX in the United States.  LCD tables generated using this script will properly classify Local, Intra-LATA and Inter-LATA calls.  This script does not work in Canada as all of Canada is a single LATA.

For example, to generate LCD tables for San Jose, California, USA for a terminal with phone number of (408) 535-XXXX :


```
./generate_lcd_lata.py --npa=408 --nxx=535
```


To generate LCD tables for Ottawa, Canada, use `generate_lcd_lir.py`:


```
./generate_lcd_lir.py --npa 613 --nxx 562
```



## Terminal-Specific Tables

`mm_manager` has the ability to support multiple terminals with different provisioning. `mm_manager` searches for configuration tables as follows:



1. `tables/NPANXXXXXX` - where `NPANXXXXXX` is the 10-digit Terminal ID (phone number.)
2. `tables/<model-specific-dir>` - where `<model-specific-dir>` is one of:

	`multipay, card_only, desk, coin, inmate`.



3. `tables/default` - will be used as a last resort if tables cannot be found in the previous directories.

`mm_manager` stores the last table update date/time in the terminal-specific directory.  This allows for quicker iteration during testing by using "force download" in the terminal’s craft interface.  This will download only the table that changed and a few tables that are generated within `mm_manager` itself.


### Terminal-specific Table Example

Two multipay terminals with different Advertising messages, and otherwise configured the same:

`tables/4085359990/mm_table_1d.bin` - Advertising messages for terminal ID 4085359990

`tables/4085359995/mm_table_1d.bin` - Advertising messages for terminal ID 4085359995

`tables/default/` -  All of the default tables are in this directory (including `mm_table_1d.bin`)


# mm_manager Utilities


<table>
  <tr>
   <td>Utility
   </td>
   <td>Description
   </td>
  </tr>
  <tr>
   <td>mm_admess
   </td>
   <td>Dump Advertising Messages table
   </td>
  </tr>
  <tr>
   <td>mm_areacode
   </td>
   <td>Dump Set-based rating (NPA) table, MTR 1.20, 2.x
   </td>
  </tr>
  <tr>
   <td>mm_callin
   </td>
   <td>Dump Call-in Parameters table
   </td>
  </tr>
  <tr>
   <td>mm_callscrn
   </td>
   <td>Dump Call Screening List
   </td>
  </tr>
  <tr>
   <td>mm_callstat
   </td>
   <td>Dump Call Statistics Parameters table
   </td>
  </tr>
  <tr>
   <td>mm_card
   </td>
   <td>Dump Credit Card / Smart Card table MTR 1.20, 2.x
   </td>
  </tr>
  <tr>
   <td>mm_card_mtr1
   </td>
   <td>Dump Credit Card / Smart Card table MTR 1.7, 1.9
   </td>
  </tr>
  <tr>
   <td>mm_carrier.exe
   </td>
   <td>Dump Carrier table MTR 1.20, 2.x
   </td>
  </tr>
  <tr>
   <td>mm_carrier_mtr1
   </td>
   <td>Dump Carrier table MTR 1.7, 1.9
   </td>
  </tr>
  <tr>
   <td>mm_coinvl
   </td>
   <td>Dump Coin Validator table
   </td>
  </tr>
  <tr>
   <td>mm_commstat
   </td>
   <td>Dump Communications Statistics Parameters table
   </td>
  </tr>
  <tr>
   <td>mm_convert_callscrn_mtr2_to_mtr1
   </td>
   <td>Convert MTR 1.20/2.x Call Screening List to MTR 1.7, 1.9.
   </td>
  </tr>
  <tr>
   <td>mm_convert_card_mtr2_to_mtr1
   </td>
   <td>Convert MTR 1.20/2.x Card Table to MTR 1.7, 1.9.
   </td>
  </tr>
  <tr>
   <td>mm_dlog2pcap
   </td>
   <td>Convert mm_manager dialog output to pcap format for visualization with WireShark.
   </td>
  </tr>
  <tr>
   <td>mm_fconfig
   </td>
   <td>Dump Feature Configuration Options table
   </td>
  </tr>
  <tr>
   <td>mm_instsv
   </td>
   <td>Dump Installation / Servicing table
   </td>
  </tr>
  <tr>
   <td>mm_lcd
   </td>
   <td>Dump LCD tables (supports uncompressed, compressed, and double-compressed tables)
   </td>
  </tr>
  <tr>
   <td>mm_luhn
   </td>
   <td>Generate / Check magnetic card Luhn check digit
   </td>
  </tr>
  <tr>
   <td>mm_rate
   </td>
   <td>Dump Rate table
   </td>
  </tr>
  <tr>
   <td>mm_rateint
   </td>
   <td>Dump International Set-based rating table (MTR 1.20. 2.x)
   </td>
  </tr>
  <tr>
   <td>mm_rdlist
   </td>
   <td>Dump Repertory Dialer list
   </td>
  </tr>
  <tr>
   <td>mm_smcard
   </td>
   <td>Dump Smart Card (TeleCard) table
   </td>
  </tr>
  <tr>
   <td>mm_table_cutter
   </td>
   <td>Extract ROM tables from firmware binaries
   </td>
  </tr>
  <tr>
   <td>mm_userif
   </td>
   <td>Dump User Interface Parameters table
   </td>
  </tr>
</table>



# Low-Level Protocol

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


*CRC-16 IBM with the 0xA001 polynomial [per this site](https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/).  Includes all bytes including START through the DATA.

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
   <td>ERROR
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


# Debugging


## Dialog Transcripts

`mm_manager` can log all bytes sent to or received from a Millennium terminal using the `-l <logfile.dlog>` option.  This can be used to record a transcript of the session, that can be “played back” to `mm_manager` by specifying this file to the -f option, without supplying -m (modem.)  This allows quick iteration when debugging and testing `mm_manager`, as a real Millennium terminal is not needed.

One useful trick is to parse the transcript with `mm_manager`, and save it to a file.  Then the code can be modified and improved and tested by re-running the transcript through `mm_manager` and comparing it with the previous run using a tool such as `tkdiff`.


## Wireshark

`mm_manager` can save all packets sent and received to a packet capture (.pcap) file for viewing in [Wireshark](https://www.wireshark.org/) using the `-p <pcapfile.pcap>` option.  This .pcap file can be opened with [Wireshark](https://www.wireshark.org/), and dissected using the [Millennium LUA Dissector Plugin](https://github.com/hharte/mm_manager/blob/main/wireshark/README.md).

In addition, mm_manager can send all packets via UDP to the localhost port 27273 (“CRASE”) so [Wireshark](https://www.wireshark.org/) can view them in real-time while communicating with a terminal.


# Filing Bug Reports

If you find a bug, please provide the following information when you report the bug on GitHub:



1. Please make sure you run `mm_manager` with the `-l <filename>` option to generate the session transcript of all the data sent to and from the manager.  Please attach this file to your bug report.
2. Use mm_manager’s `-p <pcapfile.pcap>` option to save a packet capture and attach this file to your bug report.
3. Please provide information about the type of Millennium phone you have and what ROM version it’s running.  Some of this information is displayed by `mm_manager` after it connects to the phone.
4. Please provide details about the operating system you are using, what kind of modem you are using, and if you are using a serial modem, what type of serial port you are using (built-in PC serial port, USB serial port, etc.)
5. Please provide details about the phone lines you are using: Are they VoIP, POTS, going through your own PBX, or going over the PSTN?  Analog and TDM switches are preferable to VoIP, if at all possible.
6. Steps to reproduce the issue.


# Known Issues



1. When installing a Millennium Terminal, the answer supervision test will call the Manager, and disconnect.   It may take a few seconds for mm_manager’s modem to detect this condition and go back on-hook.  You may want to wait a few seconds after the test completes before proceeding with the installation.
2. Some older firmware versions will fail to install on the first try.  They will usually install correctly on a retry.
3. Lots of tables are not well understood.  Please help figure more out if you can.


# Further Work Needed (Feel free to help!)



1. Improve modem robustness.
2. Lots of other things, please file bugs as you find them.


# References

[Nortel Millennium on Wikipedia](https://en.wikipedia.org/wiki/Nortel_payphones#Millennium)

[Chaos Computer Club Muenchen](https://wiki.muc.ccc.de/millennium:start)

[muccc / millennium on GitHub](https://github.com/muccc/millennium/)

[Description of Millennium Tables](https://wiki.muc.ccc.de/millennium:dlog:start)

[nortel-voiceware-decoder](https://github.com/hharte/nortel-voiceware-decoder) - Voiceware ROM decoder to extract voice prompts to .wav files

[millenniium-panel on GitHub](https://github.com/pc-coholic/millennium-panel)

[Armeniki’s Nortel Millennium information on GitHub](https://github.com/armeniki/Nortel-Millennium)

[Millennium Database Design Report MSR 2.1](https://github.com/muccc/millennium/blob/master/documentation/manager/A0xxxxxx_00_02.pdf)

[Millennium Multi-Pay Terminal Installation, Operation, and Maintenance Guide](https://github.com/muccc/millennium/blob/master/documentation/nortel_millennium_text.pdf)

[Millennium Desk Terminal: Installing and Repairing Terminal Hardware](https://github.com/muccc/millennium/blob/master/documentation/desk_terminals/P0883900_00_01.pdf)

[pyMM](https://github.com/pc-coholic/pyMM)
