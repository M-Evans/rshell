#/bin/bash

SCRIPTNAME=

sleep 5

while read p
do
  xdotool type --delay 20 "$p
"
done <<EOF
script $SCRIPTNAME
TESTS GO HERE
EOF

