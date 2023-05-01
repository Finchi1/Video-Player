#pragma once

#include <QSlider>

class VolumeSlider  : public QSlider
{
	Q_OBJECT

public:
	VolumeSlider(QWidget *parent);
	VolumeSlider(Qt::Orientation orientation, QWidget* parent);
	~VolumeSlider();

	void setHandleHeight(int height);
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
	bool mLeftButtonPressed{ false };
	bool mHandlePressed{ false };
	int mHandleHeight{ 0 };
	bool mIsHovered{ false };
};
