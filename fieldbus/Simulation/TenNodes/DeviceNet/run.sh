#!/bin/sh

./TENNODES -f zero.ini -r 1
./TENNODES -f zero.ini -r 2
./TENNODES -f zero.ini -r 3
./TENNODES -f zero.ini -r 4
./TENNODES -f zero.ini -r 5
./TENNODES -f zero.ini -r 6
./TENNODES -f zero.ini -r 7
./TENNODES -f zero.ini -r 8
./TENNODES -f zero.ini -r 9

./TENNODES -f random.ini -r 1
./TENNODES -f random.ini -r 2
./TENNODES -f random.ini -r 3
./TENNODES -f random.ini -r 4
./TENNODES -f random.ini -r 5
./TENNODES -f random.ini -r 6
./TENNODES -f random.ini -r 7
./TENNODES -f random.ini -r 8
./TENNODES -f random.ini -r 9

./TENNODES -f scheduled.ini -r 1
./TENNODES -f scheduled.ini -r 2
./TENNODES -f scheduled.ini -r 3
./TENNODES -f scheduled.ini -r 4
./TENNODES -f scheduled.ini -r 5
./TENNODES -f scheduled.ini -r 6
./TENNODES -f scheduled.ini -r 7
./TENNODES -f scheduled.ini -r 8
./TENNODES -f scheduled.ini -r 9

./TENNODES -f scheduled?.ini -r 1
./TENNODES -f scheduled?.ini -r 2
./TENNODES -f scheduled?.ini -r 3
./TENNODES -f scheduled?.ini -r 4
./TENNODES -f scheduled?.ini -r 5
./TENNODES -f scheduled?.ini -r 6
./TENNODES -f scheduled?.ini -r 7
./TENNODES -f scheduled?.ini -r 8
./TENNODES -f scheduled?.ini -r 9

