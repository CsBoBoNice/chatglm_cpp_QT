#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "chat_thread.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget* parent = nullptr);
    ~Widget();

    chatglm::Pipeline* m_pipeline_p;

    std::string m_model_name;

    std::vector<std::string> m_history;

    std::shared_ptr<chatglm::TextStreamer> m_text_streamer;
    std::shared_ptr<chatglm::PerfStreamer> m_perf_streamer;
    std::shared_ptr<chatglm::StreamerGroup> m_streamer;

    chatglm::GenerationConfig m_gen_config;

private:
    Ui::Widget* ui;

    ChatThread* m_ChatThread; //工作线程

private slots:
    // 自定义槽函数
    void Send_prompt();
    void Clean_history();

    void setSystemInfo(); // 设置预设信息

public slots: //槽
    void ChatOutput_show(QString str); //输入新消息

};
#endif // WIDGET_H
