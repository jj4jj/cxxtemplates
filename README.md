# cxxtemplates
a little light-weght cpp text template library

 * variable replace
 * logic branch
 * loop structure


##examples##
```
#ifndef __PBDCEX_H_DEF_{{file_name|unif}}__
#define __PBDCEX_H_DEF_{{file_name|unif}}__
#pragma once
//This file is auto generated by pbdcex [http://github.com/jj4jj/pbdcex.git],
//Please do NOT edit it ! Any bug please let jj4jj known .
//File generated time: {{time}}

{{!for vd in depencies}}
#include "{{vd}}"
{{}}
#include "pbdcex.core.hpp"
{{ns_begin}}
using pbdcex::string_t;
using pbdcex::bytes_t;
using pbdcex::array_t;
using pbdcex::serializable_t;
{{!for vmsg in msgs}}
struct {{vmsg.meta|cs.msg.name}} : public serializable_t<{{vmsg.meta|cs.msg.name}}, {{vmsg.meta|msg.name}}> {
    {{!for vf in vmsg.fields}}{{vf.meta|cs.field.wtype}}       {{vf.meta|cs.field.name}};
    {{}}////////////////////////////////////////////////////////////////////////
    void    construct(){
        if(sizeof(*this) < 4*1024){
            memset(this, 0, sizeof(*this));
        }
        else {
            {{!for vf in vmsg.fields}}
            {{!if vf.meta|field.is_array}}
            {{vf.meta|cs.field.name}}.construct();
            {{!elif vf.meta|field.is_num}}
            {{vf.meta|cs.field.name}}=static_cast<{{vf.meta|cs.field.type}}>(0);
            {{!elif vf.meta|field.is_bool}}
            {{vf.meta|cs.field.name}}=false;
            {{!else}}
            {{vf.meta|cs.field.name}}.construct();
            {{}}
            {{}}
        }
    }
    void	convto({{vmsg.meta|msg.name}} & tomsg_) const {
        tomsg_.Clear();
        {{!for vf in vmsg.fields}}
        {{!if vf.meta|field.is_array}}
        for (size_t i = 0;i < {{vf.meta|field.name}}.count; ++i){
            {{!if vf.meta|field.is_msg}}
            this->{{vf.meta|cs.field.name}}.list[i].convto(*tomsg_.add_{{vf.meta|field.name}}());
            {{!elif vf.meta|field.is_bytes }}
            tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i].data, this->{{vf.meta|cs.field.name}}.list[i].length);
            {{!elif vf.meta|field.is_string }}
            tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i].data);
            {{!else}}
            tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i]);
            {{}}}
        {{!else}}
        {{!if vf.meta|field.is_msg}}
        this->{{vf.meta|cs.field.name}}.convto(*tomsg_.mutable_{{vf.meta|field.name}}());
        {{!elif vf.meta|field.is_bytes}}
        tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.data, this->{{vf.meta|cs.field.name}}.length);
        {{!elif vf.meta|field.is_string}}
        tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.data);
        {{!else}}tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}});
        {{}}
        {{}}
        {{}}
    }
    void convfrom(const {{vmsg.meta|msg.name}} & frommsg_) {
        {{!for vf in vmsg.fields}}
        {{!if vf.meta|field.is_array}}
        this->{{vf.meta|cs.field.name}}.count=0;
        for (size_t i = 0; i < (size_t)frommsg_.{{ vf.meta | field.name }}_size() && i < {{ vf.meta | field.count }}; ++i, ++(this->{{ vf.meta | cs.field.name }}.count)){
            {{!if vf.meta|field.is_msg}}
            this->{{vf.meta|cs.field.name}}.list[i].convfrom(frommsg_.{{vf.meta|field.name}}(i));
            {{!elif vf.meta|field.is_bytes}}
            assert(frommsg_.{{vf.meta|field.name}}(i).length() <= {{vf.meta|field.length}});
            this->{{ vf.meta | cs.field.name }}.list[i].assign(frommsg_.{{ vf.meta | field.name }}(i));
            {{ !elif vf.meta | field.is_string }}
            assert(frommsg_.{{ vf.meta | field.name }}(i).length() < {{ vf.meta | field.length }});
            this->{{ vf.meta | cs.field.name }}.list[i].assign(frommsg_.{{ vf.meta | field.name }}(i).data());
            {{!else}}
            this->{{vf.meta|cs.field.name}}.list[i] = frommsg_.{{vf.meta|field.name}}(i);
            {{}}
        }
        {{!else}}
        {{!if vf.meta|field.is_msg}}
        this->{{vf.meta|cs.field.name}}.convfrom(frommsg_.{{vf.meta|field.name}}());
        {{ !elif vf.meta | field.is_bytes }}
        assert(frommsg_.{{ vf.meta | field.name }}().length() <= {{ vf.meta | field.length }});
        this->{{vf.meta|cs.field.name}}.assign(frommsg_.{{vf.meta|field.name}}());
        {{ !elif vf.meta | field.is_string }}
        assert(frommsg_.{{ vf.meta | field.name }}().length() < {{ vf.meta | field.length }});
        this->{{vf.meta|cs.field.name}}.assign(frommsg_.{{vf.meta|field.name}}().data());
        {{!else}}
        this->{{vf.meta|cs.field.name}} = frommsg_.{{vf.meta|field.name}}();
        {{}}
        {{}}
        {{}}
    }
    int     check_convfrom(const {{vmsg.meta|msg.name}} & frommsg_) const {
        int ret = 0;
        {{!for vf in vmsg.fields}}
        {{!if vf.meta|field.is_array}}
        if ((size_t)frommsg_.{{ vf.meta | field.name }}_size() > {{ vf.meta | field.count }}){ return __LINE__; }

        for (size_t i = 0; i < (size_t)frommsg_.{{ vf.meta | field.name }}_size() && i < {{ vf.meta | field.count }}; ++i){
            {{!if vf.meta|field.is_msg}}
            ret = this->{{vf.meta|cs.field.name}}.list[i].check_convfrom(frommsg_.{{vf.meta|field.name}}(i));
            if (ret) {return ret;}
            {{!elif vf.meta|field.is_bytes}}
            if (frommsg_.{{vf.meta|field.name}}(i).length() > {{vf.meta|field.length}}){ return __LINE__; }
            {{ !elif vf.meta | field.is_string }}
            if (frommsg_.{{ vf.meta | field.name }}(i).length() >= {{ vf.meta | field.length }}){ return __LINE__; }
            {{!else}}
            //
            {{}}
        }
        {{!else}}
        {{!if vf.meta|field.is_msg}}
        ret = this->{{vf.meta|cs.field.name}}.check_convfrom(frommsg_.{{vf.meta|field.name}}());
        if (ret) { return ret; }
        {{ !elif vf.meta | field.is_bytes }}
        if (frommsg_.{{ vf.meta | field.name }}().length() > {{ vf.meta | field.length }}){ return __LINE__; }
        {{ !elif vf.meta | field.is_string }}
        if (frommsg_.{{ vf.meta | field.name }}().length() >= {{ vf.meta | field.length }}){ return __LINE__; }
        {{!else}}
        //
        {{}}
        {{}}
        {{}}
        return ret;
    }
    void     diff(const {{ vmsg.meta | cs.msg.name }} & orgv , {{ vmsg.meta | msg.name }} & updates) const {
        updates.Clear();
        {{!for vf in vmsg.fields}}
        {{!if vf.meta | field.is_array }}
        {//block of checking array differ
            decltype(this->{{vf.meta | cs.field.name}}.count) i = 0;
            for (i = 0;i < this->{{vf.meta | cs.field.name}}.count; ++i) {
                auto upd_ = updates.mutable_{{vf.meta | field.name}}()->Add();
                {{!if vf.meta | field.is_msg }}
                if (i < orgv.{{vf.meta | cs.field.name}}.count) {
                    this->{{vf.meta | cs.field.name}}.list[i].diff(orgv.{{vf.meta | cs.field.name}}.list[i], *upd_);
                }
                else {
                    this->{{vf.meta | cs.field.name}}.list[i].convto(*upd_);
                }
                {{!elif vf.meta | field.is_string }}
                upd_->assign(this->{{vf.meta | cs.field.name}}.list[i].data);
                {{!elif vf.meta | field.is_bytes }}
                upd_->assign((const char*)this->{{vf.meta | cs.field.name}}.list[i].data, this->{{vf.meta | cs.field.name}}.list[i].length);
                {{!else}}
                *upd_ = this->{{vf.meta | cs.field.name}}.list[i];
                {{}}
            }            
        }//end with array diff block 
        {{!elif vf.meta | field.is_msg}}
        this->{{vf.meta | cs.field.name}}.diff(orgv.{{vf.meta | cs.field.name}}, *updates.mutable_{{ vf.meta | field.name }}());
        {{!else}}
        {{!if vf.meta | field.is_required}}
        {{!if vf.meta | field.is_string }}
        updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }}.data);
        {{!elif vf.meta | field.is_bytes}}
        updates.set_{{ vf.meta | field.name }}((char*)this->{{ vf.meta | cs.field.name }}.data, this->{{ vf.meta | cs.field.name }}.length);
        {{!else}}
        updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }});
        {{}}
        {{!else}}
        if (!(this->{{vf.meta | cs.field.name}} == orgv.{{vf.meta | cs.field.name}})) {
            {{!if vf.meta | field.is_string }}
            updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }}.data);
            {{!elif vf.meta | field.is_bytes}}
            updates.set_{{ vf.meta | field.name }}((char*)this->{{ vf.meta | cs.field.name }}.data, this->{{ vf.meta | cs.field.name }}.length);
            {{!else}}
            updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }});
            {{}}
        }
        {{}}
        {{}}
        {{}}
    }
    void     patch(const {{ vmsg.meta | msg.name }} & updates) {
        {{!for vf in vmsg.fields}}
        {{!if vf.meta | field.is_array }}
        {//block of checking array patch
            static {{vf.meta | cs.field.scalar_type}}    item_temp;
            for (int i = 0; i < updates.{{vf.meta | field.name}}_size(); ++i) {
                if (i < (int)this->{{vf.meta | cs.field.name}}.count) {
                    {{!if vf.meta | field.is_msg }}
                    this->{{vf.meta | cs.field.name}}.list[i].patch(updates.{{vf.meta | field.name}}(i));    
                    {{!elif vf.meta | field.is_string }}
                    this->{{vf.meta | cs.field.name}}.list[i].assign(updates.{{vf.meta | field.name}}(i));
                    {{!elif vf.meta | field.is_bytes }}
                    this->{{vf.meta | cs.field.name}}.list[i].assign(updates.{{vf.meta | field.name}}(i));
                    {{!else}}
                    this->{{vf.meta | cs.field.name}}.list[i] = updates.{{vf.meta | field.name}}(i);
                    {{}}
                }
                else {
                    {{!if vf.meta | field.is_msg }}
                    item_temp.convfrom(updates.{{vf.meta | field.name}}(i));
                    {{!elif vf.meta | field.is_string }}
                    item_temp.assign(updates.{{vf.meta | field.name}}(i));
                    {{!elif vf.meta | field.is_bytes }}
                    item_temp.assign(updates.{{vf.meta | field.name}}(i));
                    {{!else}}
                    item_temp = updates.{{vf.meta | field.name}}(i);
                    {{}}
                    this->{{vf.meta | cs.field.name}}.lappend(item_temp);
                }
            }//end for append or update
        }//end with array diff block 
        {{!elif vf.meta | field.is_msg}}
        this->{{vf.meta | cs.field.name}}.patch(updates.{{ vf.meta | field.name }}());
        {{!else}}
        {{!if vf.meta | field.is_required}}
        {{!if vf.meta | field.is_string }}
        this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());
        {{!elif vf.meta | field.is_bytes}}
        this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());
        {{!else}}
        this->{{ vf.meta | cs.field.name }} = updates.{{ vf.meta | field.name }}();
        {{}}
        {{!else}}
        if (updates.has_{{vf.meta | field.name}}()) {
            {{!if vf.meta | field.is_string }}
            this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());
            {{!elif vf.meta | field.is_bytes}}
            this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());
            {{!else}}
            this->{{ vf.meta | cs.field.name }} = updates.{{ vf.meta | field.name }}();
            {{}}
        }
        {{}}
        {{}}
        {{}}
    }
    int     compare(const {{vmsg.meta|cs.msg.name}} & rhs_) const {
        int cmp = 0;
        {{!for vf in vmsg.pkfields}}
        {{!if vf.meta|field.is_array}}
        cmp = this->{{vf.meta|cs.field.name}}.compare(rhs_.{{vf.meta|cs.field.name}});
        if(cmp){return cmp;}
        {{!elif vf.meta|field.is_msg}}
        cmp = this->{{vf.meta|cs.field.name}}.compare(rhs_.{{vf.meta|cs.field.name}});
        if(cmp){return cmp;}
        {{!else}}
        if (this->{{vf.meta|cs.field.name}} < rhs_.{{vf.meta|cs.field.name}}){
            return -1;
        }
        else if(this->{{vf.meta|cs.field.name}} > rhs_.{{vf.meta|cs.field.name}}){
            return 1;
        }
        {{}}
        {{}}
        return cmp;
    }
    bool    operator == (const {{vmsg.meta|cs.msg.name}} & rhs_) const {
        return this->compare(rhs_) == 0;
    }
    bool    operator < (const {{vmsg.meta|cs.msg.name}} & rhs_) const {
        return this->compare(rhs_) < 0;
    }
    size_t hash() const {
        {{!if vmsg.pkfields_num < 2}}
        {{!for vf in vmsg.pkfields}}
        {{!if vf.meta|field.is_array}}
        return this->{{vf.meta|cs.field.name}}.hash();
        {{!elif vf.meta|field.is_num}}
        return (size_t)(this->{{vf.meta|cs.field.name}});
        {{!else}}
        return this->{{vf.meta|cs.field.name}}.hash();
        {{}}
        {{}}
        {{!else}}
        size_t avhs[] = {
        {{!for vf in vmsg.pkfields}}
        {{!if vf.meta|field.is_array}}
            this->{{vf.meta|cs.field.name}}.hash(),
        {{!elif vf.meta|field.is_num}}
            (size_t)(this->{{vf.meta|cs.field.name}}),
        {{!else}}
            this->{{vf.meta|cs.field.name}}.hash(),
        {{}}
        {{}}
        };
        return pbdcex::hash_code_merge_multi_value(avhs, {{vmsg.pkfields_num}});
        {{}}
    }
};
{{}}
{{ns_end}}
#endif
```


