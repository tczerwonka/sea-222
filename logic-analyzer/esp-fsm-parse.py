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

logic_csv = 'ESP504-clean.csv'



################################################################################
#column defines
################################################################################
time_col = 0
main_spi_en = 1
A0 = 2
A1 = 3
A2 = 4
SCK = 5
MOSI = 6
MISO = 7
ch7 = 8



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
                if (row[main_spi_en]):
                    print("SPI_EN")
                # row variable is a list that represents a row in csv
                print(row[0])
                print(row)



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
