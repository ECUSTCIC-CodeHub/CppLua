#include <iostream>
#include <Windows.h>
#include "LuaVM.h"
int main() 
{
    //system("chcp 65001");
    Engine::VM vm("C:/Code/cppLua/Example/test.lua");
    vm.Excute();
    return 0;
}