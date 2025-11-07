#pragma once

#include "LuaState.h"
#include <fstream>

namespace Engine
{
    class Lex
    {
    public:
        Lex(std::ifstream& inputStream)
            : input(inputStream), current(input.get())
        {
            if (!input)
                throw std::runtime_error("无效的输入流");
        }

        Token NextToken()
        {
            while (current != EOF)
            {
                // 空白字符（空格、制表符、换行、回车）
                if (isspace(static_cast<unsigned char>(current)))
                {
                    SkipWhitespace();
                    continue;
                }

                // 标识符（字母/下划线开头）
                if (isalpha(static_cast<unsigned char>(current)) || current == '_')
                {
                    return ReadIdentifier();
                }

                // 字符串（双引号包裹）
                if (current == '"')
                {
                    return ReadString();
                }

                // 数字（正负号、数字、小数点）
                if (isdigit(static_cast<unsigned char>(current)) || current == '+' || current == '-')
                {
                    return ReadNumber();
                }
                // 小括号
                if (current == '(')
                {
                    current = input.get();
                    return { TokenType::ParL, "(" };
                }
                if (current == ')')
                {
                    current = input.get();
                    return { TokenType::ParR, ")" };
                }
                
                // 未知字符
                throw std::runtime_error("未知字符: " + std::string(1, static_cast<char>(current)));
            }

            return { TokenType::Eof };
        }

    private:
        std::ifstream& input;
        int current;

        void SkipWhitespace()
        {
            while (current != EOF && isspace(static_cast<unsigned char>(current)))
            {
                current = input.get();
            }
        }

        Token ReadIdentifier()
        {
            std::string id;
            // TODO:分析哪些字符可作为标识符
            while (current != EOF &&
                (isalnum(static_cast<unsigned char>(current)) || current == '_'))
            {
                id += static_cast<char>(current);
                current = input.get();
            }
            return { TokenType::Identifier, id };
        }

        Token ReadString()
        {
            std::string str;
            current = input.get();

            while (current != EOF && current != '"')
            {
                // 处理转义字符 \"
                if (current == '\\')
                {
                    current = input.get();
                    if (current == '"')
                    {
                        str += '"';
                    }
                    else
                    {
                        str += '\\'; // 保留未识别的转义符
                        str += static_cast<char>(current);
                    }
                }
                else
                {
                    str += static_cast<char>(current);
                }
                current = input.get();
            }

            if (current != '"')
            {
                throw std::runtime_error("未闭合的字符串");
            }

            current = input.get();
            return { TokenType::String, str };
        }

        Token ReadNumber()
        {
            std::string numStr;
            bool hasDecimal = false;

            if (current == '+' || current == '-')
            {
                numStr += static_cast<char>(current);
                current = input.get();
                if (current == EOF || !isdigit(static_cast<unsigned char>(current)))
                {
                    throw std::runtime_error("正负号后必须跟数字");
                }
            }

            while (current != EOF)
            {
                if (isdigit(static_cast<unsigned char>(current)))
                {
                    numStr += static_cast<char>(current);
                    current = input.get();
                }
                else if (current == '.' && !hasDecimal)
                {
                    hasDecimal = true;
                    numStr += static_cast<char>(current);
                    current = input.get();
                    if (current == EOF || !isdigit(static_cast<unsigned char>(current)))
                    {
                        throw std::runtime_error("小数点后必须跟数字");
                    }
                }
                else
                {
                    break;
                }
            }

            double value = std::stod(numStr);
            return { TokenType::Number, value };
        }
    };
}