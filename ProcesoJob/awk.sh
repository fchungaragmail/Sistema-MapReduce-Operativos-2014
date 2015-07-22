#!/bin/bash

cat - | awk '{ anio = substr($0, 0, 4) + 0; temp = substr($0, 18, 4); if(max[anio] < temp) { max[anio] = temp }; } END { for (var in max) print var, ":", max[var]}'
