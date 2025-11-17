#include <iostream>
#include <Windows.h>
#include "LuaVM.h"
int main()
{
    //system("chcp 65001");
    try {
        std::cout << "Creating VM..." << std::endl;
        Engine::VM vm("../Example/comprehensive_test.lua");
        std::cout << "Executing..." << std::endl;
        vm.Execute();
        std::cout << "Done!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}