#!/bin/bash
echo enter file name
read fname

exec<$fname
value=0
date1="0"
while read line
do
if [ $date1 = "0" ]
then
date1=$line
continue
fi
#date1=$(date +"%s")
#date2=$(date +"%s")
#CURRENT=$(date +%s -d '2007-09-01 17:30:24')
#TARGET=$(date +%s -d'2007-12-25 12:30:00')
#CURRENT=$(date +%s -d $line)
#TARGET=$(date +%s -d $date1)
#MPHR=3600
#MINUTES=$(( ($TARGET - $CURRENT) / $MPHR ))
d2=$line
d1=$date1
 secdiff=$((
     $(date -d "$d2" +%s) - $(date -d "$d1" +%s)
	   ))
  if (("$secdiff" > 1))
  then
  nanosecdiff=$((
        $(date -d "$d2" +%N) - $(date -d "$d1" +%N)    ))
  echo "$(($secdiff)) seconds $nanosecdiff msecs elapsed between $d1 $d2"
  fi
date1=$line
done
