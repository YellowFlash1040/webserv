#include <gtest/gtest.h>
#include "UploadModule.hpp"

// clang-format off
TEST(UploadModuleTest, test1)
{
    std::string input =
    "------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n"
    "Content-Disposition: form-data; name=\"files[]\"; filename=\"file.txt\"\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Some data inside txt file\nand a second line\n"
    "\r\n"
    "------WebKitFormBoundary7sXEyrliWNq0uCE6\r\n"
    "Content-Disposition: form-data; name=\"files[]\"; filename=\"file2.txt\"\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Data inside second file\n"
    "\r\n"
    "------WebKitFormBoundary7sXEyrliWNq0uCE6--";

    std::string boundary = "----WebKitFormBoundary7sXEyrliWNq0uCE6";

    std::vector<UploadModule::FormField> formFields;
    UploadModule::FormField field{};
    size_t pos = 0;
    while (extractFormField(input, boundary, pos, field))
        formFields.push_back(field);
}
