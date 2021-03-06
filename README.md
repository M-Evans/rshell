# rshell
UCR CS100 Project
-----------------
## What is rshell?
* a basic shell for unix-based systems
* not finished
* very limited
* contains a custom version of ``ls``.

## Features (rshell)
* Command chaining with ``||``, ``&&``, and ``;``
* Executes commands
* Comments are not interpreted
* Handles EOF being sent
* Handles the exit command
* No memory leaks! (that I know of)
* Says hello and goodbye

## Bugs (rshell)
* If a syntax error is detected, no chained commands before the syntax error will be executed. (unlike in other shells where they can sometimes be recovered from)
* Semicolons between ``&&`` and ``||`` connectors and their corresponding commands are ignored. If there is no command between connectors or the end of the line, then it's still a syntax error. Also, if the semicolon connects to the ``&&`` or ``||`` operators, then the syntax error is still thrown. Only doesn't work for a small portion of the cases.
* It does not have the capability to prepare food

## Features (ls)
* color! 
* accepts -a as a parameter (lists hidden files)
* accepts -l as a parameter (lists lots of info about the files and directories, as well as a cumulative block count of the things printed)
* accepts -R as a parameter (lists the contents of directories and their directories)
* accepts any number of files/directories to print
* parameters and flags are accepted in any order

## Bugs (ls)
* compared to the GNU ls, I sort things differently. I sort everything alphabetically including the preceding periods. GNU ls sorts everything based on the first letter, ignoring preceding periods
* it does not gracefully handle improper flags - they cause fatal errors

## Features (signals)
* ignores ``SIGINT``
  * ``SIGINT`` is recieved by child
  * without children, ``SIGINT`` does not do anything
  * handling of ``SIGINT`` is child-dependent
* ignores ``SIGTSTP``
  * ``SIGTSTP`` is received by child
  * If the child goes into the background, it is remembered
  * it can then be foregrounded or backgrounded as you please
* ``cd`` is implemented
* reads ``PATH`` to figure out which executable to run

## Bugs (signals)
* SIGINT
  * running ``rshell`` inside of ``rshell`` and sending ``SIGINT`` causes the prompt to be printed twice
* SIGTSTP / process handling
  * when backgrouding (bg) a process that requires input (such as ``cat``), they run simultaneously and seem to fight for input. In the case of cat, the first thing typed goes to cat and the second thing typed goes to ``rshell``.
* cd
  * lacks many features
* PATH
  * The command to execute is appended to each path and then checked for existence, so subfolders and executables within them may be executed

