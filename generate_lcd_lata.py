#!/usr/bin/env python3
# www.github.com/hharte/mm_manager
#
# (c) 2020-2022, Howard M. Harte
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
# usage: generate_lcd_lata.py [-h] --npa NPA --nxx NXX [--debug DEBUG] [--age AGE]
#
# For example, to generate LCD tables for the San Jose, California, USA rate center:
# python3 generate_lcd_lata.py --npa 408 --nxx 535
#
# Canada is not supported, because all of Canada is LATA 888

import argparse
import csv
import requests
import sys
import time
import xmltodict
from mm_lcd import generate_lcd_tables

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
print("(c) 2020-2022, Howard M. Harte\n")

my_npa = args.npa
my_nxx = args.nxx
lata_age_max = int(args.age)    # Maximum age of cached LATA data in days

print("Generating LCD tables for " + str(my_npa) + "-" + str(my_nxx))

r = requests.get("https://localcallingguide.com/xmlprefix.php?npa=" + str(my_npa) + "&nxx=" + str(my_nxx)).text
try:
    data = xmltodict.parse(r)['root']['prefixdata'][0]
except (KeyError):
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

if path.exists(latacsvname) and is_file_older_than_x_days(latacsvname, days=lata_age_max) is False:

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
            except (TypeError):
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
    except (TypeError):
        print("Error parsing " + exch)

lcd_npas = list()

for i in sorted (npa_dict):
    if(i == my_npa):
        lcd_npas.insert(0, i)
    else:
        lcd_npas.append(i)

if args.debug: print("NPAs for these rate centers: " + str(lcd_npas))

status = generate_lcd_tables(npanxx_dict, lcd_npas, my_npa)

if status == 0:
    print("Successfully completed generating LCD tables.")
else:
    print("LCD table limit exceeded for one or more MTR versions.")
