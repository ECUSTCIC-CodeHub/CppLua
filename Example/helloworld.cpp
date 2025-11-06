#include <iostream>
#include <Windows.h>
#include "LuaParser.h"

int main() 
{
    system("chcp 65001");
    std::ifstream fs;
    fs.open("D:/Code/Learn/mylua/CppLua/Example/test.lua");

    Engine::Parser parser(fs);
    
    parser.Parse();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}