#!/usr/bin/env python3
# www.github.com/hharte/mm_manager
#
# (c) 2020, Howard M. Harte
#
# This script generates LCD tables for the Nortel Millennium Payphone.
# It generates the LCD tables by looking up information from
# localcallingguide.com to determine the LATA, given an NPA-NXX.
# This LATA is used and each exchange in the LATA is checked to
# determine which NPA-NXX are local.  All of these NPA-NXX are stored
# in a dictionary, and are considered Intra-LATA Toll.  Finally, the
# exchange of the specified NPA-NXX is used to determine which calls
# are actually local to the payphone.
#
# The first time this script is executed for a given LATA, the LATA
# data is downloaded and stored as a .csv.  Subsequent runs for the
# same LATA will use this cached data, unless it is older than a
# specified number of days.
#
# Tables 136-155 (0x88-0x9b)
#
# The LCD Table is an array of 202 bytes.  The first two bytes contain
# the NPA followed by 'e', for example, NPA 408 would be represented as
# 0x40 0x8e.
#
# The remaining 200 bytes represent two bits for each NXX in the range
# of 200-999.
#
# These two bits encode a value as follows:
# 0 - Local Rate
# 1 - ?
# 2 - Intra-LATA
# 3 - Invalid (ie, the N11, the NPA itself.)
#
# usage: generate_lcd_lata.py [-h] [--debug DEBUG] [--country COUNTRY]
#                        [--npa NPA] [--nxx NXX]
#
# For example, to generate LCD tables for the San Jose, California, USA rate center:
# ./generate_lcd.py --npa 408 --nxx 202
#
# Canada is not supported yet, because all of Canada is LATA 888

import argparse
import array
import csv
import requests
import sys
import time
import xmltodict

from os import path

def is_file_older_than_x_days(file, days=1):
    file_time = path.getmtime(file)
    # Check against 24 hours
    if (time.time() - file_time) / 3600 > 24*days:
        return True
    else:
        return False

parser = argparse.ArgumentParser()
parser.add_argument("--npa", help="Terminal's NPA", required=True)
parser.add_argument("--nxx", help="Terminal's NXX", required=True)
parser.add_argument("--debug", help="display debug output")
parser.add_argument("--age", help="age in days of LATA data before refresh", default=14)

args = parser.parse_args()

print("LCD Table Generator for the Nortel Millennium Payphone")
print("(c) 2020, Howard M. Harte\n")

my_npa = args.npa
my_nxx = args.nxx
lata_age_max = int(args.age)    # Maximum age of cached LATA data in days

print("Generating LCD tables for " + str(my_npa) + "-" + str(my_nxx))

r = requests.get("https://localcallingguide.com/xmlprefix.php?npa=" + str(my_npa) + "&nxx=" + str(my_nxx)).text
data = xmltodict.parse(r)['root']['prefixdata']

local_exch = data['exch']
lata = data['lata']
latacsvname = "lata-" + lata + ".csv"

if lata == 888:
    print("Error: Canada is not supported.")
    sys.exit()

print("Exchange: " + local_exch + " LATA: " + lata)

npa_dict = {}
npanxx_dict = {}

if path.exists(latacsvname) and is_file_older_than_x_days(latacsvname, days=lata_age_max) == False:

    print(latacsvname + " exists and is not older than " + str(lata_age_max) + " days, skipping generation")

    with open(latacsvname, mode='r') as latafile:
        reader = csv.reader(latafile)
        next(reader, None)  # Skip CSV header
        for rows in reader:
            npanxx = rows[0] + "-" + rows[1]
            npa_dict[rows[0]] = 0
            npanxx_dict[npanxx] = 2

else:
    r = requests.get("https://localcallingguide.com/xmlrc.php?lata=" + lata).text
    data = xmltodict.parse(r)['root']

    for cur in data['rcdata']:

        exch = str(cur['exch'])
        see_exch = str(cur['see-exch'])

        if(see_exch != 'None'):
            if args.debug: print("Skipping: " + exch)
            continue

        print(" Parsing: " + exch)

        r2 = requests.get("https://localcallingguide.com/xmllocalexch.php?exch=" + exch).text
        data2 = xmltodict.parse(r2)['root']['lca-data']

        for cur2 in data2['prefix']:
            try:
                npa = str(cur2['npa'])
                npanxx = npa + "-" + str(cur2['nxx'])
                npa_dict[npa] = 0
                npanxx_dict[npanxx] = 2
            except:
                print("Error parsing " + exch)

    with open(latacsvname, 'w', newline='') as g:
        csvwriter2 = csv.writer(g)
        csvwriter2.writerow(['NPA','NXX'])

        for i in sorted (npanxx_dict):
            csvwriter2.writerow(i.split('-'))

print("Adding NPA-NXX for local exchange...")

r2 = requests.get("https://localcallingguide.com/xmllocalexch.php?exch=" + local_exch).text
data2 = xmltodict.parse(r2)['root']['lca-data']

for cur2 in data2['prefix']:
    try:
        npa = str(cur2['npa'])
        npanxx = npa + "-" + str(cur2['nxx'])
        npa_dict[npa] = 0
        npanxx_dict[npanxx] = 0
    except:
        print("Error parsing " + exch)

lcd_npas = list()

for i in sorted (npa_dict):
    if(i == my_npa):
        lcd_npas.insert(0, i)
    else:
        lcd_npas.append(i)

if args.debug: print("NPAs for these rate centers: " + str(lcd_npas))

# Start with table 136 (0x88)
table = 136

# Loop through list of LCD NPAs for which we need to generate tables.
for i in lcd_npas:
    if table > 155:
        print("Error: maximum of 16 LCD tables reached.")
        break

    fname = "mm_table_" + hex(table)[2:4] + ".bin"
    print("Generating double-compressed LCD table " + fname + " for NPA " + i + ".")

    npa_h = int(int(i) / 100)
    npa_t = int((int(i) - (npa_h * 100)) / 10)
    npa_o = int((int(i) - (npa_h * 100)) - (npa_t * 10))

    npa_h = npa_h << 4
    npa_h = npa_h | npa_t
    npa_o = npa_o << 4
    npa_o = npa_o | 0x0e

    # Start LCD table array with first 3 digits of NPA followed by 0xe.
    a = array.array('B', [npa_h, npa_o])

    stflag = 0

    for index in range(200, 1000): #, row in allnpa.iterrows():
        cur_npanxx = str(i) + "-" + str(index)
        
        if cur_npanxx in npanxx_dict:
            flag = npanxx_dict.get(cur_npanxx)
        else:
            flag = 3    # No entry in NPA table, means invalid.

        # Pack into double-compressed LCD byte
        stflag = stflag << 2
        stflag = stflag | flag

        # Every 4th entry, add compressed LCD byte to table.
        if index % 4 == 3:
            a.append(stflag)
            stflag = 0

    # Write LCD table array to file.
    f=open(fname, "wb")
    a.tofile(f)
    f.close

    # Proceed to next table
    table = table + 1
    if table == 150: table = 154

print("Successfully completed generating LCD tables.")
