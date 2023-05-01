#pragma once

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QApplication>
#include <QStackedLayout>
#include "controlslayer.h"
#include "plate.h"
#include "playlist.h"
#include "durationslider.h"
#include "volumeslider.h"
#include "database.h"

class MediaPlayer : public QWidget
{
	Q_OBJECT

public:
	MediaPlayer(QWidget* parent = nullptr);
	~MediaPlayer();

	int getTitleBarHeight() { return QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight); }
	void setPlaylistsContent(QList<QString> content);
	void clearPlaylistContent();
	void play() { mPlaylist->setPlayingContent(0); mMediaPlayer->play(); };

protected:
	void resizeEvent(QResizeEvent* event) override;
	void moveEvent(QMoveEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:
	ControlsLayer* mControlsLayer{ nullptr };
	Playlist* mPlaylist{ nullptr };
	Plate* mPlate{ nullptr };
	DurationSlider* mDurationSlider{ nullptr };
	VolumeSlider* mVolumeSlider{ nullptr };
	QLabel* mPositionLabel{ nullptr };
	QLabel* mDurationLabel{ nullptr };
	QMediaPlayer* mMediaPlayer{ nullptr };
	QVideoWidget* mVideoWidget{ nullptr };
	QAudioOutput* mAudioOutput{ nullptr };
	int mRewindDuration{ 10000 };
	int mTempMediaPosition{ 0 };
	int mDurationSliderHandleWidth{ 10 };
	int mVolumeSliderHandleHeight{ 10 };
	bool mDurationSliderHandlePressed{ false };
	int mTimeToHideControls{ 5000 };
	QTimer mMediaPlayerSetPositionTimer;
	QTimer mPlayLoadedFromDBContentTimer;
	DataBase* mDataBase{ nullptr };
	bool mPlayLoadedFromDBContent{ false };

signals: 
	void spaceClicked();
};
