It's a simple realization of standard linux shell.

To make project:
    make

To execute project:
    make run

To clean:
    make clean


Simple Shell Language

<root> := <job> [ <terminal_sign> <job> ] { <terminal_sign> }

<job> := <conveyor> [ <condition_sign> <conveyor> ]

<conveyor> := <process> [ '|' <process> ]

<process> := <cmd> OR <sub_process>

<sub_process> := ( <root> ) <redirection>

<cmd> := <program_name> [ <cmd_append> ]

<cmd_append> := <arguments> or <redirection>

<arguments> := [ <argument> ]

<redirection> := [ <redirection_sign> <file_name> ]

<redirection_sign> := '<' OR '>' OR '>>'
<terminal_sign> := ';' OR '&'
<condition_sign> := '&&' OR '||'

<program_name> := <word>
<file_name> := <word>
<argument> := <word>

