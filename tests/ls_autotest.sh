#/bin/bash



# check for xdotool
xdotool --help >/dev/null 2>&1 || { echo "ERROR: cannot start tests because xdotool doesn't work"; exit 1; }


sleep 5

while read p
do
  xdotool type --delay 20 "$p
"
done <<EOF
# regular ls
bin/ls
# ls with one flag
bin/ls -a
# ls with many flags
bin/ls -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a -a
# with lots of letters for one flag
bin/ls -aaaaaaaaaaaaaaaaaaaalllllllllllllllllllllll
# invalid flag
bin/ls -j
# invalid flag mixed
bin/ls -a -l -j
# invalid flag mixed (a different way)
bin/ls -aaallllRRRRRRRRRRRjxxx
# /\\ notice how it didn't complain about the x
# it stops at the first bad flag char
# -l
bin/ls -l
# -al
bin/ls -al

# on a dir
bin/ls bin
# on a file
bin/ls LICENSE
# on two dirs
bin/ls bin src
# on a dir and file
bin/ls bin LICENSE

# on a dir -a
bin/ls bin -a
# on a file -a
bin/ls LICENSE -a
# on two dirs -a
bin/ls bin src -a
# on a dir and file -a
bin/ls bin LICENSE -a

# on a dir -l
bin/ls bin -l
# on a file -l
bin/ls LICENSE -l
# on two dirs -l
bin/ls bin src -l
# on a dir and file -l
bin/ls bin LICENSE -l

# on a dir -la
bin/ls bin -la
# on a file -la
bin/ls LICENSE -la
# on two dirs -la
bin/ls bin src -la
# on a dir and file -la
bin/ls bin LICENSE -la

# -R
bin/ls -R
# -Ra
bin/ls -Ra
# -Rl
bin/ls -Rl
# -Rla
bin/ls -Rla

# on a dir -R
bin/ls bin -R
# on a file -R
bin/ls LICENSE -R
# on two dirs -R
bin/ls bin src -R
# on a dir and file -R
bin/ls bin LICENSE -R

# on a dir -Rl
bin/ls bin -Rl
# on a file -Rl
bin/ls LICENSE -Rl
# on two dirs -Rl
bin/ls bin src -Rl
# on a dir and file -Rl
bin/ls bin LICENSE -Rl

# on a dir -Ra
bin/ls bin -Ra
# on a file -Ra
bin/ls LICENSE -Ra
# on two dirs -Ra
bin/ls bin src -Ra
# on a dir and file -Ra
bin/ls bin LICENSE -Ra

# on a dir -Rla
bin/ls bin -Rla
# on a file -Rla
bin/ls LICENSE -Rla
# on two dirs -Rla
bin/ls bin src -Rla
# on a dir and file -Rla
bin/ls bin LICENSE -Rla
exit
EOF




