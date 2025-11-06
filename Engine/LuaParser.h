#pragma once

#include "LuaState.h"
#include "LuaLex.h"

#include <iostream>

namespace Engine
{
    class Parser
    {
    public:
        Parser(std::ifstream& inputStream)
            : lexer(inputStream)
        {
            
        }

        ProgramContext Parse()
        {
            current = lexer.NextToken();
            next = lexer.NextToken();
            while(current.token != TokenType::Eof)
            {    
                //ParseStatement();
                std::cout << current.toString() << std::endl;
                Advance();
            }

            context.Operations.push_back(OpCode::Exit);
            return context;
        }

    private:
        void ParseStatement()
        {
            if (current.token == TokenType::Identifier)
            {
                std::string id = std::get<std::string>(current.value);

                if (next.token == TokenType::Identifier && std::get<std::string>(next.value) == "(")
                {
                    ParseFunctionCall(id);
                }
            }
            else
            {
                throw std::runtime_error("语法错误：语句必须以标识符开头");
            }
        }

        void ParseFunctionCall(std::string& functionName)
        {
            Advance(); // 函数名
            Advance(); // 左括号

            // TODO: 参数获取解析
            // 1. 从全局变量表获得函数名
            // 2. 加载到调用栈上
            // 3. 加载参数到调用栈
            uint32_t globalIdx = GetGlobalIndex(functionName);
            context.Operations.push_back({OpCode::LoadGlobal, globalIdx});
            context.Operations.push_back({OpCode::LoadConst, 0});
            context.Operations.push_back({OpCode::Call, 0, 1});
            
        }

        void Advance()
        {
            current = next;
            next = lexer.NextToken();
        }

        uint32_t GetGlobalIndex(std::string& name)
        {
            uint32_t idx = 0;
            for(const auto& [ctxName, _] : context.Globals)
            {
                if (ctxName == name)
                {
                    return idx;
                }
                idx++;
            }

            if (idx == context.Globals.size())
            {
                context.Globals[name] = std::monostate{};
            }
            return static_cast<uint16_t>(context.Globals.size() - 1);
        }



    private:
        Lex lexer;
        Token current;
        Token next;

        ProgramContext context;
    };
}