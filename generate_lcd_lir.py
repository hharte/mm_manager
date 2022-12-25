#!/usr/bin/env python3
# www.github.com/hharte/mm_manager
#
# (c) 2022, Howard M. Harte
#
# This script generates LCD tables for the Nortel Millennium Payphone
# for Canada.
#
# It generates the LCD tables by looking up information from
# localcallingguide.com to determine the LIR, given an NPA-NXX.
# Each exchange in the LIR is checked to determine which NPA-NXX are
# local.
#
# After all local exchanges have been checked, the NPAs for the
# terminal are known, and COCodeStatus_ALL.csv is consulted to determine
# which NPA-NXX are valid.  Valid NPA-NXX that are valid but not already
# classified as local are added as long-distance.
#
# The first time this script is executed for a given LIR, the LIR
# data is downloaded and stored as a .csv.  Subsequent runs for the
# same LIR will use this cached data, unless it is older than a
# specified number of days.
#
# usage: generate_lcd_lir.py [-h] --npa NPA --nxx NXX [--debug DEBUG] [--age AGE]
#
# For example, to generate LCD tables for the Ottawa LIR:
# generate_lcd_lir.py --npa 613 --nxx 562

import argparse
import csv
import pandas
import requests
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
parser.add_argument("--age", help="age in days of LIR data before refresh", default=14)
parser.add_argument("--datafile", help="NPA-NXX source data file")

args = parser.parse_args()

print("Canadian LCD Table Generator for the Nortel Millennium Payphone")
print("(c) 2022, Howard M. Harte\n")

my_npa = args.npa
my_nxx = args.nxx
lir_age_max = int(args.age)    # Maximum age of cached LIR data in days

print("Generating LCD tables for " + str(my_npa) + "-" + str(my_nxx))

source_filename = 'COCodeStatus_ALL.csv'
if args.datafile: source_filename = args.datafile

print("Reading data from " + source_filename)
try:
    df = pandas.read_csv(source_filename, sep=',', dtype={'NPA': object, 'CO Code (NXX)': object}, engine='python')
except OSError:
    print("Error: Cannot read NPA-NXX data file.")
    print("Please download http://www.cnac.ca/data/COCodeStatus_ALL.zip and unzip it into the current directory.")
    exit()

df = df.rename(columns={'CO Code (NXX)': 'NXX', 'Status': 'Use', "Exchange Area": 'RateCenter'})
df["NPA"].fillna("0", inplace = True)
df["NXX"].fillna("0", inplace = True)
df["RateCenter"].fillna("None", inplace = True)

df["NPA-NXX"] = df["NPA"] + "-" + df["NXX"]

all_npanxx = df["NPA-NXX"].to_dict()

if args.debug:
    print("Debug output:")
    print(all_npanxx)

r = requests.get("https://localcallingguide.com/xmlprefix.php?npa=" + str(my_npa) + "&nxx=" + str(my_nxx)).text
try:
    data = xmltodict.parse(r)['root']['prefixdata'][0]
except (KeyError):
    data = xmltodict.parse(r)['root']['prefixdata']

local_exch = data['exch']
lir = data['lir']
lircsvname = "lir-" + lir + ".csv"

print("Exchange: " + local_exch + " lir: " + lir)

npa_dict = {}
npanxx_dict = {}

if path.exists(lircsvname) and is_file_older_than_x_days(lircsvname, days=lir_age_max) is False:

    print(lircsvname + " exists and is not older than " + str(lir_age_max) + " days, skipping generation")

    with open(lircsvname, mode='r') as lirfile:
        reader = csv.reader(lirfile)
        next(reader, None)  # Skip CSV header
        for rows in reader:
            npanxx = rows[0] + "-" + rows[1]
            npa_dict[rows[0]] = 0
            npanxx_dict[npanxx] = 2

else:
    r = requests.get("https://localcallingguide.com/xmlrc.php?lir=" + lir).text
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

    with open(lircsvname, 'w', newline='') as g:
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

df2 = df[df['NPA'].isin(lcd_npas)]
df1 = df2[['NPA', 'NXX', 'Use', 'RateCenter']]

for key, row in df1.iterrows():
    npanxx = str(row['NPA']) + '-' + row['NXX']
    if (str(row['Use']) == "AS") | (str(row['Use']) == "In Service"): # NXX is Assigned
        # Either local or LD based on rate center.
        if npanxx not in npanxx_dict:
            npanxx_dict[npanxx] = 2 # LD
    else: # UA NXX is "Unassigned"
        npanxx_dict[npanxx] = 3 # UA is invalid

if args.debug: print("NPAs for these rate centers: " + str(lcd_npas))

if args.debug:
    print(npanxx_dict)

status = generate_lcd_tables(npanxx_dict, lcd_npas, my_npa)

if status == 0:
    print("Successfully completed generating LCD tables.")
else:
    print("LCD table limit exceeded for one or more MTR versions.")
