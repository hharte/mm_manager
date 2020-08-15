#!/usr/bin/env python3
# -*- coding: latin-1 -*-
# www.github.com/hharte/mm_manager
#
# (c) 2020, Howard M. Harte
#
# This script generates the International Set-based rating table for
# Nortel Millennium Payphone.
#
# The table is generated based on a .csv file containing International
# calling codes and their associated RATE table entry.
#
# The International Set-based rating able is an array of 603 bytes.
# The first three bytes define the default flags and rate entry for
# international codes not found in the remainder of the table.
# The remainder of the table is an array of 200 entries of three bytes
# each.  Two bytes for calling code, and one byte for flags/rate table
# entry.  Thanks to æstrid for figuring this table out.
#
# usage: generate_intl_sbr.py

import array
import csv

print("International Set-based Rating Table Generator for the Nortel Millennium Payphone")
print("(c) 2020, Howard M. Harte\n")

print("Generating International Set-based Rating table")

icc_dict = {}

with open("icc_dial_codes.csv", mode='r') as iccfile:
    reader = csv.reader(iccfile)
    next(reader, None)  # Skip CSV header
    for rows in reader:
        icc_dict[rows[2]] = rows[3]

fname = "mm_table_97.bin"

# Start International table with Flags, Default Rate, Spare.
a = array.array('B', [0x01, 0x05, 0x00])

index = 0

for item in icc_dict.items():
    index += 1

    ccode = int(item[0])
    rate_entry = int(item[1])

    if (rate_entry >= 2):
        rate_entry -= 28

    a.append(ccode & 0xff)
    a.append(ccode >> 8)
    a.append(rate_entry)

for i in range(index, 200):
    a.append(0)
    a.append(0)
    a.append(0)

# Write LCD table array to file.
f=open(fname, "wb")
a.tofile(f)
f.close

print("Generated " + fname + ".")
