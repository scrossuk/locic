#!/bin/bash    
flex Lexer.l
bison -d -v Parser.y
