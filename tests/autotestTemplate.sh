#/bin/bash

# check for xdotool
xdotool --help >/dev/null 2>&1 || { echo "ERROR: cannot start tests because xdotool doesn't work"; exit 1; }


sleep 5

while read p
do
  xdotool type --delay 20 "$p
"
done <<EOF
TESTS GO HERE
EOF

