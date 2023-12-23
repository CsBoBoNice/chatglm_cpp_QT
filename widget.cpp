#include "widget.h"
#include "./ui_widget.h"

#include <QDebug>

#include <QMutex>
#include <QMutexLocker>

#include <QResource>

#include <QFile>

#include <iostream>

#define FontPointSize_DEF 12

/******************************************************************************************/

// 使用单例模式 将 std::cout 重定向到 QTextEdit 实现输出

// QTextEditOutputStream& outputStream = QTextEditOutputStream::instance(ui->textEdit_put);

// // 通过重定向 std::cout 到 QTextEdit 实现输出
// std::cout.rdbuf(outputStream.rdbuf());

// 在chatglm.cpp文件TextStreamer::put函数中被调用
class QTextEditStreamBuf : public std::streambuf
{
public:
    explicit QTextEditStreamBuf(QTextBrowser* textBrowser) : textBrowser(textBrowser) {}

protected:
    virtual int_type overflow(int_type c) override
    {
        QMutexLocker locker(&mutex); // 加锁

        if (c != traits_type::eof())
        {
            char ch = traits_type::to_char_type(c);
            textBrowser->moveCursor(QTextCursor::End); // 将光标移动到末尾

            textBrowser->insertPlainText(QString(ch));

            textBrowser->moveCursor(QTextCursor::End); // 将光标移动到末尾
        }

        return c;
    }

    virtual std::streamsize xsputn(const char* s, std::streamsize n) override
    {
        QMutexLocker locker(&mutex); // 加锁
        textBrowser->moveCursor(QTextCursor::End); // 将光标移动到末尾

        textBrowser->insertPlainText(QString::fromUtf8(s, n));

        textBrowser->moveCursor(QTextCursor::End); // 将光标移动到末尾

        return n;
    }

private:
    QTextBrowser* textBrowser;
    QMutex mutex; // Qt的互斥锁
};

class QTextBrowserOutputStream : public std::ostream
{
public:
    static QTextBrowserOutputStream& instance(QTextBrowser* textBrowser)
    {
        static QTextBrowserOutputStream instance(textBrowser);
        return instance;
    }

private:
    explicit QTextBrowserOutputStream(QTextBrowser* textBrowser)
        : std::ostream(&buffer), buffer(textBrowser) {}

    QTextEditStreamBuf buffer;
};
/******************************************************************************************/

Widget::Widget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->textBrowser_put->setReadOnly(true); // 设置只读

    qDebug("start...");

    QTextBrowserOutputStream& outputStream = QTextBrowserOutputStream::instance(ui->textBrowser_put);

    // 通过重定向 std::cout 到 QTextEdit 实现逐字输出
    std::cout.rdbuf(outputStream.rdbuf());

    // 连接发送按钮的按下信号
    connect(ui->pushButton_send, &QPushButton::clicked, this, &Widget::Send_prompt);

    // 绑定清空历史
    connect(ui->pushButton_clean, &QPushButton::clicked, this, &Widget::Clean_history);

    // 绑定设置预设信息
    connect(ui->pushButton_system, &QPushButton::clicked, this, &Widget::setSystemInfo);

    ui->textEdit_system->setPlainText(CHAT_SYSTEM_SET);

    // // 在这里添加你的输出语句 测试 重定向 std::cout 到 QTextEdit 实现输出
    // std::cout << "Hello, QTextEdit! This is a test." << std::endl;

    // 开个线程用于chatGLM运行
    m_ChatThread = new ChatThread(this);

    // 连接线程返回的信号
    connect(m_ChatThread, &ChatThread::ChatOutput, this, &Widget::ChatOutput_show);

    m_ChatThread->start(); // 启动线程
}

Widget::~Widget()
{
    delete ui;
}

void Widget::Send_prompt()
{
    QString send_str = ui->textEdit_send->toPlainText();

    ui->textEdit_send->clear(); // 清空输入

    m_ChatThread->ChatInput(send_str);

}

void Widget::Clean_history()
{
    bool ret = m_ChatThread->isRuning();

    // 未在运行则清空
    if(true != ret)
    {
        ui->textEdit_send->clear(); // 清空
        ui->textBrowser_put->clear(); // 清空
        m_ChatThread->CleanHistory();
    }
}

// 设置预设信息
void Widget::setSystemInfo()
{
    bool ret = m_ChatThread->isRuning();

    // 未在运行则清空
    if(true != ret)
    {
        ui->textEdit_send->clear(); // 清空
        ui->textBrowser_put->clear(); // 清空

        QString sysInfo = ui->textEdit_system->toPlainText();
        m_ChatThread->setSystemInfo(sysInfo);

        m_ChatThread->CleanHistory();
    }
}

void Widget::ChatOutput_show(QString str)
{
    qDebug("ChatOutput_show: %s", str.toStdString().c_str());

    // 从QTextBrowser获取所有文本
    QString allText = ui->textBrowser_put->toMarkdown();

    // 以Markdown格式显示文本
    ui->textBrowser_put->setMarkdown(allText);

    ui->textBrowser_put->moveCursor(QTextCursor::End); // 将光标移动到末尾
}
