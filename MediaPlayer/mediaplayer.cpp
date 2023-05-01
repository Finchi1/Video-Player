#include "mediaplayer.h"
#include <QLayout>
#include <QEvent>
#include <QFileDialog>
#include <QResizeEvent>
#include <QStackedLayout>
#include <QString>
#include <QKeyEvent>
#include <QScreen>
#include <QRect>

MediaPlayer::MediaPlayer(QWidget* parent)
	: QWidget(parent)
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int screenWidth = screenRect.width();
	int screenHeight = screenRect.height();
	resize(0.47 * screenWidth, 0.63 * screenHeight);
	mDurationSliderHandleWidth = screenWidth / 100;
	mVolumeSliderHandleHeight = screenHeight / 50;

	mControlsLayer = new ControlsLayer(this);
	mPlaylist = new Playlist(this);
	mDataBase = mPlaylist->getDataBase();
	mControlsLayer->setUpperLayerWidget(mPlaylist);
	mControlsLayer->setTimeToHideControls(mTimeToHideControls);
	mPlate = mControlsLayer->getPlate();
	mPlate->previousNextSetEnabled(false);
	mDurationSlider = mControlsLayer->getDurationSlider();
	mDurationSlider->setHandleWidth(mDurationSliderHandleWidth);
	connect(mDurationSlider, &DurationSlider::handlePressed, [=]() {mDurationSliderHandlePressed = true; });
	connect(mDurationSlider, &DurationSlider::handleReleased, [=]() {mDurationSliderHandlePressed = false; });
	mVolumeSlider = mControlsLayer->getVolumeSlider();
	mVolumeSlider->setHandleHeight(mVolumeSliderHandleHeight);
	mPositionLabel = mControlsLayer->getPositionLabel();
	mDurationLabel = mControlsLayer->getDurationLabel();
	mMediaPlayer = new QMediaPlayer(this);
	mVideoWidget = new QVideoWidget(this);
	mAudioOutput = new QAudioOutput(this);
	mMediaPlayer->setVideoOutput(mVideoWidget);
	mMediaPlayer->setAudioOutput(mAudioOutput);

	// Get data from database
	mPlaylist->setContentList(mDataBase->getPlaylist());
	mPlaylist->setPlayingContent(mDataBase->getPlayingRow());
	mVolumeSlider->setSliderPosition(mDataBase->getVolume());

	this->setMinimumSize(mControlsLayer->minimumSize());

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addWidget(mVideoWidget);

	auto playPauseContent = [=]()
	{
		if (mMediaPlayer->playbackState() == QMediaPlayer::PlaybackState::PlayingState)
			mMediaPlayer->pause();
		else if (mMediaPlayer->playbackState() == QMediaPlayer::PlaybackState::PausedState)
		{
			mMediaPlayer->play();
		}
		if (mMediaPlayer->playbackState() == QMediaPlayer::PlaybackState::StoppedState && mPlaylist->getContentList()->size() > 0)
			if(mPlaylist->getPlayingRow() == -1)
			{
				mMediaPlayer->setSource(mPlaylist->getContent(0));
				mPlaylist->setPlayingContent(0);
				mMediaPlayer->play();
			}
			else
			{
				mMediaPlayer->setSource(mPlaylist->getContent(mPlaylist->getPlayingRow()));
				mMediaPlayer->play();
			}
	};
	connect(mPlate, &Plate::playPauseClicked, playPauseContent);
	connect(this, &MediaPlayer::spaceClicked, playPauseContent);
	connect(mControlsLayer, &ControlsLayer::spaceClicked, playPauseContent);
	connect(mPlaylist, &Playlist::spaceClicked, playPauseContent);


	auto fullScreenShow = [=]()
	{
		if (this->isFullScreen())
			this->showNormal();
		else
			this->showFullScreen();
	};
	connect(mPlate, &Plate::fullScreenClicked, fullScreenShow);

	auto mediaPlayerPositionChange = [=]()
	{
		mMediaPlayer->setPosition(mDurationSlider->sliderPosition());
	};
	connect(mDurationSlider, &DurationSlider::handleReleased, mediaPlayerPositionChange);

	auto setDurationSliderPosition = [=](int position)
	{
		if (!mDurationSliderHandlePressed)
			mDurationSlider->setSliderPosition(position);
		QString posH = qvariant_cast<QString>(mMediaPlayer->position() / (1000 * 60 * 60));
		QString posM = qvariant_cast<QString>(mMediaPlayer->position() / (1000 * 60) - posH.toInt() * 60);
		QString posS = qvariant_cast<QString>(mMediaPlayer->position() / 1000 - posM.toInt() * 60 - posH.toInt() * 60 * 60);
		if (posH.length() < 2)
			posH.prepend("0");
		if (posM.length() < 2)
			posM.prepend("0");
		if (posS.length() < 2)
			posS.prepend("0");
		QString durH = qvariant_cast<QString>(mMediaPlayer->duration() / (1000 * 60 * 60));
		QString durM = qvariant_cast<QString>(mMediaPlayer->duration() / (1000 * 60) - durH.toInt() * 60);
		QString durS = qvariant_cast<QString>(mMediaPlayer->duration() / 1000 - durM.toInt() * 60 - durH.toInt() * 60 * 60);
		if (durH.length() < 2)
			durH.prepend("0");
		if (durM.length() < 2)
			durM.prepend("0");
		if (durS.length() < 2)
			durS.prepend("0");
		if (durH.toInt() > 0)
		{
			mPositionLabel->setText(posH + ":" + posM + ":" + posS);
			mDurationLabel->setText(durH + ":" + durM + ":" + durS);
		}
		else
		{
			mPositionLabel->setText(posM + ":" + posS);
			mDurationLabel->setText(durM + ":" + durS);
		}
	};
	connect(mMediaPlayer, &QMediaPlayer::positionChanged, setDurationSliderPosition);

	auto nextPreviousChange = [=](int difference)
	{
		if (mPlaylist->getContentList()->size() > 1 && difference > 0)
			if (static_cast<qsizetype>(mPlaylist->getPlayingRow()) + 1 < mPlaylist->getContentList()->size())
				mPlaylist->setPlayingContent(mPlaylist->getPlayingRow() + 1);
			else
				mPlaylist->setPlayingContent(0);
		else if (mPlaylist->getContentList()->size() > 1 && difference < 0)
			if (static_cast<qsizetype>(mPlaylist->getPlayingRow()) - 1 >= 0)
				mPlaylist->setPlayingContent(mPlaylist->getPlayingRow() - 1);
			else
				mPlaylist->setPlayingContent(mPlaylist->getContentList()->size() - 1);
	};
	connect(mControlsLayer, &ControlsLayer::changedPlayingRow, nextPreviousChange);

	auto playbackState = [=]()
	{
		if (mMediaPlayer->playbackState() == QMediaPlayer::StoppedState)
		{
			mPlate->changePlayPauseButtonState(State::PAUSED);
			mControlsLayer->setPausedState(false);
			mControlsLayer->setPlayingState(false);
			if (mMediaPlayer->position() == mMediaPlayer->duration())
				nextPreviousChange(1);
		}
		else if (mMediaPlayer->playbackState() == QMediaPlayer::PausedState)
		{
			mPlate->changePlayPauseButtonState(State::PAUSED);
			mControlsLayer->setPausedState(true);
			mControlsLayer->setPlayingState(false);
		}
		else if (mMediaPlayer->playbackState() == QMediaPlayer::PlayingState)
		{
			mPlate->changePlayPauseButtonState(State::PLAYING);
			mControlsLayer->setPausedState(false);
			mControlsLayer->setPlayingState(true);
		}
	};
	connect(mMediaPlayer, &QMediaPlayer::playbackStateChanged, playbackState);

	auto changeVolume = [=](int value)
	{
		mAudioOutput->setVolume(value / 100.0f);
	};
	connect(mVolumeSlider, &QSlider::valueChanged, changeVolume);

	mAudioOutput->setVolume(mVolumeSlider->value() / 100.0f);

	auto playlistShowHide = [=]()
	{
		if (!mPlaylist->isVisible())
			mPlaylist->show();
		else
			mPlaylist->hide();
	};
	connect(mPlate, &Plate::playlistClicked, playlistShowHide);

	auto playContent = [=](int row)
	{
		mMediaPlayer->setSource(mPlaylist->getContent(row));
		mMediaPlayer->play();
		mPlate->setEnabled(true);
	};
	connect(mPlaylist, &Playlist::playingContentChanged, playContent);

	auto contentChanged = [=]()
	{
		if (mPlaylist->getContentList()->size() > 1)
			mPlate->previousNextSetEnabled(true);
		else
			mPlate->previousNextSetEnabled(false);
	};
	connect(mPlaylist, &Playlist::contentChanged, contentChanged);

	auto contentDeleted = [=](int index)
	{
		QString source = mMediaPlayer->source().toString();
		// Disk letter to uppercase for correct source strings comparation
		if (source.size() > 0)
			source[0] = source.at(0).toUpper();
		if (mPlaylist->getContentList()->size() > 0 && source == mPlaylist->getContent(index))
		{
			mMediaPlayer->stop();
			mPositionLabel->setText(QString());
			mDurationLabel->setText(QString());
		}
	};
	connect(mPlaylist, &Playlist::contentDeleted, contentDeleted);

	auto rewindBack = [=]()
	{
		if (mMediaPlayer->position() > mRewindDuration)
			mMediaPlayer->setPosition(mMediaPlayer->position() - mRewindDuration);
		else
			mMediaPlayer->setPosition(0);
	};
	connect(mControlsLayer, &ControlsLayer::rewindBack, rewindBack);

	auto rewindForward = [=]()
	{
		if (mMediaPlayer->duration() - mMediaPlayer->position() > mRewindDuration)
			mMediaPlayer->setPosition(mMediaPlayer->position() + mRewindDuration);
		else
			nextPreviousChange(1);
	};
	connect(mControlsLayer, &ControlsLayer::rewindForward, rewindForward);

	auto exitFullScreen = [=]()
	{
		if (this->isFullScreen())
			this->showNormal();
	};
	connect(mPlaylist, &Playlist::escapePressed, exitFullScreen);
	connect(mControlsLayer, &ControlsLayer::escapePressed, exitFullScreen);

	auto setDuration = [=]()
	{
		mDurationSlider->setRange(0, mMediaPlayer->duration());
		if (mPlayLoadedFromDBContent)
		{
			mDurationSlider->setSliderPosition(mDataBase->getPosition());
			mPlayLoadedFromDBContent = false;
		}
	};
	connect(mMediaPlayer, &QMediaPlayer::durationChanged, setDuration);

	// Play if playlist data loaded from databse
	auto playLoadedFromDBContent = [=]()
	{
		mPlayLoadedFromDBContentTimer.stop();
		if (mPlaylist->getContentList()->size() > 0)
		{
			mMediaPlayer->setSource(mPlaylist->getContent(mPlaylist->getPlayingRow()));
			mPlayLoadedFromDBContent = true;
			mMediaPlayer->play();
		}
	};
	connect(&mPlayLoadedFromDBContentTimer, &QTimer::timeout, playLoadedFromDBContent);
	mPlayLoadedFromDBContentTimer.start(500);
}

MediaPlayer::~MediaPlayer()
{

}

void MediaPlayer::setPlaylistsContent(QList<QString> content)
{
	mPlaylist->setContentList(content);
	mPlaylist->setPlayingContent(0);
}

void MediaPlayer::clearPlaylistContent()
{
	mPlaylist->clearContentList();
}

void MediaPlayer::resizeEvent(QResizeEvent* event)
{
	if (mControlsLayer)
		mControlsLayer->resize(this->size());
}

void MediaPlayer::moveEvent(QMoveEvent* event)
{
	if (mControlsLayer)
		mControlsLayer->move(event->pos());
}

void MediaPlayer::closeEvent(QCloseEvent* event)
{
	mDataBase->setPlayingRow(mPlaylist->getPlayingRow());
	mDataBase->setPosition(mMediaPlayer->position());
	mDataBase->setVolume(mAudioOutput->volume() * 100);
	mDataBase->setPlaylist(*mPlaylist->getContentList());
	mMediaPlayer->stop();
	qApp->quit();
}

void MediaPlayer::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Escape && this->isFullScreen())
		this->showNormal();
	if (event->key() == Qt::Key::Key_Space)
		emit spaceClicked();
}

