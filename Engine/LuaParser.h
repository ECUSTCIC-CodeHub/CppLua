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

                if (next.token == TokenType::ParL)
                {
                    Advance(); // 跳过函数名
                    ParseFunctionCall(id);
                }
                else if (next.token == TokenType::Assign)
                {
                    Advance(); // 跳过变量名
                    ParseAssignment(id);
                }
                else
                {
                    throw std::runtime_error("语法错误：标识符后需要括号或赋值符号 " + current.toString());
                }
            }
            else if (current.token == TokenType::Local)
            {
                ParseLocalDeclaration();
            }
            else if (current.token == TokenType::SemiColon)
            {
                // 空语句，直接跳过
                Advance();
                return;
            }
            else
            {
                throw std::runtime_error("语法错误：不支持的语句开始类型 " + current.toString());
            }

            // 对于非空语句，确保在语句结束后推进到下一个token
            // 处理可能存在的分号
            if (current.token == TokenType::SemiColon)
            {
                Advance();
            }
        }

        void ParseFunctionCall(std::string& functionName)
        {
            // 1. 从全局变量表获得函数名，保存到常量表中
            // 2. 加载函数名到调用栈上
            // 3. 参数加入到常量表中
            // 4. 加载到参数到调用栈上
            // 5. 调用函数
            Consume("(");  // 消耗函数名后的左括号

            std::vector<uint16_t> paramIndices;
            int globalIdx = GetGlobalIndex(functionName);
            context.Constants.push_back(functionName);
            context.Operations.push_back({ OpCode::LoadGlobal, context.Constants.size() - 1 });

            ParseExpression(paramIndices);

            Consume(")");  // 消耗右括号

            context.Operations.push_back({OpCode::Call, globalIdx, 1});
        }

        void ParseExpression(std::vector<uint16_t>& paramIndices)
        {
            // 首先解析左操作数
            switch (current.token)
            {
            case TokenType::String:
            case TokenType::Number:
            case TokenType::True:
            case TokenType::False:
            case TokenType::Nil:
                paramIndices.push_back(GetConstantIndex(current.value));
                context.Operations.emplace_back(OpCode::LoadConst, paramIndices.back());
                Advance();
                break;
            case TokenType::Identifier:
            {
                std::string varName = static_cast<std::string>(current.value);
                paramIndices.push_back(GetConstantIndex(varName));
                // TODO: 实现变量加载
                Advance();
                break;
            }
            case TokenType::ParL:
            {
                Advance(); // 跳过 (
                ParseExpression(paramIndices);
                Consume(")"); // 消耗 )
                break;
            }
            default:
                throw std::runtime_error("语法错误：不支持的表达式类型 " + current.toString());
            }

            // 检查是否有运算符
            if (current.token == TokenType::Add || current.token == TokenType::Sub ||
                current.token == TokenType::Mul || current.token == TokenType::Div) {
                // 暂时跳过运算符，简单处理为只加载左操作数
                TokenType op = current.token;
                Advance(); // 跳过运算符
                // 递归解析右操作数（目前简化处理）
                std::vector<uint16_t> rightParam;
                ParseExpression(rightParam);
                // TODO: 实现实际的运算操作
            }
        }

        uint16_t ParseExpression()
        {
            switch (current.token)
            {
            case TokenType::String:
            case TokenType::Number:
            case TokenType::True:
            case TokenType::False:
            case TokenType::Nil:
            {
                uint16_t constIndex = GetConstantIndex(current.value);
                context.Operations.emplace_back(OpCode::LoadConst, constIndex);
                Advance();
                return constIndex;
            }
            case TokenType::Identifier:
            {
                std::string varName = static_cast<std::string>(current.value);
                uint16_t constIndex = GetConstantIndex(varName);
                // TODO: 实现变量加载
                Advance();
                return constIndex;
            }
            case TokenType::ParL:
            {
                Advance(); // 跳过 (
                uint16_t result = ParseExpression();
                Consume(")"); // 消耗 )
                return result;
            }
            // TODO: 添加更多表达式类型（运算符、函数调用等）
            default:
                throw std::runtime_error("语法错误：不支持的表达式类型 " + current.toString());
            }
        }

        void ParseAssignment(const std::string& varName)
        {

            Consume("="); // 跳过赋值符号

            // 解析右侧表达式
            uint16_t valueIndex = ParseExpression();

            // TODO: 实现赋值操作码，暂时跳过实现
            std::cout << "赋值语句: " << varName << " = " << valueIndex << std::endl;
        }

        void ParseLocalDeclaration()
        {
            Advance(); // 跳过 local 关键字

            if (current.token != TokenType::Identifier)
            {
                throw std::runtime_error("语法错误：local 后需要标识符");
            }

            std::string varName = static_cast<std::string>(current.value);
            Advance(); // 跳过变量名

            if (current.token == TokenType::Assign)
            {
                ParseAssignment(varName);
            }
            // TODO: 实现局部变量声明，暂时跳过
            std::cout << "局部变量声明: " << varName << std::endl;
        }

        void Advance()
        {
            std::cout << current.toString() << std::endl;
            current = next;
            next = lexer.NextToken();
        }

        void Consume(const std::string& expectedStr)
        {
            bool matched = false;
            if (expectedStr == "(" && current.token == TokenType::ParL) {
                matched = true;
            } else if (expectedStr == ")" && current.token == TokenType::ParR) {
                matched = true;
            } else if (expectedStr == "=" && current.token == TokenType::Assign) {
                matched = true;
            } else if (expectedStr == ";" && current.token == TokenType::SemiColon) {
                matched = true;
            } else if (static_cast<std::string>(current.value) == expectedStr) {
                matched = true;
            }

            if (matched)
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