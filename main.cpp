#include <iostream>
#include <fstream>
#include <sstream>

#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include "stmt.h"
#include "vm.h"

static void repl() {
    std::string line;
    for (;;) {
        std::cout << "> ";

        std::getline(std::cin, line);

        if (line.empty()) {
            printf("\n");
            break;
        }

        std::cout << line << std::endl;
    }
}

static std::string read_file(const char* path) {
    std::ifstream file(path);

    if (file) {
        std::ostringstream ss;

        ss << file.rdbuf();
        std::string file_contents = ss.str();

        return file_contents;
    } else {
        std::cerr << "Could not open file " << "\"" << path << "\"." << std::endl;
        exit(74);
    }
}

static void compile_file(const char* path) {
    std::string source = read_file(path);

    Scanner scanner = Scanner(source);
    std::vector<Token> tokens = scanner.scan_tokens();

    Parser parser = Parser(tokens);
    std::vector<StmtPtr> stmts = parser.parse();

    Compiler compiler = Compiler(stmts);
    compiler.compile();

    VM vm = VM();
    vm.run();

    //if (result == COMPILER_RESULT_ERROR) exit(65);
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        compile_file(argv[1]);
    } else {
        std::cerr << "Usage: tessera [path]" << std::endl;
        exit(64);
    }
    return 0;
}
