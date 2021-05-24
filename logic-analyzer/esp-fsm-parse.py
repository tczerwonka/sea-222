#!/usr/bin/python3

################################################################################
# 23may2021
#  esp-fsm-parse.py
#  state machine of sorts to parse the logic analyzer output from
#  the SEA ESP504 based upon known lines.
#
#  I should have done this years ago.
#
################################################################################
from csv import reader
import sys



################################################################################
#column defines
################################################################################
file = 2

#first
if (file == 1):
    logic_csv = 'ESP504-clean.csv'
    time_col = 0
    main_spi_en = 1
    A0 = 2
    A1 = 3
    A2 = 4
    SCK = 5
    MOSI = 6
    MISO = 7
    ch7 = 8

#Time [s],EN,SCK,MISO,MOSI,A0-23,A1-22,A2-21,Channel 7
if (file == 2):
    logic_csv = 'turnon2.csv'
    time_col = 0
    main_spi_en = 1
    SCK = 2
    MISO = 3
    MOSI = 4
    A0 = 5
    A1 = 6
    A2 = 7
    ch7 = 8



################################################################################
#decode the rest of the in_latch
################################################################################
def decode_in_latch(row):
    print("in_latch")



################################################################################
# read_logic_csv
################################################################################
def read_logic_csv():
    with open(logic_csv, 'r') as read_obj:
        # pass the file object to reader() to get the reader object
        csv_reader = reader(read_obj)
        header = next(csv_reader)
        if header != None:
            # Iterate over each row in the csv using reader object
            for row in csv_reader:
                print(row, end='')
                if (row[main_spi_en] == '0'):
                    if ((row[A2] == '0') & (row[A1] == '0') & (row[A0] == '0')):
                        print("UNDEFINED0-0")
                    if ((row[A2] == '0') & (row[A1] == '0') & (row[A0] == '1')):
                        decode_in_latch(row)
                    if ((row[A2] == '0') & (row[A1] == '1') & (row[A0] == '0')):
                        print("UNDEFINED2-0")
                    if ((row[A2] == '0') & (row[A1] == '1') & (row[A0] == '1')):
                        print("LED-DTMF-EN")
                    if ((row[A2] == '1') & (row[A1] == '0') & (row[A0] == '0')):
                        print("DSP_LATCH")
                    if ((row[A2] == '1') & (row[A1] == '0') & (row[A0] == '1')):
                        print("UNDEFINED5-0")
                    if ((row[A2] == '1') & (row[A1] == '1') & (row[A0] == '0')):
                        print("UNDEFINED6-0")
                    if ((row[A2] == '1') & (row[A1] == '1') & (row[A0] == '1')):
                        print("UNDEFINED7-0")
                if (row[main_spi_en] == '1'):
                    if ((row[A2] == '0') & (row[A1] == '0') & (row[A0] == '0')):
                        print("UNDEFINED0-1")
                    if ((row[A2] == '0') & (row[A1] == '0') & (row[A0] == '1')):
                        print("UNDEFINED1-1")
                    if ((row[A2] == '0') & (row[A1] == '1') & (row[A0] == '0')):
                        print("UNDEFINED2-1")
                    if ((row[A2] == '0') & (row[A1] == '1') & (row[A0] == '1')):
                        print("LED-DTMF-EN")
                    if ((row[A2] == '1') & (row[A1] == '0') & (row[A0] == '0')):
                        print("UNDEFINED4-1")
                    if ((row[A2] == '1') & (row[A1] == '0') & (row[A0] == '1')):
                        print("DAC_EN")
                    if ((row[A2] == '1') & (row[A1] == '1') & (row[A0] == '0')):
                        print("SYN_EN")
                    if ((row[A2] == '1') & (row[A1] == '1') & (row[A0] == '1')):
                        print("MAIN_EN")



################################################################################
# main
################################################################################
def main():
    print("starting")
    read_logic_csv()
    sys.exit(0)



################################################################################
################################################################################
if __name__ == "__main__":
    main()
