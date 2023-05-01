#pragma once

#include <QWidget>
#include <qtextedit.h>

class Test  : public QWidget
{
	Q_OBJECT

public:
	Test(QWidget *parent=nullptr);
	~Test();
	QTextEdit* text{ nullptr };
	
};
