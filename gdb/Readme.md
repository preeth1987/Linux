																			GDB:GNU DEBUGGER


 
         A debugger is a program that runs other programs allowing users to exercise control over these programs and to examine variables when problems arise.GDB is most popular debugger for UNIX system.
 
BASIC FEATURES OF DEBUGGER:
 
*  To identify the statement or expression where program crashed
*  Values of program variables at a particular point during execution of program
*  Result of particular expression in a program
*  If an error occurs while executing a function, it can be used to identify the line of program  that contains call to that function and parameters to that function.
 
STARTING GDB:
 
          To  debug a particular program we indicate to cc(c compiler) by indicating -g flag to it.
ex:   cc  -g tree.c        where tree.c is a c file .This creates a.out executable file.We run this under the control of gdb by executing     gdb a.out  . This starts up text interface to the debugger.
 
GDB COMMANDS:
 
          When gdb starts program is not running . It wont run until we tell GDB to run it.
*run command-line-args
            starts  executing the program.
 
*file filename
            Reloads the debugging info.Here filename is name of the new file to be loaded or else GDB will keep trying to debug old program.
 
*break place
            Creates a breakpoint ; the program will halt when it gets there.
            We can create breakpoints at the start of a function like  break traverse . We can also set breakpoints at perticular line in a source file like break 20 to set breakpoint at line 20.
 
*delete N
            Remove breakpoints number N.
 
*list
            Prints the next 10 lines of source code to be executed.
 
*info break
            Give info about each breakpoint.It can be used to find local variables and args passed to the function like info locals and info args.
 
*help command
            Provides brief  description of a GDB command or topic.Plain help lists the possible topics.
 
*step
            Executes current line of a program and stops on the next statement to be executed.
 
*next
            Similar to step, however , if the current line of the program contains function call, it executes the function and stops at the next line.
 
*continue        
            Continues execution of the program until a brakpoint is hit or program stops.
 
*whatis var
            Prints the type of variable in a code.
 
*ptype name
            Used to print the structure.Here name can be structure of typedef  variable.
 
*print E
            Prints the value of E in the program , where E is a c expression(usually just a varible)
 
*set  varname=value
            It is used to set the variable to new value.
 
*backtrace
            This produces a list of the function calls, which is also known as stack trace(it is often used to know which function calls occured in order to get to the line where program crashed)
 
*whereas
            Lists the stack in reverse order(reverse order in which functions are called) since it is LIFO datastructure.It also gives the starting address of the function in memory and also function name.
 
*up
            Used to move control to next function.
 
*down
            Used to move control to previous function.
 
*quit
            Quit GDB.
 
*x/nfu  addr
 
 
Use the x command to examine memory.
n, f, and u are all optional parameters that specify how much memory to display and how to format it; addr is an expression giving the address where you want to start displaying memory. If you use defaults for nfu, you need not type the slash `/'. Several commands set convenient defaults for addr.
 
n, the repeat count
The repeat count is a decimal integer; the default is 1. It specifies how much memory (counting by units u) to display.
 
f, the display format
The display format is one of the formats used by print, `s' (null-terminated string), or `i' (machine instruction). The default is `x' (hexadecimal) initially. The default changes each time you use either x or print.
 
u, the unit size
The unit size is any of
 
b
Bytes.
h
Halfwords (two bytes).
w
Words (four bytes). This is the initial default.
g
Giant words (eight bytes).
Each time you specify a unit size with x, that size becomes the default unit the next time you use x. (For the `s' and `i' formats, the unit size is ignored and is normally not written.)
 
*set print pretty [on|off]
     To print the values formatted for better readability. 
 
*set pagination [on|off]
     Turn on or off pagination in case output spans multiple pages.
 
*commands
     This prompts for commands which can be executed when a break point is hit. 
