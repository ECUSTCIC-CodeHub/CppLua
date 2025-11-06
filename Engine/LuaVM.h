#pragma once

#include "LuaState.h"
#include "LuaParser.h"

namespace Engine
{
    class VM
    {
    public:
        explicit VM(const std::string& lua)
            : fileStream(lua), parser(fileStream)
        {
            if (!fileStream.is_open())
                throw std::runtime_error("无法打开脚本文件：" + lua);
            context = parser.Parse();
            sp = 0;
        }

        void Excute()
        {
            try
            {
                for(size_t ip = 0; ip < context.Operations.size(); ++ip)
                {
                    const auto& op = context.Operations[ip];
                    switch (op.opCode)
                    {
                    case OpCode::Exit:
                        return;
                    case OpCode::LoadConst:
                    {
                        uint32_t constIdx = op.args[0];
                        context.stack[sp] = context.Constants[constIdx];
                        sp++;
                        break;
                    }
                    case OpCode::LoadGlobal:
                    {
                        uint32_t globalIdx = op.args[0];
                        auto key = static_cast<std::string>(context.Constants[globalIdx]);
                        auto fun = context.Globals[key];
                        context.stack[sp] = fun;
                        sp++;
                        break;
                    }
                    case OpCode::Call:
                    {
                        uint32_t funcIdx = op.args[0];
                        uint32_t argCount = op.args[1];
                        std::vector<Value> args;
                        for (uint32_t i = 0; i < argCount; ++i)
                        {
                            args.push_back(context.stack[i + 1]);
                        }
                        Value result = context.stack[0].Call(args);
                        context.stack[0] = result;
                        break;
                    }
                    default:
                        throw std::runtime_error("未知的操作码");
                    }
                }
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error("执行错误：" + std::string(e.what()));
            }
        }
    private:
        std::ifstream fileStream; // 必须在Parser之前声明，保证初始化顺序
        Parser parser;
        ProgramContext context;

        size_t sp;

    private:

    };
}