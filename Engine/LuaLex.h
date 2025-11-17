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

                // 数字（数字、小数点）
                if (isdigit(static_cast<unsigned char>(current)) ||
                    (current == '.' && isdigit(static_cast<unsigned char>(PeekNext()))))
                {
                    return ReadNumber();
                }

                // 算术运算符
                if (current == '+')
                {
                    current = input.get();
                    return { TokenType::Add, "+" };
                }
                if (current == '-')
                {
                    current = input.get();
                    return { TokenType::Sub, "-" };
                }
                if (current == '*')
                {
                    current = input.get();
                    return { TokenType::Mul, "*" };
                }
                if (current == '/')
                {
                    current = input.get();
                    // 检查是否为 //
                    if (current == '/')
                    {
                        current = input.get();
                        return { TokenType::Idiv, "//" };
                    }
                    return { TokenType::Div, "/" };
                }
                if (current == '%')
                {
                    current = input.get();
                    return { TokenType::Mod, "%" };
                }
                if (current == '^')
                {
                    current = input.get();
                    return { TokenType::Pow, "^" };
                }
                if (current == '#')
                {
                    current = input.get();
                    return { TokenType::Len, "#" };
                }

                // 位运算符
                if (current == '&')
                {
                    current = input.get();
                    return { TokenType::BitAnd, "&" };
                }
                if (current == '~')
                {
                    current = input.get();
                    if (current == '=')
                    {
                        current = input.get();
                        return { TokenType::NotEq, "~=" };
                    }
                    return { TokenType::BitXor, "~" };
                }
                if (current == '|')
                {
                    current = input.get();
                    return { TokenType::BitOr, "|" };
                }

                // 比较运算符
                if (current == '=')
                {
                    current = input.get();
                    if (current == '=')
                    {
                        current = input.get();
                        return { TokenType::Equal, "==" };
                    }
                    return { TokenType::Assign, "=" };
                }
                if (current == '<')
                {
                    current = input.get();
                    if (current == '=')
                    {
                        current = input.get();
                        return { TokenType::LesEq, "<=" };
                    }
                    if (current == '<')
                    {
                        current = input.get();
                        return { TokenType::ShiftL, "<<" };
                    }
                    return { TokenType::Less, "<" };
                }
                if (current == '>')
                {
                    current = input.get();
                    if (current == '=')
                    {
                        current = input.get();
                        return { TokenType::GreEq, ">=" };
                    }
                    if (current == '>')
                    {
                        current = input.get();
                        return { TokenType::ShiftR, ">>" };
                    }
                    return { TokenType::Greater, ">" };
                }

                // 分隔符和括号
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
                if (current == '{')
                {
                    current = input.get();
                    return { TokenType::CurlyL, "{" };
                }
                if (current == '}')
                {
                    current = input.get();
                    return { TokenType::CurlyR, "}" };
                }
                if (current == '[')
                {
                    current = input.get();
                    return { TokenType::SqurL, "[" };
                }
                if (current == ']')
                {
                    current = input.get();
                    return { TokenType::SqurR, "]" };
                }
                if (current == ';')
                {
                    current = input.get();
                    return { TokenType::SemiColon, ";" };
                }
                if (current == ':')
                {
                    current = input.get();
                    if (current == ':')
                    {
                        current = input.get();
                        return { TokenType::DoubColon, "::" };
                    }
                    return { TokenType::Colon, ":" };
                }
                if (current == ',')
                {
                    current = input.get();
                    return { TokenType::Comma, "," };
                }
                if (current == '.')
                {
                    current = input.get();
                    if (current == '.')
                    {
                        current = input.get();
                        if (current == '.')
                        {
                            current = input.get();
                            return { TokenType::Dots, "..." };
                        }
                        return { TokenType::Concat, ".." };
                    }
                    return { TokenType::Dot, "." };
                }

                // 未知字符
                throw std::runtime_error("未知字符: " + std::string(1, static_cast<char>(current)));
            }

            return { TokenType::Eof };
        }

    private:
        std::ifstream& input;
        int current;

        // 查看下一个字符但不移动指针
        int PeekNext()
        {
            if (input.eof())
                return EOF;
            return input.peek();
        }

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
            while (current != EOF &&
                (isalnum(static_cast<unsigned char>(current)) || current == '_'))
            {
                id += static_cast<char>(current);
                current = input.get();
            }

            // 检查是否为关键字
            if (id == "and") return { TokenType::And, id };
            if (id == "break") return { TokenType::Break, id };
            if (id == "do") return { TokenType::Do, id };
            if (id == "else") return { TokenType::Else, id };
            if (id == "elseif") return { TokenType::Elseif, id };
            if (id == "end") return { TokenType::End, id };
            if (id == "false") return { TokenType::False, false };
            if (id == "for") return { TokenType::For, id };
            if (id == "function") return { TokenType::Function, id };
            if (id == "goto") return { TokenType::Goto, id };
            if (id == "if") return { TokenType::If, id };
            if (id == "in") return { TokenType::In, id };
            if (id == "local") return { TokenType::Local, id };
            if (id == "nil") return { TokenType::Nil, std::monostate{} };
            if (id == "not") return { TokenType::Not, id };
            if (id == "or") return { TokenType::Or, id };
            if (id == "repeat") return { TokenType::Repeat, id };
            if (id == "return") return { TokenType::Return, id };
            if (id == "then") return { TokenType::Then, id };
            if (id == "true") return { TokenType::True, true };
            if (id == "until") return { TokenType::Until, id };
            if (id == "while") return { TokenType::While, id };

            return { TokenType::Identifier, id };
        }

        Token ReadString()
        {
            std::string str;
            current = input.get();

            while (current != EOF && current != '"')
            {
                // 处理转义字符
                if (current == '\\')
                {
                    current = input.get();
                    switch (current)
                    {
                    case 'a':  str += '\a'; break;
                    case 'b':  str += '\b'; break;
                    case 'f':  str += '\f'; break;
                    case 'n':  str += '\n'; break;
                    case 'r':  str += '\r'; break;
                    case 't':  str += '\t'; break;
                    case 'v':  str += '\v'; break;
                    case '\\': str += '\\'; break;
                    case '\"': str += '\"'; break;
                    case '\'': str += '\''; break;
                    case '\n': break; // 忽略换行符继续字符串
                    case EOF:
                        throw std::runtime_error("字符串转义序列不完整");
                    default:
                        // 未知的转义字符，保留原样
                        str += '\\';
                        str += static_cast<char>(current);
                        break;
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

            // 读取数字部分
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
                }
                else
                {
                    break;
                }
            }

            // 检查科学计数法
            if (current == 'e' || current == 'E')
            {
                numStr += static_cast<char>(current);
                current = input.get();

                if (current == '+' || current == '-')
                {
                    numStr += static_cast<char>(current);
                    current = input.get();
                }

                bool hasExpDigit = false;
                while (current != EOF && isdigit(static_cast<unsigned char>(current)))
                {
                    numStr += static_cast<char>(current);
                    current = input.get();
                    hasExpDigit = true;
                }

                if (!hasExpDigit)
                {
                    throw std::runtime_error("科学计数法需要指数部分");
                }
            }

            if (numStr.empty() || numStr == "." || numStr == "e" || numStr == "E")
            {
                throw std::runtime_error("无效的数字格式");
            }

            double value = std::stod(numStr);
            return { TokenType::Number, value };
        }
    };
}