# Bash script for image translation
#!/bin/bash

RVG='triangle1.rvg'

# First argument is the number of steps for x, second for y (starting from 0)

xsteps=$1
ysteps=$2
xstart=0
ystart=0
#xstart=$3
#ystart=$4

xtvals=$(seq $xstart $xsteps)
ytvals=$(seq $ystart $ysteps)


for tx in $xtvals; do
  for ty in $ytvals; do
    for input in $RVG; do
        output=${input%.*}'-x'$tx'-y'$ty.png
        lua process.lua driver.lua.png $input $output -tx:$tx -ty:$ty 
    done
  done
done
