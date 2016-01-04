# cxxtemplates
a little light-weght cpp text template library
*. variable replace
*. logic branch
*. loop structure


##examples##
```
    templates_text
        #include "{{file}}"
        struct {{struct}}{
            {{for field in fields}}
                {{field.type}} {{field.name|lowercase}}
                    {{!if field.array}}
                        [{{field.count}}];
                    {{!elif field.type = "message"}}
                        //elseif message
                    {{!else}}
                        //else
                    {{}}{{#end if}}
            {{}}{{#end for}}
        };
    render environment //env describe with json
        {"file": "test.h",
        "fields": [{"type":"string", "name":"Name","$idx":"0","array": 1,"count": 5,"length":32}],
        "struct":"test"},
        "@lowercase": 0x7f23e800}
    output:
        #include "test.h"
        struct test {
            string  name[5];
        };

```


