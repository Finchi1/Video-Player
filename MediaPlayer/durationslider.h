#pragma once

#include <QSlider>
#include <QTimer>

class DurationSlider  : public QSlider
{
	Q_OBJECT

public:
	DurationSlider(QWidget *parent);
	DurationSlider(Qt::Orientation orientation, QWidget* parent);
	~DurationSlider();
	void setHandleWidth(int width);
	bool isHovered() { return mIsHovered; }

signals:
	void handlePressed();
signals:
	void handleReleased();
signals: 
	void hoverChanged();

protected:
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	QTimer mLeftButtonClickTimer;
	bool mLeftButtonPressed{ false };
	bool mHandlePressed{ false };
	int mHandleWidth{ 0 };
	bool mIsHovered{ false };
};