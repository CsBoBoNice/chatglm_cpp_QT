#ifndef CHAT_THREAD_H
#define CHAT_THREAD_H

#include <QThread>

#include "chatglm.h"

// 系统设定
#define CHAT_SYSTEM_SET ("You are ChatGLM3, a large language model trained by Zhipu.AI. Follow the user's instructions carefully. Respond using markdown.")
// #define CHAT_SYSTEM_SET ("你是Zhipu.AI培训的大型语言模型ChatGLM3, 请仔细遵循用户的指示. 使用markdown格式进行响应.")
// #define CHAT_SYSTEM_SET ("你是一位智能AI助手，你叫ChatGLM，你连接着一台电脑，但请注意不能联网。在使用Python解决任务时，你可以运行代码并得到结果，如果运行结果有错误，你需要尽可能对代码进行改进。")

// 预设地址
#define MODEL_PATH_BIN ("/storage/emulated/0/chatglm/ggml.bin")

class ChatThread : public QThread
{
    Q_OBJECT

public:
    // 系统设定
    std::string m_systemInfo = CHAT_SYSTEM_SET;

private:

    bool m_stop = false; //停止线程run()

    bool m_runing = false; // 正在生成聊天信息

    // 历史消息
    std::vector<chatglm::ChatMessage> m_messages;

    // 功能
    std::string m_role;

protected:
    void run(); //线程的事件循环
public:
    explicit ChatThread(QObject* parent = nullptr);

    void stopThread(); //结束线程run()

signals: // 信号
    void ChatOutput(QString str); //产生新消息

public :
    void ChatInput(QString str); //输入新消息
    bool CleanHistory(); //清空历史
    bool isRuning(); //检查是否正在运行
    void stopChatThread(); // 暂停线程

    bool setSystemInfo(QString info); // 设置预设信息

};

#endif // TDICETHREAD_H
