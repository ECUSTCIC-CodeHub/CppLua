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
            Value::function print_func = [](const std::vector<Value>& args) -> Value 
                {
                for (size_t i = 0; i < args.size(); ++i) {
                    try {
                        std::cout << static_cast<std::string>(args[i]);
                    }
                    catch (const std::bad_variant_access&) {
                        std::cout << "[unknown]";
                    }
                }
                std::cout << std::endl;
                return Value{}; // 返回空值（std::monostate）
                };
            context.Globals["print"] = Value(print_func);
        }

        ProgramContext Parse()
        {
            current = lexer.NextToken();
            next = lexer.NextToken();
            while(current.token != TokenType::Eof)
            {    
                ParseStatement();
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
                std::string id = static_cast<std::string>(current.value);

                if (current.token == TokenType::Identifier && next.token == TokenType::ParL)
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
            // 1. 从全局变量表获得函数名，保存到常量表中
            // 2. 加载函数名到调用栈上
            // 3. 参数加入到常量表中
            // 4. 加载到参数到调用栈上
            // 5. 调用函数
            Advance(); // 函数名
            Consume("(");

            std::vector<uint16_t> paramIndices;
            int globalIdx = GetGlobalIndex(functionName);
            context.Constants.push_back(functionName);
            context.Operations.push_back({ OpCode::LoadGlobal, context.Constants.size() - 1 });

            ParseExpression(paramIndices);

            Consume(")");

            context.Operations.push_back({OpCode::Call, globalIdx, 1});
        }

        void ParseExpression(std::vector<uint16_t>& paramIndices)
        {
            switch (current.token)
            {
            case TokenType::String:
            case TokenType::Number:
                paramIndices.push_back(GetConstantIndex(current.value));
                context.Operations.emplace_back(OpCode::LoadConst, paramIndices.back());
                Advance();
                break;
            default:
                throw std::runtime_error("语法错误：不支持的参数类型 " + current.toString());
            }
        }

        void Advance()
        {
            std::cout << current.toString() << std::endl;
            current = next;
            next = lexer.NextToken();
        }

        void Consume(const std::string& expectedStr)
        {
            if (static_cast<std::string>(current.value) == expectedStr)
            {
                Advance();
            }
            else
            {
                throw std::runtime_error(
                    "语法错误：预期 '" + expectedStr + "'，实际得到 " + current.toString()
                );
            }
        }

        uint16_t GetConstantIndex(const Value& val)
        {
            auto it = std::find(context.Constants.begin(), context.Constants.end(), val);
            if (it != context.Constants.end())
            {
                return static_cast<uint16_t>(std::distance(context.Constants.begin(), it));
            }

            if (context.Constants.size() >= UINT16_MAX)
                throw std::runtime_error("常量表溢出");
            uint16_t idx = static_cast<uint16_t>(context.Constants.size());
            context.Constants.push_back(val);
            return idx;
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