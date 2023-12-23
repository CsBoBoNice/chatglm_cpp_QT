#include "chat_thread.h"
#include <QDebug>

#include <iomanip>
#include <iostream>

enum InferenceMode
{
    INFERENCE_MODE_CHAT,
    INFERENCE_MODE_GENERATE,
};

struct Args
{
    std::string model_path = "chatglm-ggml.bin";
    InferenceMode mode = INFERENCE_MODE_CHAT;
    bool sync = false;
    std::string prompt = "你好";
    std::string system = "";
    int max_length = 2048;
    int max_new_tokens = -1;
    int max_context_length = 512;
    bool interactive = false;
    int top_k = 0;
    float top_p = 0.7;
    float temp = 0.95;
    float repeat_penalty = 1.0;
    int num_threads = 0;
    bool verbose = false;
};

static inline void print_message(const chatglm::ChatMessage& message)
{
    std::cout << message.content << "\n";

    if (!message.tool_calls.empty() && message.tool_calls.front().type == chatglm::ToolCallMessage::TYPE_CODE)
    {
        std::cout << message.tool_calls.front().code.input << "\n";
    }
}

ChatThread::ChatThread(QObject* parent)
    : QThread{parent}
{

}

void ChatThread::stopThread()
{
    //停止线程
    m_stop = true;
}

void ChatThread::ChatInput(QString str) //输入新消息
{
    qDebug("human: %s", str.toStdString().c_str());

    std::string prompt = str.toStdString();

    // 文本为空 退出
    if (prompt.empty())
    {
        return;
    }

    // 上一次回答未结束 退出
    if(m_runing == true)
    {
        return;
    }

    // 将消息输入显示框
    std::cout << prompt << std::endl << std::flush;

    // // 检查大小是否超过 最小值为4 确保是2的倍数，因为一问一答
    // while(m_messages.size() >= 10) // 这段代码是为了保证只回答最新数据，但是又保留最早的一问一答，确保稳定
    // {
    // // for (size_t i = 0; i < m_history.size(); ++i)
    // // {
    // // qDebug("[%zu]: %s\n\n", i, m_history[i].c_str());
    // // }

    // // 删除最旧的数据
    // m_messages.erase(m_messages.begin() + 2); // 从第三条开始删除
    // m_messages.erase(m_messages.begin() + 2); // 从第三条开始删除

    // // qDebug("m_history.size() : %zu", m_history.size());

    // // for (size_t i = 0; i < m_history.size(); ++i)
    // // {
    // // qDebug("[%zu]: %s\n\n", i, m_history[i].c_str());
    // // }

    // }

    std::string role;
    role = m_role;

    // 未定义角色则为用户聊天
    if (role.empty())
    {
        role = chatglm::ChatMessage::ROLE_USER;
    }

    m_messages.emplace_back(std::move(role), std::move(prompt));

    m_runing = true; // 有新的消息开始聊天
}

bool ChatThread::CleanHistory() //清空历史
{
    // 上一次回答未结束 退出
    if(m_runing == true)
    {
        return false;
    }

    std::vector<chatglm::ChatMessage> system_messages;

    // 设定放入历史消息
    system_messages.emplace_back(chatglm::ChatMessage::ROLE_SYSTEM, m_systemInfo);

    // 历史消息
    m_messages = system_messages;

    // 打印系统设定
    std::cout << "System" << " > " << m_systemInfo << std::endl;

    std::cout << "\n" << "human >" << std::flush;

    return true;
}

//检查是否正在运行
bool ChatThread::isRuning()
{
    return m_runing;
}

// 停止线程
void ChatThread::stopChatThread()
{
    m_stop = true;
}

// 设置预设信息
bool ChatThread::setSystemInfo(QString info)
{

    if(info.isEmpty())
    {
        return false;
    }

    if(true == m_runing)
    {
        return false;
    }

    m_systemInfo = info.toStdString();

    std::vector<chatglm::ChatMessage> system_messages;

    return true;
}

void ChatThread::run()
{
    //线程的事件循环
    m_stop = false; //启动线程时令m_stop=false

    qDebug("ggml_time_init...");

    Args args;
    args.model_path = MODEL_PATH_BIN; // 模型路径 D:/ChatGLM3_6B/models/chatglm-ggml/chatglm3-6b-32k-ggml_q4_0.bin
    args.interactive = true; // -i, --interactive 以交互模式运行 run in interactive mode

    args.mode = INFERENCE_MODE_CHAT;// --mode 推理模式选择 {聊天， 生成}（默认： 聊天） inference mode chose from {chat, generate} (default: chat)

    args.max_length = 2048; //-l, --max_length 最大总长度，包括提示和输出  max total length including prompt and output (default: 2048)
    args.max_context_length = 512; // -c, --max_context_length 最大上下文长度 max context length (default: 512)\n"

    args.top_k = 0; // --top_k top-k sampling (default: 0) 在生成文本时，模型会考虑在每一步选择概率最高的前 k 个候选词语，然后从中随机选择一个作为下一个词语
    args.top_p = 0.7; // --top_p top-p sampling (default: 0.7) 控制生成的多样性，允许更多的不确定性
    args.temp = 0.95; // --temp temperature (default: 0.95) 高温度值会增加词语之间的差异，使得模型更加开放和创造性；低温度值则更加强调高概率的词语，使生成更加确定和保守
    args.repeat_penalty = 1.0; // --repeat_penalty 为了避免生成文本中过度重复的片段，可以通过引入一个重复惩罚 来减少模型生成相似内容的概率 penalize repeat sequence of tokens (default: 1.0, 1.0 = disabled)

    args.num_threads = 0;// -t, --threads 用于推理的线程数 number of threads for inference

    args.verbose = false; // -v, --verbose 显示详细输出，包括配置/系统/性能信息 display verbose output including config/system/performance info

    args.prompt = "hi"; // p, --prompt PROMPT  开始生成的提示   prompt to start generation with (default: 你好)

    args.sync = false; // --sync 无流的同步生成 synchronized generation without streaming

    // -s, --system SYSTEM 系统消息，用于设置助手的行为 system message to set the behavior of the assistant
    args.system = m_systemInfo;

    args.max_new_tokens = -1; // ---max_new_tokens 要生成的最大令牌数，忽略提示令牌数 max number of tokens to generate, ignoring the number of prompt tokens

    ggml_time_init();
    int64_t start_load_us = ggml_time_us();
    chatglm::Pipeline pipeline(args.model_path);

    int64_t end_load_us = ggml_time_us();

    qDebug("ggml_time_init OK!");

    std::string model_name = pipeline.model->config.model_type_name();

    auto text_streamer = std::make_shared<chatglm::TextStreamer>(std::cout, pipeline.tokenizer.get());
    auto perf_streamer = std::make_shared<chatglm::PerfStreamer>();
    std::vector<std::shared_ptr<chatglm::BaseStreamer>> streamers{perf_streamer};

    if (!args.sync)
    {
        streamers.emplace_back(text_streamer);
    }

    auto streamer = std::make_unique<chatglm::StreamerGroup>(std::move(streamers));

    chatglm::GenerationConfig gen_config(args.max_length, args.max_new_tokens, args.max_context_length, args.temp > 0,
                                         args.top_k, args.top_p, args.temp, args.repeat_penalty, args.num_threads);

    if (args.verbose)
    {
        std::cout << "system info: | "
                  << "AVX = " << ggml_cpu_has_avx() << " | "
                  << "AVX2 = " << ggml_cpu_has_avx2() << " | "
                  << "AVX512 = " << ggml_cpu_has_avx512() << " | "
                  << "AVX512_VBMI = " << ggml_cpu_has_avx512_vbmi() << " | "
                  << "AVX512_VNNI = " << ggml_cpu_has_avx512_vnni() << " | "
                  << "FMA = " << ggml_cpu_has_fma() << " | "
                  << "NEON = " << ggml_cpu_has_neon() << " | "
                  << "ARM_FMA = " << ggml_cpu_has_arm_fma() << " | "
                  << "F16C = " << ggml_cpu_has_f16c() << " | "
                  << "FP16_VA = " << ggml_cpu_has_fp16_va() << " | "
                  << "WASM_SIMD = " << ggml_cpu_has_wasm_simd() << " | "
                  << "BLAS = " << ggml_cpu_has_blas() << " | "
                  << "SSE3 = " << ggml_cpu_has_sse3() << " | "
                  << "VSX = " << ggml_cpu_has_vsx() << " |\n";

        std::cout << "inference config: | "
                  << "max_length = " << args.max_length << " | "
                  << "max_context_length = " << args.max_context_length << " | "
                  << "top_k = " << args.top_k << " | "
                  << "top_p = " << args.top_p << " | "
                  << "temperature = " << args.temp << " | "
                  << "repetition_penalty = " << args.repeat_penalty << " | "
                  << "num_threads = " << args.num_threads << " |\n";

        std::cout << "loaded " << pipeline.model->config.model_type_name() << " model from " << args.model_path
                  << " within: " << (end_load_us - start_load_us) / 1000.f << " ms\n";

        std::cout << std::endl;
    }

    if (args.mode != INFERENCE_MODE_CHAT && args.interactive)
    {
        std::cerr << "interactive demo is only supported for chat mode, falling back to non-interactive one\n";
        args.interactive = false;
    }

    std::vector<chatglm::ChatMessage> system_messages;

    // 判断设定是否为空 非空将设定放入历史消息
    if (!args.system.empty())
    {
        system_messages.emplace_back(chatglm::ChatMessage::ROLE_SYSTEM, args.system);
    }

    // 历史消息
    m_messages = system_messages;

    // 打印系统设定
    if (!args.system.empty())
    {
        std::cout << std::setw(model_name.size()) << std::left << "System"
                  << " > " << args.system << std::endl;
    }

    std::cout << "\n" << "human >" << std::flush;

    m_role = chatglm::ChatMessage::ROLE_USER;

    while(!m_stop)
    {
        //循环主体
        if (true == m_runing)
        {
            msleep(100); //线程休眠100ms // 好像争抢资源会崩溃

            std::cout << "\n" << model_name << " > ";
            chatglm::ChatMessage output = pipeline.chat(m_messages, gen_config, streamer.get());

            if (args.sync)
            {
                print_message(output);
            }

            /********************************************************************/
            QString output_role = QString::asprintf("%s", output.role.c_str());
            QString contrast_role = QString::asprintf("%s", chatglm::ChatMessage::ROLE_ASSISTANT.c_str());

            qDebug("output_role: %s", output_role.toStdString().c_str());
            qDebug("contrast_role: %s", contrast_role.toStdString().c_str());

            if(output_role == contrast_role)
            {
                QString output_str = QString::asprintf("%s", output.content.c_str());

                std::cout << "\n\n" << "human >" << std::flush;

                qDebug("robot: %s", output_str.toStdString().c_str());

                emit ChatOutput(output_str); //发射信号
            }

            /********************************************************************/

            m_messages.emplace_back(std::move(output)); // 放入历史

            std::string role;

            // 判断是否是工具调用
            if (!m_messages.empty() && !m_messages.back().tool_calls.empty())
            {
                const auto& tool_call = m_messages.back().tool_calls.front();

                if (tool_call.type == chatglm::ToolCallMessage::TYPE_FUNCTION) // 工具调用
                {
                    // function call
                    std::cout << "Function Call > Please manually call function `" << tool_call.function.name
                              << "` with args `" << tool_call.function.arguments << "` and provide the results below.\n"
                              << "Observation   > " << std::flush;
                }
                else if (tool_call.type == chatglm::ToolCallMessage::TYPE_CODE) // 代码执行
                {
                    // code interpreter
                    std::cout << "Code Interpreter > Please manually run the code and provide the results below.\n"
                              << "Observation      > " << std::flush;
                }
                else
                {
                    CHATGLM_THROW << "unexpected tool type " << tool_call.type;
                }

                role = chatglm::ChatMessage::ROLE_OBSERVATION;
            }
            else // 用户聊天
            {
                role = chatglm::ChatMessage::ROLE_USER;
            }

            m_role = role;

            // 显示详细输出
            if (args.verbose)
            {
                std::cout << "\n" << perf_streamer->to_string() << "\n\n";
            }

            perf_streamer->reset(); // 重置性能统计

            m_runing = false; // 回答结束
        }

        msleep(500); //线程休眠500ms

    }

    std::cout << "Bye\n";

    // 在  m_stop==true时结束线程任务
    quit(); //相当于exit(0), 退出线程的事件循环
}
