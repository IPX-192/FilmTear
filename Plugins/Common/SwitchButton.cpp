#include <QPainter>
#include "SwitchButton.h"
#include "ui_SwitchButton.h"

SwitchButton::SwitchButton(QString title, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SwitchButton),
    m_title(title)
{
    ui->setupUi(this);
    m_checked = false;

    //开、闭时背景颜色
    m_bgColorOff = QColor(100, 100, 100);
    m_bgColorOn = QColor(85, 170, 255);
    //开、闭时滑块颜色
    m_sliderColorOff = QColor(255, 255, 255);
    m_sliderColorOn = QColor(255, 255, 255);
    m_space = 6;      //背景间隔
    m_rectRadius = 5;
    m_step = width() / 10;  //步长
    //起始、目标位置
    m_startX = 0;
    m_endX = 0;
    m_timer = new QTimer(this);
    m_timer->setInterval(10);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateValue()));
    setFont(QFont("Microsoft Yahei", 12));
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags | Qt::FramelessWindowHint | Qt::Tool |
                   Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);   //隐藏标题栏，不在任务栏显示
}

SwitchButton::~SwitchButton()
{
    delete ui;
}

void SwitchButton::SetTitle(QString title)
{
    m_title = title;
}

void SwitchButton::ChangeState(bool flag)
{
    if (m_checked != flag)
    {
        QMouseEvent *event = nullptr;
        mousePressEvent(event);
    }
}

void SwitchButton::drawBg(QPainter *painter)
{
    painter->save();

    painter->setPen(Qt::NoPen);
    if (!m_checked)
    {
        painter->setBrush(m_bgColorOff);
    }
    else
    {
        painter->setBrush(m_bgColorOn);
    }
    painter->drawRoundedRect(rect(), m_rectRadius, m_rectRadius);

    painter->restore();
}

void SwitchButton::drawSlider(QPainter *painter)
{
    painter->save();

    painter->setPen(Qt::NoPen);
    if (!m_checked)
    {
        painter->setBrush(m_sliderColorOff);
    }
    else
    {
        painter->setBrush(m_sliderColorOn);
    }
    int sliderHeight = height() - m_space;
    int sliderWidth = sliderHeight;
    QRect sliderRect(m_startX + m_space / 2, m_space / 2, sliderWidth , sliderHeight);
    painter->drawRoundedRect(sliderRect, m_rectRadius, m_rectRadius);

    painter->restore();
}

void SwitchButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_checked = !m_checked;
    emit CheckedChanged(m_checked);

    //状态切换改变后自动计算终点坐标
    if (m_checked)
    {
        m_endX = width() - height();
    }
    else
    {
        m_endX = 0;
    }

    m_timer->start();
}


void SwitchButton::updateValue()
{
    if (m_checked)
    {
        if (m_startX < m_endX)
        {
            m_startX = m_startX + m_step;
        }
        else
        {
            m_startX = m_endX;
            m_timer->stop();
        }
    }
    else
    {
        if (m_startX > m_endX)
        {
            m_startX = m_startX - m_step;
        }
        else
        {
            m_startX = m_endX;
            m_timer->stop();
        }
    }

    update();
}

void SwitchButton::resizeEvent(QResizeEvent *event)
{
    //尺寸大小改变后自动设置起点坐标为终点
    if (m_checked)
    {
        m_startX = width() - height();
    }
    else
    {
        m_startX = 0;
    }

    update();
}

void SwitchButton::paintEvent(QPaintEvent *event)
{
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //绘制背景
    drawBg(&painter);
    //绘制滑块
    drawSlider(&painter);

    painter.setPen(QPen(QColor(255, 255, 255)));
    if (!m_checked)
    {
        painter.drawText(m_startX + height() + m_space, height() / 2 + m_space, m_title);
    }
    else
    {
        painter.drawText(height() / 3 + m_space, height() / 2 + m_space, m_title);
    }
}
