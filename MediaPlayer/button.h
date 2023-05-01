#pragma once

#include <QPushButton>
#include <QKeyEvent>

class Button  : public QPushButton
{
	Q_OBJECT

public:
	Button(QWidget* parent = nullptr) : QPushButton(parent) {}
	~Button() {}

signals:
	void spaceClicked();

protected:
	void keyPressEvent(QKeyEvent* event) { if(event->key()==Qt::Key::Key_Space) emit spaceClicked(); }
};
