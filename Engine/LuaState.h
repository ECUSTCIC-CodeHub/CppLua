#pragma once
#include <variant>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <array>
#include <sstream> 
#include <functional>
#include <cmath>
#include <concepts>
#include <type_traits>
#include <ranges>

namespace Engine
{
    enum class TokenType
    {
        Eof,
        Identifier,
        String,
        Number,
        ParL,
        ParR,
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

        Value()                 : value(std::monostate{}) {}
        Value(std::monostate)   : value(std::monostate{}) {}

        Value(bool v) : value(v) {}

        Value(int v)    : value(static_cast<number>(v)) {}
        Value(float v)  : value(static_cast<number>(v)) {}
        Value(double v) : value(static_cast<number>(v)) {}

        Value(const char* v) : value(std::string(v)) {}
        Value(std::string v) : value(std::move(v)) {}
        
        Value(function v) : value(std::move(v)) {}

        operator type() const { return value; }
        operator type && () { return std::move(value); }

        friend bool operator==(const Value& lhs, const Value& rhs)
        {
            if (lhs.value.index() != rhs.value.index())
                return false;

            switch (lhs.value.index())
            {
            case 0: // std::monostate
                return true;

            case 1: // bool
                return std::get<bool>(lhs.value) == std::get<bool>(rhs.value);

            case 2: // number (double)
                return std::get<number>(lhs.value) == std::get<number>(rhs.value);

            case 3: // std::string
                return std::get<std::string>(lhs.value) == std::get<std::string>(rhs.value);

            case 4: // function (std::function)
                return false;

            default:
                return false;
            }
        }

        friend bool operator!=(const Value& lhs, const Value& rhs)
        {
            return !(lhs == rhs);
        }

        explicit operator bool() const
        {
            if (std::holds_alternative<bool>(value))
            {
               return std::get<bool>(value);
            }
            throw std::bad_variant_access();
        }
        explicit operator number() const
        {
            if (std::holds_alternative<number>(value))
            {
                return std::get<number>(value);
            }
            throw std::bad_variant_access();
        }
        explicit operator std::string() const 
        {
            if (std::holds_alternative<std::string>(value)) 
            {
                return std::get<std::string>(value);
            }
            if (std::holds_alternative<number>(value)) 
            {
                return std::to_string(std::get<number>(value));
            }
            throw std::bad_variant_access();
        }

        Value Call(const std::vector<Value>& args = {})
        {
            if(!std::holds_alternative<function>(value))
            {
                throw std::runtime_error("尝试调用非函数类型的值");
            }
            const auto& func = std::get<function>(value);

            if(!func)
            {
                throw std::runtime_error("尝试调用空函数");
            }
            return func(args);
        }

        Value Call()
        {
            return Call({});
        }

    public:
        type value;
    };

    struct Token
    {
        TokenType token;
        Value value;

        std::string toString() const
        {
            std::string result;

            // 1. 输出 Token 类型名称
            result += "TokenType: ";
            switch (token)
            {
            case TokenType::Eof:        result += "Eof"; break;
            case TokenType::Identifier: result += "Identifier"; break;
            case TokenType::String:     result += "String"; break;
            case TokenType::Number:     result += "Number"; break;
            case TokenType::ParL:       result += "ParL(左括号)"; break;
            case TokenType::ParR:       result += "ParR(右括号)"; break;
            default:                    result += "Unknown"; break;
            }

            // 2. 输出 Value 信息（类型 + 值）
            result += " | Value: [Type: ";

            // 根据 variant 的 index 判断 Value 实际类型
            switch (value.value.index())
            {
            case 0: // std::monostate
                result += "None, Value: <uninitialized>";
                break;

            case 1: // bool
                result += "Boolean, Value: ";
                result += std::get<bool>(value.value) ? "true" : "false";
                break;

            case 2: // number (double)
                result += "Number, Value: ";
                {
                    double num = std::get<Value::number>(value.value);
                    // 处理整数情况（避免输出 .000000）
                    if (num == static_cast<long long>(num))
                    {
                        result += std::to_string(static_cast<long long>(num));
                    }
                    else
                    {
                        result += std::to_string(num);
                        // 移除末尾多余的 0 和小数点（可选优化）
                        result.erase(result.find_last_not_of('0') + 1, std::string::npos);
                        if (result.back() == '.')
                        {
                            result.pop_back();
                        }
                    }
                }
                break;

            case 3: // std::string
                result += "String, Value: \"";
                result += std::get<std::string>(value.value);
                result += "\"";
                break;

            case 4: // function
                result += "Function, Value: <callable>";
                // 可选：判断函数是否为空
                if (!std::get<Value::function>(value.value))
                {
                    result += " (empty)";
                }
                break;

            default:
                result += "Unknown, Value: <invalid>";
                break;
            }

            result += "]";
            return result;
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
            std::enable_if_t<std::is_constructible_v<std::vector<uint32_t>, Container&&>, int> = 0,
            std::enable_if_t<!std::convertible_to<std::remove_cvref_t<Container>, uint32_t>, int> = 0>
        Operation(OpCode op_code, Container&& container)
            :opCode(op_code), args(std::forward<Container>(container))
        { }

        template<typename... Args,   // 匹配实参
            std::enable_if_t<std::conjunction_v<std::is_convertible<Args, uint32_t>...>, int> = 0>
        Operation(OpCode code, Args... arguments)
            : opCode(code), args({ static_cast<uint32_t>(arguments)... })
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