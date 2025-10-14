
#include <gtest/gtest.h>
#include "CGI.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>

static std::string make_temp_dir()
{
    std::string tmpl = "/tmp/cgi_testXXXXXX";
    // mkdtemp modifies the buffer and returns pointer on success
    char *buf = &tmpl[0];
    char *res = mkdtemp(buf);
    if (!res) throw std::runtime_error("mkdtemp failed");
    return std::string(res);
}

static std::string write_script(const std::string &dir, const std::string &name, const std::string &content)
{
    std::string path = dir + "/" + name;
    std::ofstream ofs(path.c_str(), std::ios::binary);
    if (!ofs) throw std::runtime_error("Failed to create script file: " + path);
    ofs << content;
    ofs.close();
    // make executable by owner
    if (chmod(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
        unlink(path.c_str());
        throw std::runtime_error("chmod failed for: " + path);
    }
    return path;
}

static void cleanup_file_and_dir(const std::string &file, const std::string &dir)
{
    unlink(file.c_str());
    rmdir(dir.c_str());
}

// Test 1: GET-like script: prints header + fixed body
TEST(CGI, CgiExecute_GetOutput)
{
    std::string tmpdir = make_temp_dir();
    try {
        // script prints headers and a static body "HelloFromCGI"
        std::string script_content =
            "#!/bin/sh\n"
            "printf 'Content-Type: text/plain\\r\\n\\r\\n'\n"
            "printf 'HelloFromCGI'\n";
        std::string script = write_script(tmpdir, "test_get.sh", script_content);

        std::vector<std::string> args; // unused
        std::vector<std::string> env = {
            "REQUEST_METHOD=GET",
            "QUERY_STRING=",
            "CONTENT_LENGTH=0"
        };

        // rootDir must match the directory that contains the script
        std::string raw = CGI::execute(script, args, env, "", tmpdir);

        // Should contain header separator and the body
        EXPECT_NE(raw.find("Content-Type: text/plain\r\n\r\n"), std::string::npos);
        EXPECT_NE(raw.find("HelloFromCGI"), std::string::npos);

        cleanup_file_and_dir(script, tmpdir);
    } catch (...) {
        // best effort cleanup on exception
        // remove all files in tmpdir (only our single file) and rmdir
        // we try to rmdir anyway
        rmdir(tmpdir.c_str());
        throw;
    }
}

// Test 2: POST-like script: prints header then echoes stdin
TEST(CGI, CgiExecute_PostEcho)
{
    std::string tmpdir = make_temp_dir();
    try {
        // script prints headers then cats stdin to output
        std::string script_content =
            "#!/bin/sh\n"
            "printf 'Content-Type: text/plain\\r\\n\\r\\n'\n"
            "cat -\n";
        std::string script = write_script(tmpdir, "test_post.sh", script_content);

        std::vector<std::string> args;
        std::string body = "name=Alice&age=30";
        std::vector<std::string> env = {
            "REQUEST_METHOD=POST",
            "QUERY_STRING=",
            "CONTENT_LENGTH=" + std::to_string(body.size())
        };

        std::string raw = CGI::execute(script, args, env, body, tmpdir);

        // header present
        EXPECT_NE(raw.find("Content-Type: text/plain\r\n\r\n"), std::string::npos);
        // body echoed
        EXPECT_NE(raw.find(body), std::string::npos);

        cleanup_file_and_dir(script, tmpdir);
    } catch (...) {
        rmdir(tmpdir.c_str());
        throw;
    }
}

// Test 3: script outside of rootDir -> expect exception (path traversal protection)
TEST(CGI, CgiExecute_PathTraversal)
{
    std::string tmpdir = make_temp_dir();
    try {
        // create a script in tmpdir
        std::string script_content =
            "#!/bin/sh\n"
            "printf 'Content-Type: text/plain\\r\\n\\r\\n'\n"
            "printf 'ShouldNotRun'\n";
        std::string script = write_script(tmpdir, "evil.sh", script_content);

        std::vector<std::string> args;
        std::vector<std::string> env = {
            "REQUEST_METHOD=GET",
            "QUERY_STRING=",
            "CONTENT_LENGTH=0"
        };

        // rootDir is deliberately different (parent directory), so validateScriptPath should reject
        std::string otherRoot = "/var/www/cgi-bin"; // unlikely equals tmpdir

        // We expect execute to throw due to path validation (or execve failure). Use EXPECT_THROW.
        EXPECT_THROW({
            CGI::execute(script, args, env, "", otherRoot);
        }, std::exception);

        cleanup_file_and_dir(script, tmpdir);
    } catch (...) {
        rmdir(tmpdir.c_str());
        throw;
    }
}
