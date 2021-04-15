//
// Created by wangyu on 2020/7/3.
//

#include <gtest/gtest.h>

#include "log/logging.h"
#include "http/FormData.h"

using namespace ws::base;
using namespace ws::http;

TEST(FormData, formdata){
    FormData form_data;
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
    buffer.append(s.data(), s.size());
    EXPECT_TRUE(parser.parse(buffer));

    FormDataItem item = form_data.get("name");

    LOG_DEBUG << item.content;

    EXPECT_TRUE(true);

}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}