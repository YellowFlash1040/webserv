// #include <gtest/gtest.h>
// #include "UploadModule.hpp"

// // clang-format off
// TEST(UploadModuleTest, test1)
// {
//     std::string body =
//     "------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n"
//     "Content-Disposition: form-data; name=\"files[]\"; filename=\"file.txt\"\r\n"
//     "Content-Type: text/plain\r\n"
//     "\r\n"
//     "Some data inside txt file\nand a second line\n"
//     "\r\n"
//     "------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n"
//     "Content-Disposition: form-data; name=\"files[]\"; filename=\"file2.txt\"\r\n"
//     "Content-Type: text/plain\r\n"
//     "\r\n"
//     "Data inside second file\n"
//     "\r\n"
//     "------WebKitFormBoundary7sXEyrliWNq0uCE6--";

//     std::string boundary = "----WebKitFormBoundary7sXEyrliWNq0uCE6";

//     UploadModule::parseFormData(body, boundary);
// }
