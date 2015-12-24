# cxxtemplates
a little light-weght cpp text template library
*. variable replace
*. logic branch
*. loop structure


##examples##
```
    #include "{{file}}"
    struct {{struct}}{
        {{for field in fields}}
            {{field.type}} {{field.name}}
                {{if field.is_repeated}}
                    [{{field.count}}];
                {{}}
        {{}}
    };
```


