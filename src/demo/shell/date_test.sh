#!/bin/bash

printf "\nDate\t\tDOW\tDOY\t%%U %%V %%W\n"

for d in "12 "{25..31}" 08" "1 "{1..4}" 09" "12 "{25..31}" 09" "1 "{1..4}" 10" "12 "{25..31}" 10" "1 "{1..4}" 11"
do
    date -j -f "%m %d %y" "$d" +"%b %e %Y%t%a%t%j%t%U %V %W"
    echo \(`date -j -f "%m %d %y" "$d" +"%j"` - `echo \(\($(date -j -f "%m %d %y" "$d" +"%w") + 6\) % 7\) + 1 | bc` + 10\) / 7 | bc
done
