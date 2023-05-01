#include "test.h"
#include <QHBoxLayout>


Test::Test(QWidget *parent)
	: QWidget(parent)
{
	this->resize(640, 480);
	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	text = new QTextEdit(this);
	mainLayout->addWidget(text);
	this->show();
}

Test::~Test()
{}
