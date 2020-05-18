#==================================================================
# Author       : Pointner Sebastian
# Company      : Johannes Kepler University
# Name         : SMT Macro Placer
# Workfile     : db_to_csv.sh
#
# Date         : 18.May 2020
# Copyright    : Johannes Kepler University
# Description  : Script to Export Sqlite3 Tables to CSV
#==================================================================

#! /bin/bash

sqlite3 $1 <<EOF
.headers on
.mode csv
.output $2
SELECT * FROM macros
EOF
