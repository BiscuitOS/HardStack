.macro [中文教程](https://biscuitos.github.io/blog/GNUASM-.macro/)
--------------------------------------------------

## .macro macname macargs ...

Begin the definition of a macro called macname. If your macro 
definition requires arguments, specify their names after the 
macro name, separated by commas or spaces. You can supply a 
default value for any macro argument by following the name with 
‘=deflt’. For example, these are all valid .macro statements:

```
  .macro comm           Begin the definition of a macro called comm, 
                        which takes no arguments.
  .macro plus1 p, p1
  .macro plus1 p p1     Either statement begins the definition of a 
                        macro called plus1, which takes two arguments; 
                        within the macro definition, write ‘\p’ or 
                        ‘\p1’ to evaluate the arguments.
  .macro reserve_str p1=0 p2
                        Begin the definition of a macro called 
                        reserve_str, with two arguments. The first 
                        argument has a default value, but not the 
                        second. After the definition is complete, 
                        you can call the macro either as ‘reserve_str 
                        a,b’ (with ‘\p1’ evaluating to a and ‘\p2’ 
                        evaluating to b), or as ‘reserve_str ,b’ (with 
                        ‘\p1’ evaluating as the default, in this case 
                        ‘0’, and ‘\p2’ evaluating to b).
```

When you call a macro, you can specify the argument values either by 
position, or by keyword. For example, ‘sum 9,17’ is equivalent to 
‘sum to=17, from=9’.

