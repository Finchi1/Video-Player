#include "volumeslider.h"
#include <QMouseEvent>

VolumeSlider::VolumeSlider(QWidget* parent)
	: QSlider(parent)
{
	setAttribute(Qt::WA_MouseTracking);
}

VolumeSlider::VolumeSlider(Qt::Orientation orientation, QWidget* parent)
{
	setAttribute(Qt::WA_MouseTracking);
	this->setOrientation(orientation);
}

VolumeSlider::~VolumeSlider()
{}

void VolumeSlider::setHandleHeight(int height)
{
	mHandleHeight = height;
	QString styleSheet = this->styleSheet();
	if (this->orientation() == Qt::Vertical)
		styleSheet += "QSlider::handle:vertical {height: %1px;}";
	this->setStyleSheet(styleSheet.arg(mHandleHeight));
}

void VolumeSlider::mouseMoveEvent(QMouseEvent* event)
{
	if (!mIsHovered)
	{
		mIsHovered = true;
		emit hoverChanged();
	}
	if (mLeftButtonPressed && mHandlePressed)
	{
		qreal y = event->pos().y();
		qreal dif = (this->maximum() - this->minimum()) / static_cast<qreal>(this->height());
		if((this->maximum() - this->minimum()) - dif * y>=this->minimum() && (this->maximum() - this->minimum()) - dif * y<= this->maximum())
			this->setSliderPosition((this->maximum() - this->minimum()) - dif * y);
		else if((this->maximum() - this->minimum()) - dif * y > this->maximum())
			this->setSliderPosition(this->maximum());
		else
			this->setSliderPosition(this->minimum());
	}
}

void VolumeSlider::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mLeftButtonPressed = true;
		qreal y = event->pos().y();
		qreal dif = (this->maximum() - this->minimum()) / static_cast<qreal>(this->height());
		qreal handleHeight = mHandleHeight * dif;
		if ((this->maximum() - this->minimum()) - dif * y > this->sliderPosition() - handleHeight)
		{
			mHandlePressed = true;
			emit handlePressed();
		}
	}
}

void VolumeSlider::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		qreal y = event->pos().y();
		qreal dif = (this->maximum() - this->minimum()) / static_cast<qreal>(this->height());
		if ((this->maximum() - this->minimum()) - dif * y >= this->minimum() && (this->maximum() - this->minimum()) - dif * y <= this->maximum())
			this->setSliderPosition((this->maximum() - this->minimum()) - dif * y);
		else if ((this->maximum() - this->minimum()) - dif * y > this->maximum())
			this->setSliderPosition(this->maximum());
		else
			this->setSliderPosition(this->minimum());
		mLeftButtonPressed = false;
		mHandlePressed = false;
		emit handleReleased();
	}
}

void VolumeSlider::leaveEvent(QEvent* event)
{
	if (mIsHovered)
	{
		mIsHovered = false;
		emit hoverChanged();
	}
}
