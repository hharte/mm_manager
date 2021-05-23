#!/usr/bin/env python3
# Script to generate LCD tables for the Nortel Millennium Payphone
# Tables 136-138 (0x88-0x8a)
#
# www.github.com/hharte/mm_manager
#
# (c) 2020, Howard M. Harte
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
# 2 - Long-distance NXX
# 3 - Invalid (ie, the N11, the NPA itself.)
#
# USA: https://nationalnanpa.com/reports/reports_cocodes_assign.html
# Canada: http://www.cnac.ca/co_codes/co_code_status.htm
#
# All NPA-NXX in the USA: wget https://nationalnanpa.com/nanp1/allutlzd.zip
# All NPA-NXX in  Canada: wget http://www.cnac.ca/data/COCodeStatus_ALL.zip
#
# usage: generate_lcd.py [-h] [--debug DEBUG] [--country COUNTRY]
#                        [--state STATE] [--npa NPA] [--datafile DATAFILE]
#                        --ratecenters RATECENTERS [RATECENTERS ...]
#
# For example, to generate LCD tables for the San Jose, California, USA rate center:
# ./generate_lcd.py --state CA --npa 408 --ratecenters "SNJS NORTH" "SNJS WEST" "SNJS SOUTH" CAMPBELL SARATOGA SUNNYVALE "LOS GATOS"
#
# To generate LCD tables for Ottawa, Canada:
# ./generate_lcd.py --country CA --npa 613 --ratecenters Ottawa-Hull

import argparse
import array
import pandas

parser = argparse.ArgumentParser()
parser.add_argument("--debug", help="display debug output")
parser.add_argument("--country", help="Country 'US' or 'CA'", default='US')
parser.add_argument("--state", help="State (only for Country 'US')", default='CA')
parser.add_argument("--npa", help="Terminal's NPA", default='408')
parser.add_argument("--datafile", help="NPA-NXX source data file")
parser.add_argument("--ratecenters", nargs='+', help='List of rate centers', required=True)

args = parser.parse_args()

print("LCD Table Generator for the Nortel Millennium Payphone")
print("(c) 2020, Howard M. Harte\n")
print("Terminal NPA: " + args.npa)

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

    df = df.rename(columns={'CO Code (NXX)': 'NXX', 'Status': 'Use', "Rate Center": 'RateCenter'})
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

# Start with table 136 (0x88)
table = 136

# Loop through list of LCD NPAs for which we need to generate tables.
for i in lcd_npas:
    if table > 155:
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

    allnpa = df[df['NPA'].str.match(i)]

    stflag = 0

    for index in range(200, 1000): #, row in allnpa.iterrows():
        use = allnpa.loc[allnpa['NXX'] == str(index)]
        flag = 0

        if use.empty:   # No entry in NPA table, means invalid.
            flag = 3
        else:

            if (str(use['Use'].values[0]) == "AS") | (str(use['Use'].values[0]) == "In Service"):   # NXX is Assigned
                # Either local or LD based on rate center.
                if use['RateCenter'].values[0] in args.ratecenters:
                    flag = 0 # local
                else:
                    flag = 2 # LD
            else:   # UA NXX is "Unassigned"
                flag = 3 # UA is invalid

        # Pack into double-compressed LCD byte
        stflag = stflag << 2
        stflag = stflag | flag

        # Every 4th entry, add compressed LCD byte to table.
        if index % 4 == 3:
            a.append(stflag)
            stflag = 0

    # Display table data
#        print(a)

    # Write LCD table array to file.
    f=open(fname, "wb")
    a.tofile(f)

    # Proceed to next table
    table = table + 1
    if table == 150: table = 154

if table > 155:
    print("Error: maximum of 16 LCD tables reached.")
else:
    print("Successfully completed generating LCD tables.")
