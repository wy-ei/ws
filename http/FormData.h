//
// Created by dodo on 2020/7/3.
//

#ifndef WS_FORMDATA_H
#define WS_FORMDATA_H

#include <utility>

#include "comm.h"
#include "../base/Buffer.h"
#include "../base/except.h"


namespace ws{
namespace http{

using ws::base::Buffer;

struct FormDataItem {
    std::string name;
    std::string content;
    std::string filename;
    std::string content_type;
};

class FormData {
    friend std::ostream& operator<<(std::ostream& os, const FormData& form_data){
        os << "FormData:{\n";

        for(auto& item: form_data.form_){
            const FormDataItem &d = item.second;
            os << d.name << ':' << d.content << " - " << d.content_type << " - " << d.filename << '\n';
        }

        os << "}\n";
        return os;
    }
public:


    void add(const std::string& name, const std::string& value){
        FormDataItem item {name, value, "", ""};
        form_.emplace(name, std::move(item));
    }

    void add(const FormDataItem& item){
        form_.emplace(item.name, item);
    }

    void add(FormDataItem&& item){
        std::string name = item.name;
        form_.emplace(name, std::move(item));
    }

    void del_all(){
        form_.clear();
    }

    void del(const std::string& name){
        auto it = form_.find(name);
        if(it != form_.end()){
            form_.erase(it);
        }
    }

    bool has(const std::string& name){
        auto it = form_.find(name);
        return it != form_.end();
    }

    const FormDataItem& get(const std::string& name){
        auto it = form_.find(name);
        if(it != form_.end()){
            return it->second;
        }else{
            throw std::out_of_range("FormData KeyError");
        }
    }
private:
    std::unordered_map<std::string, FormDataItem> form_;
};



class MultipartFormDataParser {
    enum State {Boundary, Header, NewEntry, Body, Boundary_end, End};
public:
    explicit MultipartFormDataParser(FormData& form_data):form_data_(form_data){}

    void set_boundary(std::string boundary) { boundary_ = std::move(boundary); }
    bool is_valid() const { return is_valid_; }
    bool parse(const std::string& s);
    bool parse(Buffer& buffer);
private:
    std::string boundary_;
    State state_ {Boundary};
    bool is_valid_ = false;
    FormDataItem form_item_;
    FormData& form_data_;
};



} // end namespace http
} // namespace ws

#endif //WS_FORMDATA_H
