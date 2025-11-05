#pragma once
#include <variant>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <array>

namespace Engine
{
    enum class TokenType
    {
        Eof,
        Identifier,
        String,
        Number,
    };

    using Value = std::variant<std::monostate, std::string, double, bool>;

    struct Token
    {
        TokenType token;
        Value value;
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
        std::vector<uint8_t> args;
        
        Operation() = delete;

        Operation(OpCode op_code)
            :opCode(op_code)
        { }

        template <typename Container, // 匹配容器
            std::enable_if_t<std::is_constructible_v<std::vector<uint16_t>, Container&&>, int> = 0>
        Operation(OpCode op_code, Container&& container)
            :op(op_code), args(std::forward<Container>(container))
        { }

        template<typename... Args,   // 匹配实参
            std::enable_if_t<std::conjunction_v<std::is_convertible<Args, uint16_t>...>, int> = 0>
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