#pragma once

#include "LuaState.h"
#include "LuaLex.h"

namespace Engine
{
    class Parser
    {
    public:
        Parser(std::ifstream& inputStream)
            : lexer(inputStream)
        {
            current = lexer.NextToken();
            next = lexer.NextToken();
        }

        ProgramContext Parse()
        {
            while(current.token != TokenType::Eof)
            {
                ParseStatement();
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
            Advance();
            Advance();

            uint32_t globalIdx = GetGlobalIndex(functionName);
            
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