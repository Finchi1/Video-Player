#include "durationslider.h"
#include <QMouseEvent>

DurationSlider::DurationSlider(QWidget* parent)
	: QSlider(parent)
{
	connect(&mLeftButtonClickTimer, &QTimer::timeout, [=]() {mLeftButtonClickTimer.stop(); });
	setAttribute(Qt::WA_MouseTracking);
}

DurationSlider::DurationSlider(Qt::Orientation orientation, QWidget* parent)
{
	this->setOrientation(orientation);
	connect(&mLeftButtonClickTimer, &QTimer::timeout, [=]() {mLeftButtonClickTimer.stop(); });
	setAttribute(Qt::WA_MouseTracking);
}

DurationSlider::~DurationSlider()
{}

void DurationSlider::setHandleWidth(int width)
{
	mHandleWidth = width;
	QString styleSheet = this->styleSheet();
	if (this->orientation() == Qt::Horizontal)
		styleSheet += "QSlider::handle:horizontal {width: %1px;}";
	this->setStyleSheet(styleSheet.arg(mHandleWidth));
}

void DurationSlider::mouseMoveEvent(QMouseEvent* event)
{
	if (!mIsHovered)
	{
		mIsHovered = true;
		emit hoverChanged();
	}
	if (mLeftButtonPressed && mHandlePressed)
	{
		qreal x = event->pos().x();
		qreal dif = (this->maximum() - this->minimum()) / static_cast<qreal>(this->width());
		if (dif * x >= this->minimum() && dif * x <= this->maximum())
			this->setSliderPosition(dif * x);
		else if (dif * x > this->maximum())
			this->setSliderPosition(this->maximum());
		else
			this->setSliderPosition(this->minimum());
	}
}

void DurationSlider::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mLeftButtonClickTimer.start(500);
		mLeftButtonPressed = true;

		qreal x = event->pos().x();
		qreal dif = (this->maximum() - this->minimum()) / static_cast<qreal>(this->width());
		qreal handleWidth = mHandleWidth * dif;
		if (x * dif > this->sliderPosition() - handleWidth)
		{
			mHandlePressed = true;
			emit handlePressed();
		}
	}
}

void DurationSlider::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton && mLeftButtonClickTimer.isActive())
	{
		qreal x = event->pos().x();
		qreal dif = (this->maximum() - this->minimum()) / static_cast<qreal>(this->width());
		if (dif * x >= this->minimum() && dif * x <= this->maximum())
			this->setSliderPosition(dif * x);
		else if (dif * x > this->maximum())
			this->setSliderPosition(this->maximum());
		else
			this->setSliderPosition(this->minimum());
	}
	if (event->button() == Qt::LeftButton)
	{
		mLeftButtonPressed = false;
		mHandlePressed = false;
		emit handleReleased();
	}
}

void DurationSlider::leaveEvent(QEvent* event)
{
	if (mIsHovered)
	{
		mIsHovered = false;
		emit hoverChanged();
	}
}
