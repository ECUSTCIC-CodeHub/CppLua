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
#include <algorithm>

namespace Engine
{
    enum class TokenType
    {
        Eof,
        Identifier,

        //常量
        String,
        Number,

        // 关键词
        And, Break, Do, Else, Elseif, End,
        False, For, Function, Goto, If, In,
        Local, Nil, Not, Or, Repeat, Return,
        Then, True, Until, While,

        // 符号
        // +       -       *       /       %       ^       #
        Add, Sub, Mul, Div, Mod, Pow, Len,
        // &       ~       |       <<      >>      //
        BitAnd, BitXor, BitOr, ShiftL, ShiftR, Idiv,
        // ==       ~=     <=      >=      <       >        =
        Equal, NotEq, LesEq, GreEq, Less, Greater, Assign,
        // (       )       {       }       [       ]       ::
        ParL, ParR, CurlyL, CurlyR, SqurL, SqurR, DoubColon,
        // ;               :       ,       .       ..      ...
        SemiColon, Colon, Comma, Dot, Concat, Dots,
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

        template<typename t>
            requires std::is_convertible_v<t, number>
        Value(t v) : value(static_cast<number>(v)) {}

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

            result += "TokenType: ";
            switch (token)
            {
                // 基础类型
            case TokenType::Eof:        result += "Eof"; break;
            case TokenType::Identifier: result += "Identifier"; break;
            case TokenType::String:     result += "String"; break;
            case TokenType::Number:     result += "Number"; break;

                // 关键词
            case TokenType::And:        result += "And"; break;
            case TokenType::Break:      result += "Break"; break;
            case TokenType::Do:         result += "Do"; break;
            case TokenType::Else:       result += "Else"; break;
            case TokenType::Elseif:     result += "Elseif"; break;
            case TokenType::End:        result += "End"; break;
            case TokenType::False:      result += "False"; break;
            case TokenType::For:        result += "For"; break;
            case TokenType::Function:   result += "Function"; break;
            case TokenType::Goto:       result += "Goto"; break;
            case TokenType::If:         result += "If"; break;
            case TokenType::In:         result += "In"; break;
            case TokenType::Local:      result += "Local"; break;
            case TokenType::Nil:        result += "Nil"; break;
            case TokenType::Not:        result += "Not"; break;
            case TokenType::Or:         result += "Or"; break;
            case TokenType::Repeat:     result += "Repeat"; break;
            case TokenType::Return:     result += "Return"; break;
            case TokenType::Then:       result += "Then"; break;
            case TokenType::True:       result += "True"; break;
            case TokenType::Until:      result += "Until"; break;
            case TokenType::While:      result += "While"; break;

                // 算术运算符
            case TokenType::Add:        result += "Add(+)"; break;
            case TokenType::Sub:        result += "Sub(-)"; break;
            case TokenType::Mul:        result += "Mul(*)"; break;
            case TokenType::Div:        result += "Div(/)"; break;
            case TokenType::Mod:        result += "Mod(%)"; break;
            case TokenType::Pow:        result += "Pow(^)"; break;
            case TokenType::Len:        result += "Len(#)"; break;

                // 位运算与移位
            case TokenType::BitAnd:     result += "BitAnd(&)"; break;
            case TokenType::BitXor:     result += "BitXor(~)"; break;
            case TokenType::BitOr:      result += "BitOr(|)"; break;
            case TokenType::ShiftL:     result += "ShiftL(<<)"; break;
            case TokenType::ShiftR:     result += "ShiftR(>>)"; break;
            case TokenType::Idiv:       result += "Idiv(//)"; break;

                // 比较运算符与赋值
            case TokenType::Equal:      result += "Equal(==)"; break;
            case TokenType::NotEq:      result += "NotEq(~=)"; break;
            case TokenType::LesEq:      result += "LesEq(<=)"; break;
            case TokenType::GreEq:      result += "GreEq(>=)"; break;
            case TokenType::Less:       result += "Less(<)"; break;
            case TokenType::Greater:    result += "Greater(>)"; break;
            case TokenType::Assign:     result += "Assign(=)"; break;

                // 括号与作用域
            case TokenType::ParL:       result += "ParL(左括号)"; break;
            case TokenType::ParR:       result += "ParR(右括号)"; break;
            case TokenType::CurlyL:     result += "CurlyL(左大括号{)"; break;
            case TokenType::CurlyR:     result += "CurlyR(右大括号})"; break;
            case TokenType::SqurL:      result += "SqurL(左方括号[)"; break;
            case TokenType::SqurR:      result += "SqurR(右方括号])"; break;
            case TokenType::DoubColon:  result += "DoubColon(::)"; break;

                // 分隔符与连接符
            case TokenType::SemiColon:  result += "SemiColon(;)"; break;
            case TokenType::Colon:      result += "Colon(:)"; break;
            case TokenType::Comma:      result += "Comma(,)"; break;
            case TokenType::Dot:        result += "Dot(.)"; break;
            case TokenType::Concat:     result += "Concat(..)"; break;
            case TokenType::Dots:       result += "Dots (...)"; break;

            default:                    result += "Unknown"; break;
            }

            result += " | Value: [Type: ";

            switch (value.value.index())
            {
            case 0: // std::monostate
                result += "None, Value: <uninitialized>";
                break;

            case 1: // bool
                result += "Boolean, Value: ";
                result += std::get<bool>(value.value) ? "true" : "false";
                break;

            case 2: // number
                result += "Number, Value: ";
                {
                    double num = std::get<Value::number>(value.value);
                    if (num == static_cast<long long>(num))
                    {
                        result += std::to_string(static_cast<long long>(num));
                    }
                    else
                    {
                        result += std::to_string(num);
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