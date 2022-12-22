# sish
A Simple $hell

# Support
Pipeline
IO redirection
Backgrounding

# Builtins
echo
cd
exit

# Flags
[-x] prints command
[-c] execute command

# Details
Why Lex and Yacc?
Shell commandline language follows a context free grammar and thus it is correct to use a LALR parse generator.
And using a configurable parser generator made it alot easier for me to add new features later on.

See discussion in:
https://cs61.seas.harvard.edu/site/2022/BNFGrammars/

Usage
$ make
$ ./sh
