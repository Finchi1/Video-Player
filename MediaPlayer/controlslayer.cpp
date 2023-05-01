#include "controlslayer.h"
#include <QPainter>
#include <QLayout>
#include <QGraphicsDropShadowEffect>
#include <QPainterPath>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QApplication>

ControlsLayer::ControlsLayer(QWidget* parent)
	: QWidget(parent)
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	mScreenWidth = screenRect.width();
	mScreenHeight = screenRect.height();
	setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_Hover);
	setMouseTracking(true);
	mPlate = new Plate(this);
	mDurationSlider = new DurationSlider(Qt::Horizontal, this);
	QString horizontalSliderStyle
	{
		R"(	
		QSlider::groove:horizontal {
			height: 10px;
			background: rgba(181, 181, 181, 70);
			border-radius: 2px;
		}

		QSlider::handle:horizontal {
			background: rgba(75, 75, 75, 255);
			border-style: solid;
			border-width: 1px;
			border-color: rgba(75, 75, 75, 255);
			width: 10px;
			margin: 0 0;
			border-radius: 0;
		}

		QSlider::add-page:horizontal {
			background: rgba(181, 181, 181, 70);
			border-radius: 2px;
		}

		QSlider::sub-page:horizontal {
			background-color: rgba(44, 44, 44, 70);
			border-radius: 2px;
		}
		QSlider::groove:horizontal:hover {
			background: rgba(181, 181, 181, 255);
		}
		QSlider::handle:horizontal:hover {
			background: rgba(75, 75, 75, 255);
		}
		QSlider::add-page:horizontal:hover {
			background: rgba(181, 181, 181, 255);
		}
		)"
	};
	mDurationSlider->setStyleSheet(horizontalSliderStyle);
	mVolumeSlider = new VolumeSlider(Qt::Vertical, this);
	QString verticalSliderStyle
	{
		R"(	
		QSlider::groove:vertical {
			width: 10px;
			background: rgba(181, 181, 181, 70);
			border-radius: 2px;
		}

		QSlider::handle:vertical {
			background: rgba(75, 75, 75, 255);
			border-style: solid;
			border-width: 1px;
			border-color: rgba(75, 75, 75, 255);
			height: 10px;
			margin: 0 0;
			border-radius: 0;
		}

		QSlider::add-page:vertical {
			background-color: rgba(44, 44, 44, 70);
			border-radius: 2px;
		}

		QSlider::sub-page:vertical {
			background: rgba(181, 181, 181, 70);
			border-radius: 2px;
		}
		QSlider::groove:vertical:hover {
			background: rgba(181, 181, 181, 255);
		}
		QSlider::handle:vertical:hover {
			background: rgba(75, 75, 75, 255);
		}
		QSlider::add-page:vertical:hover {
			background: rgba(181, 181, 181, 255);
		}
		)"
	};
	mVolumeSlider->setStyleSheet(verticalSliderStyle);
	mVolumeSlider->setRange(0, 100);
	mPositionLabel = new QLabel(this);
	mDurationLabel = new QLabel(this);
	QString durationLabelStyle
	{
		R"(
			QLabel {
				background-color: rgba(0, 0, 0, 0);
				color: rgba(181, 181, 181, 255);
				border-color: rgba(0, 0, 0, 255);
			}
		)"
	};
	mPositionLabel->setStyleSheet(durationLabelStyle);
	mDurationLabel->setStyleSheet(durationLabelStyle);
	QGraphicsDropShadowEffect* effectShadow = new QGraphicsDropShadowEffect(this);
	effectShadow->setOffset(-1, -1);
	effectShadow->setColor(QColor(0, 0, 0, 255));
	mDurationLabel->setGraphicsEffect(effectShadow);
	mDurationLabel->setFont(QFont("Times New Roman", 14));
	mPositionLabel->setGraphicsEffect(effectShadow);
	mPositionLabel->setFont(QFont("Times New Roman", 14));
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	QHBoxLayout* plateLayout = new QHBoxLayout;
	QHBoxLayout* durationLayout = new QHBoxLayout;
	plateLayout->addStretch(1);
	plateLayout->addWidget(mPlate);
	plateLayout->addStretch(1);
	QHBoxLayout* volumeLayout = new QHBoxLayout;
	volumeLayout->addWidget(mVolumeSlider, 0, Qt::AlignRight);
	mainLayout->addLayout(volumeLayout);
	durationLayout->addWidget(mPositionLabel);
	durationLayout->addWidget(mDurationSlider);
	durationLayout->addWidget(mDurationLabel);
	mainLayout->addLayout(durationLayout);
	mainLayout->addLayout(plateLayout);
	mainLayout->setAlignment(Qt::AlignBottom);
	mainLayout->setContentsMargins(mMargin, mMargin, mMargin, mPlate->height());

	connect(mPlate, &Plate::nextClicked, [=]() {emit changedPlayingRow(1); });
	connect(mPlate, &Plate::previousClicked, [=]() {emit changedPlayingRow(-1); });

	connect(&mPlayAtimationTimer, &QTimer::timeout, [=]() {mPlayAtimationTimer.stop(); mIsPlaying = false; this->update(); });

	auto hideAll = [=]()
	{
		if (!mIsMousePressed && !mIsControlsHovered)
		{
			mMouseMoveEventTimer.stop();
			mIsMouseMoved = false;
			if (mPlate->isVisible())
				mPlate->hide();
			if (mDurationSlider->isVisible())
				mDurationSlider->hide();
			if (mVolumeSlider->isVisible())
				mVolumeSlider->hide();
			if (mPositionLabel->isVisible())
				mPositionLabel->hide();
			if (mDurationLabel->isVisible())
				mDurationLabel->hide();
			this->setCursor(Qt::BlankCursor);
			this->update();
		}
	};
	connect(&mMouseMoveEventTimer, &QTimer::timeout, hideAll);

	auto rewind = [&]()
	{
		mRewindTimer.stop();
		mRewindBack = false;
		mRewindForward = false;
		this->update();
	};
	connect(&mRewindTimer, &QTimer::timeout, rewind);

	auto checkControlsHovering = [=]()
	{
		if (!mPlate->isHovered() && !mDurationSlider->isHovered() && !mVolumeSlider->isHovered())
			mIsControlsHovered = false;
		else
			mIsControlsHovered = true;
		this->update();
	};
	connect(mPlate, &Plate::hoverChanged, checkControlsHovering);
	connect(mDurationSlider, &DurationSlider::hoverChanged, checkControlsHovering);
	connect(mVolumeSlider, &VolumeSlider::hoverChanged, checkControlsHovering);

	connect(mPlate, &Plate::spaceClicked, [=]() {emit spaceClicked(); });

	show();
}

ControlsLayer::~ControlsLayer()
{
}

void ControlsLayer::paintText(QPainter* painter, QString text, QPoint pos, QColor pen, QColor brush, int pixelSize, QFont font)
{
	QPainterPath path;
	font.setPixelSize(pixelSize);
	path.addText(pos, font, text);
	painter->setPen(pen);
	painter->setBrush(brush);
	painter->drawPath(path);
}

void ControlsLayer::paintEvent(QPaintEvent* event)
{
	QPainter* painter = new QPainter(this);
	painter->fillRect(this->rect(), QColor(0, 0, 0, 1));
	QRect leftRect(0, 0, this->width() / 4, this->height());
	QRect rightRect(this->width() / 4 * 3, 0, this->width(), this->height());
	QPen pen(Qt::NoPen);
	painter->setPen(pen);
	QBrush brush(QColor(181, 181, 181, 30));
	painter->setBrush(brush);
	qreal k = 0.000001;
	int rewindHeight = 40 * k * mScreenWidth * mScreenHeight;
	int rewindHalfWidth = 30 * k * mScreenWidth * mScreenHeight;

	if (mLeftRectHovered && mMouseMoveEventTimer.isActive() && !mIsControlsHovered)
	{
		QPainterPath rewindBack;
		QPoint startRewindBack(this->width() / 8 - rewindHalfWidth, height() / 2);
		rewindBack.moveTo(startRewindBack.x(), startRewindBack.y());
		rewindBack.lineTo(startRewindBack.x() + rewindHalfWidth, startRewindBack.y() + rewindHeight / 2);
		rewindBack.lineTo(startRewindBack.x() + rewindHalfWidth, startRewindBack.y() - rewindHeight / 2);
		rewindBack.lineTo(startRewindBack.x(), startRewindBack.y());
		rewindBack.moveTo(startRewindBack.x() + rewindHalfWidth, startRewindBack.y());
		rewindBack.lineTo(startRewindBack.x() + rewindHalfWidth * 2, startRewindBack.y() + rewindHeight / 2);
		rewindBack.lineTo(startRewindBack.x() + rewindHalfWidth * 2, startRewindBack.y() - rewindHeight / 2);
		rewindBack.lineTo(startRewindBack.x() + rewindHalfWidth, startRewindBack.y());
		if (!mRewindBack)
			painter->fillPath(rewindBack, QBrush(QColor(181, 181, 181, 90)));
		else
			painter->fillPath(rewindBack, QBrush(QColor(181, 181, 181, 181)));
	}

	if (mRightRectHovered && mMouseMoveEventTimer.isActive() && !mIsControlsHovered)
	{
		QPainterPath rewindForward;
		QPoint startRewindForward(this->width() / 8 * 7 + rewindHalfWidth, height() / 2);
		rewindForward.moveTo(startRewindForward.x(), startRewindForward.y());
		rewindForward.lineTo(startRewindForward.x() - rewindHalfWidth, startRewindForward.y() + rewindHeight / 2);
		rewindForward.lineTo(startRewindForward.x() - rewindHalfWidth, startRewindForward.y() - rewindHeight / 2);
		rewindForward.lineTo(startRewindForward.x(), startRewindForward.y());
		rewindForward.moveTo(startRewindForward.x() - rewindHalfWidth, startRewindForward.y());
		rewindForward.lineTo(startRewindForward.x() - rewindHalfWidth * 2, startRewindForward.y() + rewindHeight / 2);
		rewindForward.lineTo(startRewindForward.x() - rewindHalfWidth * 2, startRewindForward.y() - rewindHeight / 2);
		rewindForward.lineTo(startRewindForward.x() - rewindHalfWidth, startRewindForward.y());
		if (!mRewindForward)
			painter->fillPath(rewindForward, QBrush(QColor(181, 181, 181, 90)));
		else
			painter->fillPath(rewindForward, QBrush(QColor(181, 181, 181, 181)));
	}


	if (mIsPaused)
	{
		int pauseHeight = 40 * k * mScreenWidth * mScreenHeight;
		int pauseWidth = 30 * k * mScreenWidth * mScreenHeight;
		QRect pauseStickRectLeft(this->width() / 2 - pauseWidth / 2, this->height() / 2 - pauseHeight / 2, pauseWidth / 4, pauseHeight);
		QRect pauseStickRectRight(this->width() / 2 + pauseWidth / 4, this->height() / 2 - pauseHeight / 2, pauseWidth / 4, pauseHeight);
		if (!mCenterRectHovered)
		{
			painter->fillRect(pauseStickRectLeft, QBrush(QColor(181, 181, 181, 90)));
			painter->fillRect(pauseStickRectRight, QBrush(QColor(181, 181, 181, 90)));
		}
		else
		{
			painter->fillRect(pauseStickRectLeft, QBrush(QColor(181, 181, 181, 181)));
			painter->fillRect(pauseStickRectRight, QBrush(QColor(181, 181, 181, 181)));
		}
	}
	if (mIsPlaying)
	{
		int playHeight = 40 * k * mScreenWidth * mScreenHeight;
		int playWidth = 30 * k * mScreenWidth * mScreenHeight;
		QPainterPath play;
		QPoint startPlay(this->width() / 2 + playWidth / 2, height() / 2);
		play.moveTo(startPlay.x(), startPlay.y());
		play.lineTo(startPlay.x() - playWidth, startPlay.y() + playHeight / 2);
		play.lineTo(startPlay.x() - playWidth, startPlay.y() - playHeight / 2);
		play.lineTo(startPlay.x(), startPlay.y());
		if (!mCenterRectHovered)
			painter->fillPath(play, QBrush(QColor(181, 181, 181, 90)));
		else
			painter->fillPath(play, QBrush(QColor(181, 181, 181, 181)));
	}
	painter->end();
	// Upper layer widgets raise
	for (auto i : mUpperLayerWidgets)
		i->raise();
}

void ControlsLayer::mouseMoveEvent(QMouseEvent* event)
{
	if (event->pos().x() >= 0 && event->pos().x() < this->width() / 4)
		mLeftRectHovered = true;
	else
		mLeftRectHovered = false;
	if (event->pos().x() >= this->width() / 4 && event->pos().x() <= this->width() / 4 * 3)
		mCenterRectHovered = true;
	else
		mCenterRectHovered = false;
	if (event->pos().x() > this->width() / 4 * 3 && event->pos().x() <= this->width())
		mRightRectHovered = true;
	else
		mRightRectHovered = false;
	this->update();
	mMouseMoveEventTimer.start(mTimeToHideControls);
	mIsMouseMoved = true;
	if (!mPlate->isVisible())
		mPlate->show();
	if (!mDurationSlider->isVisible())
		mDurationSlider->show();
	if (!mVolumeSlider->isVisible())
		mVolumeSlider->show();
	if (!mPositionLabel->isVisible())
		mPositionLabel->show();
	if (!mDurationLabel->isVisible())
		mDurationLabel->show();
	if (this->cursor() == Qt::BlankCursor)
		this->setCursor(Qt::ArrowCursor);
}

void ControlsLayer::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mIsMousePressed = true;
		this->update();
		mMouseMoveEventTimer.start(mTimeToHideControls);
		mIsMouseMoved = true;
		if (!mPlate->isVisible())
			mPlate->show();
		if (!mDurationSlider->isVisible())
			mDurationSlider->show();
		if (!mVolumeSlider->isVisible())
			mVolumeSlider->show();
		if (!mPositionLabel->isVisible())
			mPositionLabel->show();
		if (!mDurationLabel->isVisible())
			mDurationLabel->show();
		if (this->cursor() == Qt::BlankCursor)
			this->setCursor(Qt::ArrowCursor);
	}
}

void ControlsLayer::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		mIsMousePressed = false;
		this->update();
		mMouseMoveEventTimer.start(mTimeToHideControls);
		mIsMouseMoved = true;
		if (!mPlate->isVisible())
			mPlate->show();
		if (!mDurationSlider->isVisible())
			mDurationSlider->show();
		if (!mVolumeSlider->isVisible())
			mVolumeSlider->show();
		if (!mPositionLabel->isVisible())
			mPositionLabel->show();
		if (!mDurationLabel->isVisible())
			mDurationLabel->show();
		if (this->cursor() == Qt::BlankCursor)
			this->setCursor(Qt::ArrowCursor);
	}
}

void ControlsLayer::mouseDoubleClickEvent(QMouseEvent* event)
{
	// left
	if (event->pos().x() > 0 && event->pos().x() < this->width() / 4)
	{
		emit rewindBack();
		mRewindTimer.start(500);
		mRewindBack = true;
		this->update();
	}
	// center
	else if (event->pos().x() >= this->width() / 4 && event->pos().x() <= this->width() / 4 * 3)
	{
		emit mPlate->playPauseClicked();
	}
	// right
	else if (event->pos().x() >= this->width() / 4 * 3 && event->pos().x() <= this->width())
	{
		emit rewindForward();
		mRewindTimer.start(500);
		mRewindForward = true;
		this->update();
	}
}

void ControlsLayer::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Escape)
		emit escapePressed();
	if (event->key() == Qt::Key::Key_Space)
		emit spaceClicked();
}

void ControlsLayer::leaveEvent(QEvent* event)
{
	mLeftRectHovered = false;
	mCenterRectHovered = false;
	mRightRectHovered = false;
}
