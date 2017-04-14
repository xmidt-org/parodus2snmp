# parodus2snmp

Webpa-to-SNMP bridge

[![Build Status](https://travis-ci.org/Comcast/parodus2snmp.svg?branch=master)](https://travis-ci.org/Comcast/parodus2snmp)
[![codecov.io](http://codecov.io/github/Comcast/parodus2snmp/coverage.svg?branch=master)](http://codecov.io/github/Comcast/parodus2snmp?branch=master)
[![Coverity](https://img.shields.io/coverity/scan/9155.svg)]("https://scan.coverity.com/projects/comcast-parodus2snmp)
[![Apache V2 License](http://img.shields.io/badge/license-Apache%20V2-blue.svg)](https://github.com/Comcast/parodus2snmp/blob/master/LICENSE.txt)

# Building and Testing Instructions

```
mkdir build
cd build
cmake ..
make
make test
```

# Coding Formatter Settings

Please format pull requests using the following command to keep the style consistent.

```
astyle -A10 -S -f -U -p -D -c -xC90 -xL
```
