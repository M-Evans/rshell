# rshell
UCR CS100 Project
-----------------
## What is rshell?
* a basic shell for unix-based systems
* not finished
* very limited

## Features
* Command chaining with ``||``, ``&&``, and ``;``
* Executes commands
* Comments are not interpreted
* Handles EOF being sent
* Handles the exit command
* No memory leaks! (that I know of)
* Says hello and goodbye

## Bugs
* If a syntax error is detected, no chained commands before the syntax error will be executed. (unlike in other shells where they can sometimes be recovered from)
* Semicolons between ``&&`` and ``||`` connectors are ignored
* It does not have the capability to prepare food
