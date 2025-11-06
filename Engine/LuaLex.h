#pragma once

#include "LuaState.h"
#include <fstream>

namespace Engine
{
    class Lex
    {
    public:
        Lex(std::ifstream& inputStream)
            : input(inputStream), current(input.get()) // 初始化时读取第一个字符
        {
            if (!input)
                throw std::runtime_error("无效的输入流");
        }

        Token NextToken()
        {
            while (current != EOF)
            {
                // 跳过空白字符（空格、制表符、换行、回车）
                if (isspace(static_cast<unsigned char>(current)))
                {
                    SkipWhitespace();
                    continue;
                }

                // 识别标识符（字母/下划线开头）
                if (isalpha(static_cast<unsigned char>(current)) || current == '_')
                {
                    return ReadIdentifier();
                }

                // 识别字符串（双引号包裹）
                if (current == '"')
                {
                    return ReadString();
                }

                // 识别数字（正负号、数字、小数点）
                if (isdigit(static_cast<unsigned char>(current)) || current == '+' || current == '-')
                {
                    return ReadNumber();
                }

                if (current == '(')
                {
                    current = input.get(); // 读取下一个字符
                    return { TokenType::ParL, "(" }; // 返回 ParL Token
                }

                // 识别右括号 ')'
                if (current == ')')
                {
                    current = input.get(); // 读取下一个字符
                    return { TokenType::ParR, ")" }; // 返回 ParR Token
                }
                
                // 未知字符（抛出异常或返回错误Token，这里选择抛出）
                throw std::runtime_error("未知字符: " + std::string(1, static_cast<char>(current)));
            }

            // 到达文件末尾，返回EOF Token（值为monostate，无实际内容）
            return { TokenType::Eof };
        }

    private:
        std::ifstream& input;  // 输入流（引用，外部管理生命周期）
        int current;       // 当前读取的字符（用int接收get()，可判断EOF）

        // 跳过所有空白字符（空格、制表符、换行、回车）
        void SkipWhitespace()
        {
            while (current != EOF && isspace(static_cast<unsigned char>(current)))
            {
                current = input.get();
            }
        }

        // 读取标识符（字母/下划线开头，后跟字母/数字/下划线）
        Token ReadIdentifier()
        {
            std::string id;
            // 收集所有标识符字符
            while (current != EOF &&
                (isalnum(static_cast<unsigned char>(current)) || current == '_'))
            {
                id += static_cast<char>(current);
                current = input.get();
            }
            // 标识符的值存储为std::string
            return { TokenType::Identifier, id };
        }

        // 读取字符串（双引号包裹，支持转义字符 \"）
        Token ReadString()
        {
            std::string str;
            current = input.get(); // 跳过开头的双引号

            while (current != EOF && current != '"')
            {
                // 处理转义字符 \"
                if (current == '\\')
                {
                    current = input.get(); // 读取转义符后的字符
                    if (current == '"')
                    {
                        str += '"'; // 转义为普通双引号
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

            // 检查是否正常结束（找到闭合双引号）
            if (current != '"')
            {
                throw std::runtime_error("未闭合的字符串");
            }

            current = input.get(); // 跳过结尾的双引号
            // 字符串的值存储为std::string
            return { TokenType::String, str };
        }

        // 读取数字（支持整数、小数、正负号）
        Token ReadNumber()
        {
            std::string numStr;
            bool hasDecimal = false;

            // 处理正负号（仅允许开头出现一次）
            if (current == '+' || current == '-')
            {
                numStr += static_cast<char>(current);
                current = input.get();
                // 正负号后必须跟数字，否则是无效语法
                if (current == EOF || !isdigit(static_cast<unsigned char>(current)))
                {
                    throw std::runtime_error("正负号后必须跟数字");
                }
            }

            // 收集数字字符（整数部分 + 小数点 + 小数部分）
            while (current != EOF)
            {
                if (isdigit(static_cast<unsigned char>(current)))
                {
                    numStr += static_cast<char>(current);
                    current = input.get();
                }
                else if (current == '.' && !hasDecimal)
                {
                    // 仅允许一个小数点
                    hasDecimal = true;
                    numStr += static_cast<char>(current);
                    current = input.get();
                    // 小数点后必须跟数字，否则是无效语法（如 "123." 不合法）
                    if (current == EOF || !isdigit(static_cast<unsigned char>(current)))
                    {
                        throw std::runtime_error("小数点后必须跟数字");
                    }
                }
                else
                {
                    // 遇到非数字字符，停止读取
                    break;
                }
            }

            // 将字符串转换为double（支持整数和小数）
            double value = std::stod(numStr);
            return { TokenType::Number, value };
        }
    };
}