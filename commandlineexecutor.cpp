#include "commandlineexecutor.h"
#include <QString>

CommandLineExecutor::CommandLineExecutor()
{
    if(GenerateCmake){
        // generate cmake object in another class -> invoke
    }
}



std::string CommandLineExecutor::ExecuteCommand(std::string cmd) {

  std::string data;
  FILE * stream;
  const int max_buffer = 256;
  char buffer[max_buffer];
  // ????
  //cmd.append(" 2>&1");

  stream = popen(cmd.c_str(), "r");
  if (stream) {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL)
          data.append(buffer);
    pclose(stream);
  }
  return data;
}

/* compiler
------------------------------------------------------------------------ */

void CommandLineExecutor::setCompiler(const std::string &compiler, const BuildMode &mode, const std::string &addflags){

    switch (mode) {
        case Debug:
            flags += "-O0 -g";
            break;
        case Release:
            flags += "-O3";
            break;
        default: break;
    }
    //DetermineCompilerVersion(compiler);
    compile_args += compiler + version + " " + flags + " " + addflags;
}

void CommandLineExecutor::setExecutableName(const std::string &name, const std::string &path){
    executable_name = name;
    executable_path = path;
    compile_args += " -o " + executable_path + name;
}

void CommandLineExecutor::setSourceFiles(const std::vector<std::string> &sources){

    for (unsigned int i = 0; i < sources.size(); i++) {
        compile_args += " " + sources[i];
    }
}

void CommandLineExecutor::setLibraryPaths(const std::vector<std::string> &library_paths){

    compile_args += "-L ";
    for (unsigned int i = 0; i < library_paths.size(); i++) {
        compile_args += " " + library_paths[i];
    }

}

void CommandLineExecutor::DetermineCompilerVersion(const std::string &tool){
    // find version in linux ( ls /usr/bin | grep -i "some_tool[-]" ----->>> parse, get version )
    std::string temp = ExecuteCommand("ls /usr/bin | grep -i '" + tool + "-'"  );
    // version = temp[end???];
}

std::string CommandLineExecutor::Build(bool cmake){

    std::string output = "";
    if(cmake){
        cmake_build += "mkdir " + ProjectRootDir + "cmake-build && cd " + ProjectRootDir + "cmake-build " +
                " && cmake .. && make -j2";
        output = ExecuteCommand(cmake_build);
    }
    else{
        // string will be returned into console
        output = ExecuteCommand(compile_args);
    }

    return output;
}
std::string CommandLineExecutor::Execute(bool cmake){

    std::string output = "";
    if(cmake){
        cmake_exec += "cd " + ProjectRootDir + "cmake-build" + " && ./" + executable_name;
        output = ExecuteCommand(cmake_exec);
    }
    // string will be returned into console
    executable_name = executable_path + executable_name;
    output = ExecuteCommand(executable_name);

    return output;
}

/* clang tools
------------------------------------------------------------------------ */

std::string CommandLineExecutor::ClangFormat(const std::vector<std::string> &sources){
    std::string clang_format_args = "/usr/bin/clang-format ";
    std::string base_flags = "";

    for (unsigned int i = 0; i < sources.size(); i++) {
        clang_format_args += " " + sources[i];
    }
    clang_format_args += base_flags;
    std::string output = ExecuteCommand(clang_format_args);

    return output;
}

std::string CommandLineExecutor::ClangCheck(const std::vector<std::string> &sources){
    std::string clang_check_args = "/usr/bin/clang-check ";
    std::string base_flags = "";

    for (unsigned int i = 0; i < sources.size(); i++) {
        clang_check_args += " " + sources[i];
    }
    clang_check_args += base_flags;
    std::string output = ExecuteCommand(clang_check_args);

    return output;
}

std::string CommandLineExecutor::ClangTidy(const std::vector<std::string> &sources){
    std::string clang_tidy_args = "/usr/bin/clang-tidy ";
    std::string base_flags = "";

    for (unsigned int i = 0; i < sources.size(); i++) {
        clang_tidy_args += " " + sources[i];
    }
    clang_tidy_args += base_flags;
    std::string output = ExecuteCommand(clang_tidy_args);

    return output;
}

/* cother tools
------------------------------------------------------------------------ */

std::string CommandLineExecutor::Valgrind(){
    std::string valgrind_args = "/usr/bin/valgrind --tool=memcheck ./" + executable_name;
    std::string base_flags = "";

    valgrind_args += base_flags;
    std::string output = ExecuteCommand(valgrind_args);

    return output;
}

void CommandLineExecutor::OpenGdbGui(){
    ExecuteCommand("/usr/bin/gdbgui ./" + executable_name);
}
