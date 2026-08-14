#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class SwitchButton;
}

class SwitchButton : public QWidget
{
    Q_OBJECT

public:
    explicit SwitchButton(QString title = "", QWidget *parent = nullptr);
    ~SwitchButton();

    void SetTitle(QString title);
    void ChangeState(bool flag);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    //绘制背景框
    void drawBg(QPainter *painter);
    //绘制滑块
    void drawSlider(QPainter *painter);

private slots:
    void updateValue();

private:
    Ui::SwitchButton *ui;
    bool m_checked = false;

    //开、闭时背景颜色
    QColor m_bgColorOff;
    QColor m_bgColorOn;

    //开、闭时滑块颜色
    QColor m_sliderColorOff;
    QColor m_sliderColorOn;

    int m_space;  //背景间隔
    int m_rectRadius;

    int m_step;  //步长

    //起始、目标位置
    int m_startX = 0;
    int m_endX = 0;

    QTimer *m_timer = nullptr;

    QString m_title = "";

signals:
   void CheckedChanged(bool);
};

#endif // SWITCHBUTTON_H
