

int main(int argc, const char* argv[]) {

    const char* filename;
    const char* input = readFile(fileaname);
    
    Lexer lexer(input);
    Parser parser(lexer);
    
    bytecode = Parser.parse();

    VM vm;
    vm.interpret(bytecode);

    return 0;
}

