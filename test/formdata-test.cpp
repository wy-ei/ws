//
// Created by wangyu on 2020/7/3.
//

#include "../utils/test.h"
#include "../log/logging.h"
#include "../http/FormData.h"

using ws::TEST;
using ws::assert_true;

using namespace ws::base;

int formdata_test(){
//    ws::logging::set_level(ws::logging::INFO);

    TEST("formdata", []{
        ws::http::FormData form_data;
        Buffer buffer;

        std::string s = R"__(------WebKitFormBoundaryeOeWcRvkpBvpFDtA
Content-Disposition: form-data; name="foo"

123
------WebKitFormBoundaryeOeWcRvkpBvpFDtA
Content-Disposition: form-data; name="bar"

234
--
------WebKitFormBoundaryeOeWcRvkpBvpFDtA--
)__";

        ws::http::MultipartFormDataParser parser(form_data);
        parser.set_boundary("----WebKitFormBoundaryeOeWcRvkpBvpFDtA");

        int i = 0;
        while(i < s.size()){
            buffer.append(&s[i], 1);
            i++;
            bool finished = parser.parse(buffer);
            if(finished){
                LOG_INFO << "finished";
                break;
            }
        }

        LOG_DEBUG << '\n' << " finished: " << " is_valid " << parser.is_valid() << form_data;
    });
}

int main_formdata_test(int argc, char* argv[]){
    //ws::logging::start_async_backend(argv[0]);

    formdata_test();

}