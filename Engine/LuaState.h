#pragma once
#include <variant>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <array>
#include <string>
#include <sstream> 
#include <functional>
#include <cmath>

namespace Engine
{
    enum class TokenType
    {
        Eof,
        Identifier,
        String,
        Number,
    };

    struct Value
    {
        using number = double;
        using function = std::function<Value(const std::vector<Value>&)>;
        using type = std::variant<
            std::monostate,
            bool,
            number,
            std::string,
            function
            >;

        Value() :value(std::monostate{}) {}

        Value(bool v) :value(v) {}

        Value(int v) :value(static_cast<number>(v)) {}
        Value(float v) :value(static_cast<number>(v)) {}
        Value(double v) :value(static_cast<number>(v)) {}

        Value(const char* v) : value(std::string(v)) {}
        Value(std::string v) : value(std::move(v)) {}
        
        Value(function v) : value(std::move(v)) {}

    private:
        type value;
    };

    struct Token
    {
        TokenType token;
        Value value;

        std::string toString()
        {
            std::string tk("type: ");
            switch (this->token)
            {
            case TokenType::Eof:
                tk += "Eof";
                break;
            case TokenType::Identifier:
                tk += "Identifier";
                break;
            case TokenType::String:
                tk += "String";
                break;
            case TokenType::Number:
                tk += "Number";
                break;
            default:
                tk += "Unknown";
            }

            tk += ", value: ";

           
            return tk;
        }
    };

    enum class OpCode
    {
        Exit,
        Call,
        LoadConst,
        LoadGlobal,
        SetGlobal,
    };

    struct Operation
    {
        OpCode opCode;
        std::vector<uint32_t> args;
        
        Operation() = delete;

        Operation(OpCode op_code)
            :opCode(op_code)
        { }

        template <typename Container, // 匹配容器
            std::enable_if_t<std::is_constructible_v<std::vector<uint32_t>, Container&&>, int> = 0>
        Operation(OpCode op_code, Container&& container)
            :opCode(op_code), args(std::forward<Container>(container))
        { }

        template<typename... Args,   // 匹配实参
            std::enable_if_t<std::conjunction_v<std::is_convertible<Args, uint32_t>...>, int> = 0>
        Operation(OpCode code, Args... arguments)
            : opCode(code), args({ static_cast<uint8_t>(arguments)... })
        { }
    };

    struct ProgramContext
    {
        std::vector<Value> Constants;                   // 常量表
        std::unordered_map<std::string, Value> Globals; // 全局变量表
        std::vector<Operation> Operations;              // 字节流
        std::array<Value, 64> stack;                    // 操作栈，栈大小待定，暂64
    };
}