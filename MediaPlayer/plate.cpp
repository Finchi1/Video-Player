#include "plate.h"
#include <QPainter>
#include <QPainterPath>
#include <QLayout>
#include <QGraphicsEffect>
#include <QScreen>
#include <QApplication>

Plate::Plate(QWidget* parent)
	: QWidget(parent)
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int screenWidth = screenRect.width();
	int screenHeight = screenRect.height();
	setFixedHeight(0.08 * screenHeight);
	mVerticalContentMargin = 0.007 * screenHeight;
	mHorizontalContentMargin = 0.007 * screenWidth;
	mIconSize = 0.03 * screenHeight;

	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_MouseTracking);
	setWindowBlur(reinterpret_cast<HWND>(this->winId()));
	setContentsMargins(0, 0, 0, 0);

	QString buttonStyleSheet = QString
	{
		R"(
			QPushButton:!hover
			{
				background-color: rgba(255,255,255,50);
				padding: 5px;
				border-radius: %1px;
				font: bold;
				font-family: Times New Roman;
				font-size: 14;
			}
			QPushButton:hover
			{
				background-color: rgba(255,255,255,150);
				padding: 5px;
				border-radius: %1px;
				font: bold;
				font-family: Times New Roman;
				font-size: 14;
			}
			QPushButton:pressed
			{
				background-color: rgba(255,255,255,255);
				padding: 5px;
				border-radius: %1px;
				font: bold;
				font-family: Times New Roman;
				font-size: 14;
			}
		)"
	}.arg((this->height() - mVerticalContentMargin) / 2);

	mPlayPause = new Button(this);
	mPlayPause->setFixedSize(this->height() - mVerticalContentMargin, this->height() - mVerticalContentMargin);
	mPlayPause->setStyleSheet(buttonStyleSheet);
	mPlayPause->setIcon(QIcon(":/plate-images/play.png"));
	mPlayPause->setIconSize(QSize(mIconSize, mIconSize));

	mFullScreen = new Button(this);
	mFullScreen->setFixedSize(mPlayPause->size());
	mFullScreen->setStyleSheet(buttonStyleSheet);
	mFullScreen->setIcon(QIcon(":/plate-images/fullscreen.png"));
	mFullScreen->setIconSize(QSize(mIconSize, mIconSize));

	mNext = new Button(this);
	mNext->setFixedSize(mPlayPause->size());
	mNext->setStyleSheet(buttonStyleSheet);
	mNext->setIcon(QIcon(":/plate-images/next.png"));
	mNext->setIconSize(QSize(mIconSize, mIconSize));

	mPrevious = new Button(this);
	mPrevious->setFixedSize(mPlayPause->size());
	mPrevious->setStyleSheet(buttonStyleSheet);
	mPrevious->setIcon(QIcon(":/plate-images/perv.png"));
	mPrevious->setIconSize(QSize(mIconSize, mIconSize));

	mPlaylist = new Button(this);
	mPlaylist->setFixedSize(mPlayPause->size());
	mPlaylist->setStyleSheet(buttonStyleSheet);
	mPlaylist->setIcon(QIcon(":/plate-images/open.png"));
	mPlaylist->setIconSize(QSize(mIconSize, mIconSize));

	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(mHorizontalContentMargin, 0, mHorizontalContentMargin, 0);
	mainLayout->addWidget(mPlaylist);
	mainLayout->addWidget(mPrevious);
	mainLayout->addWidget(mPlayPause);
	mainLayout->addWidget(mNext);
	mainLayout->addWidget(mFullScreen);

	connect(mPlaylist, &Button::clicked, [=]() {emit playlistClicked(); });
	connect(mPrevious, &Button::clicked, [=]() {mPreviousNextButtonTimer.start(500); emit previousClicked(); mPrevious->setDisabled(true); });
	connect(mPlayPause, &Button::clicked, [=]() {emit playPauseClicked(); });
	connect(mNext, &Button::clicked, [=]() {mPreviousNextButtonTimer.start(500); emit nextClicked(); mNext->setDisabled(true); });
	connect(mFullScreen, &Button::clicked, [=]() {emit fullScreenClicked(); });
	connect(&mPreviousNextButtonTimer, &QTimer::timeout, [=]() {mPreviousNextButtonTimer.stop();  mNext->setEnabled(true); mPrevious->setEnabled(true); });

	connect(mPlaylist, &Button::spaceClicked, [=]() {emit spaceClicked(); });
	connect(mPrevious, &Button::spaceClicked, [=]() {emit spaceClicked(); });
	connect(mPlayPause, &Button::spaceClicked, [=]() {emit spaceClicked(); });
	connect(mNext, &Button::spaceClicked, [=]() {emit spaceClicked(); });
	connect(mFullScreen, &Button::spaceClicked, [=]() {emit spaceClicked(); });

	show();
}

Plate::~Plate()
{

}

void Plate::changePlayPauseButtonState(State state)
{
	if (state == State::PLAYING)
	{
		mPlayPause->setIcon(QIcon(":/plate-images/pause.png"));
		mPlayPause->setIconSize(QSize(mIconSize, mIconSize));
	}
	else if (state == State::PAUSED)
	{
		mPlayPause->setIcon(QIcon(":/plate-images/play.png"));
		mPlayPause->setIconSize(QSize(mIconSize, mIconSize));
	}
}

void Plate::paintEvent(QPaintEvent* event)
{
	QPainter* painter = new QPainter(this);
	painter->setRenderHint(QPainter::RenderHint::Antialiasing);
	QPainterPath mainPath;
	mainPath.addRoundedRect(this->rect(), mIconSize/2.5, mIconSize/2.5);
	QBrush brush(mPanelColor);
	painter->setBrush(brush);
	painter->setPen(Qt::NoPen);
	painter->drawPath(mainPath);
	painter->end();
}

void Plate::mouseMoveEvent(QMouseEvent* event)
{
	if (!mIsHovered)
	{
		mIsHovered = true;
		emit hoverChanged();
	}
}

void Plate::leaveEvent(QEvent* event)
{
	if (mIsHovered)
	{
		mIsHovered = false;
		emit hoverChanged();
	}
}

void Plate::setWindowBlur(HWND hWnd)
{
	const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
	if (hModule)
	{
		struct ACCENTPOLICY
		{
			int nAccentState;
			int nFlags;
			int nColor;
			int nAnimationId;
		};
		struct WINCOMPATTRDATA
		{
			int nAttribute;
			PVOID pData;
			ULONG ulDataSize;
		};
		typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);
		const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
		if (SetWindowCompositionAttribute)
		{
			ACCENTPOLICY policy = { 3, 0, 0, 0 };
			WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) };
			SetWindowCompositionAttribute(hWnd, &data);
		}
		FreeLibrary(hModule);
	}
}
