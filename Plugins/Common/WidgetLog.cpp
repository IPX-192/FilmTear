#include <windows.h>
#include "WidgetLog.h"
#include "ui_WidgetLog.h"

WidgetLog::WidgetLog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetLog)
{
    ui->setupUi(this);
}

WidgetLog::~WidgetLog()
{
    delete ui;
}


void WidgetLog::addLog(QString msg, LogLevel level)
{
    if (m_rowTotal>=1000) {
        m_rowTotal = 0;
        ui->textBrowser->clear();
    }
        m_rowTotal++;
	int year, month, day, hour, min, sec, ms;
	SYSTEMTIME tm;
	GetLocalTime(&tm);
	year = tm.wYear;
	month = tm.wMonth;
	day = tm.wDay;
	hour = tm.wHour;
	min = tm.wMinute;
	sec = tm.wSecond;
    ms = tm.wMilliseconds;
	QString strTime=QString::asprintf("[%02d-%02d %02d:%02d:%02d.%03d] ",
	     month, day, hour, min, sec, ms);
    QString strText = strTime + msg + "\n";
	ui->textBrowser->moveCursor(QTextCursor::End);
	QTextCursor cursor = ui->textBrowser->textCursor();
	QTextCharFormat fmt;
    QFont font;
    font.setPointSize(10);
    fmt.setFont(font);
	if (level == LogLevel::Log_Info)
		fmt.setForeground(Qt::black);
	else if(level == LogLevel::Log_Debug){
		fmt.setForeground(Qt::blue);
	}
	else {
		fmt.setForeground(Qt::red);
	}
	cursor.insertText(strText, fmt);
	ui->textBrowser->moveCursor(QTextCursor::End);
	/*if(level == LogLevel::Log_Error)
	{
		ui->textBrowser->append(QString("<font size=4 color=red><p style='line-height:5px; width:100% ; white-space: pre-wrap; '> %1 </p></font>").arg(strText));
	}else
	{
		ui->textBrowser->append(strTime+QString("<font size=4 color=black><p style='line-height:5px; width:100% ; white-space: pre-wrap; '> %1 </p></font>").arg(strText));
	}*/
}

void WidgetLog::clearLog()
{
    ui->textBrowser->clear();
}

void WidgetLog::on_toolButton_clear_clicked()
{
    ui->textBrowser->clear();
}
