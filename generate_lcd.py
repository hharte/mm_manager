#!/usr/bin/env python3
# Script to generate LCD tables for the Nortel Millennium Payphone
#
# www.github.com/hharte/mm_manager
#
# (c) 2020-2022, Howard M. Harte
#
# USA: https://nationalnanpa.com/reports/reports_cocodes_assign.html
# Canada: http://www.cnac.ca/co_codes/co_code_status.htm
#
# All NPA-NXX in the USA: wget https://nationalnanpa.com/nanp1/allutlzd.zip
# All NPA-NXX in  Canada: wget http://www.cnac.ca/data/COCodeStatus_ALL.zip
#
# usage: generate_lcd.py [-h] [--debug DEBUG] [--country COUNTRY] [--state STATE]
#                        [--npa NPA] [--nxx NXX] [--datafile DATAFILE]
#                        --ratecenters RATECENTERS [RATECENTERS ...]
#
# For example, to generate LCD tables for the San Jose, California, USA rate center:
# python3 generate_lcd.py --state CA --npa 408 --nxx 535 --ratecenters "SNJS NORTH" "SNJS WEST" "SNJS SOUTH" CAMPBELL SARATOGA SUNNYVALE "LOS GATOS"
#
# To generate LCD tables for Ottawa, Canada:
# python3 generate_lcd.py --country CA --npa 613 --nxx 535 --ratecenters Ottawa-Hull

import argparse
import array
import pandas
from mm_lcd import *

parser = argparse.ArgumentParser()
parser.add_argument("--debug", help="display debug output")
parser.add_argument("--country", help="Country 'US' or 'CA'", default='US')
parser.add_argument("--state", help="State (only for Country 'US')", default='CA')
parser.add_argument("--npa", help="Terminal's NPA", default='408')
parser.add_argument("--nxx", help="Terminal's NXX", default='535')
parser.add_argument("--datafile", help="NPA-NXX source data file")
parser.add_argument("--ratecenters", nargs='+', help='List of rate centers', required=True)

args = parser.parse_args()

print("LCD Table Generator for the Nortel Millennium Payphone")
print("(c) 2020-2022, Howard M. Harte\n")
print("Terminal NPA-NXX: " + args.npa + "-" +args.nxx)

if args.country == 'US':
    source_filename = 'allutlzd.txt'
    if args.datafile: source_filename = args.datafile

    print("Reading " + args.country + " data from " + source_filename)
    try:
        if ".csv" in source_filename: # Import as CSV
            df = pandas.read_csv(source_filename, dtype={'NPA-NXX': object}, engine='python')
        else: # Import as TSV
            df = pandas.read_csv(source_filename, dtype={'NPA-NXX': object}, sep=' *\t', engine='python')
    except OSError:
        print("Error: Cannot read NPA-NXX data file.")
        print("Please download https://nationalnanpa.com/nanp1/allutlzd.zip and unzip it into the current directory.")
        exit()

    df["NPA-NXX"].fillna("0", inplace = True)
    df["State"].fillna("Unknown", inplace = True)
    df["Use"].fillna("Unknown", inplace = True)
    df["RateCenter"].fillna("None", inplace = True)

    # Split NPA-NXX into separate columns in dataframe.
    df[['NPA','NXX']] = df['NPA-NXX'].str.split("-",expand=True)

    # Filter out by state.
    sc = df[df['State'].str.match(args.state)]

else:
    source_filename = 'COCodeStatus_ALL.csv'
    if args.datafile: source_filename = args.datafile

    print("Reading " + args.country + " data from " + source_filename)
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
    sc = df

if args.debug: print(sc[['NPA', 'NXX', 'RateCenter']])

def prepRateForRegex(r):
    return r'^' + r + '$'

rate_centers = map(prepRateForRegex,args.ratecenters)

rc = sc[sc['RateCenter'].str.match('|'.join(rate_centers))]
if args.debug: print(rc[['NPA', 'NXX', 'RateCenter']])

# Get list of unique NPAs that have NXX in matching rate centers.
npas = rc['NPA'].unique()

lcd_npas = list()

# Convert list of NPAs to array, starting with Terminal's own NPA.
for i in range(len(npas)):
    if(npas[i] == args.npa):
        lcd_npas.insert(0, npas[i])
    else:
        lcd_npas.append(npas[i])

print("Rate Centers: " + str(args.ratecenters))
print("NPAs for these rate centers: " + str(lcd_npas))

npa_dict = {}

df1 = df[['NPA', 'NXX', 'Use', 'RateCenter']]
for key, row in df1.iterrows():
    npanxx = str(row['NPA']) + '-' + row['NXX']
    if (str(row['Use']) == "AS") | (str(row['Use']) == "In Service"): # NXX is Assigned
        # Either local or LD based on rate center.
        if row['RateCenter'] in args.ratecenters:
            flag = 0 # local
        else:
            flag = 2 # LD
    else: # UA NXX is "Unassigned"
        flag = 3 # UA is invalid
    npa_dict[npanxx] = flag

status = generate_lcd_tables(npa_dict, lcd_npas, args.npa, args.nxx)

if status == 0:
    print("Successfully completed generating LCD tables.")
else:
    print("LCD table limit exceeded for one or more MTR versions.")
