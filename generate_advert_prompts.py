#!/usr/bin/env python3
# -*- coding: latin-1 -*-
#
# This script generates the Advert Prompts table for Nortel Millennium Payphone.
#
# The table is generated based on a .csv file containing 20 advertising messages.
# The first 10 are on hook messages and the last 10 are off hook.
#
# Messages must be a maxiumum of 20 charcters long. Attributes can be used to
# modify how the message is displayed. Messages shoter than 20 characters will
# have spaces padded to the end.
#
# Either pass in a CSV as a parameter or modify the default configuration in
# 'config/advert_prompts.csv'.
#
# You can optionally include a terminal ID to write the table directly to
# the tables folder.
#
# Table details:
# https://wiki.muc.ccc.de/millennium:dlog:dlog_mt_advert_prompts
#
# usage: generate_advert_prompts.py [--csv path_to_file.csv] [--terminal terminal_id]


import argparse
import array
import csv
import os

parser = argparse.ArgumentParser()
parser.add_argument("--csv", help="Path to CSV", default="config/advert_prompts.csv")
parser.add_argument("--terminal", help="Terminal ID for custom tables")

args = parser.parse_args()

print("Advert Prompts Table Generator for the Nortel Millennium Payphone")
print("Generating Advert Prompts table")

prompts = []

with open(args.csv, mode='r') as promptsfile:
    READER = csv.reader(promptsfile)
    next(READER, None)  # Skip CSV header
    for rows in READER:
        prompt = [int(rows[0]), int(rows[1]), rows[2]]
        prompts.append(prompt)

FNAME = "mm_table_1d.bin"
if args.terminal:
    FNAME = "tables/" + args.terminal + "/" + FNAME
    os.makedirs(os.path.dirname(FNAME), exist_ok=True)

table = array.array('B')

for prompt in prompts:
    duration_adjusted = int(prompt[0] / 10) # divide duration by 10ms
    duration_high, duration_low = divmod(duration_adjusted, 0x100)
    message = prompt[2]

    table.append(duration_low) # timing low
    table.append(duration_high) # timing high
    table.append(prompt[1]) # attributes
    table.append(0x00) # additional attributes

    for letter in message:
        table.append(ord(letter))

    for i in range(len(message), 20):
        table.append(0x20)

# Write table array to file.
F = open(FNAME, "wb")
table.tofile(F)

print("Generated " + FNAME + ".")

# check if table is 480 bytes, if not the transfer will fail.
filesize = F.tell()
if filesize != 480:
    print("!!! FILESIZE IS WRONG, TABLE MAY NOT WORK AS INTENDED !!!")
    print("Make sure the CSV has 20 messages total and each message has a maximum length of 20 characters.")
