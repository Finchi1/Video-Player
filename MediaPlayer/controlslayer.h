#pragma once

#include "plate.h"
#include "durationslider.h"
#include "volumeslider.h"
#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QFont>
#include <QTimer>

class ControlsLayer  : public QWidget
{
	Q_OBJECT

public:
	ControlsLayer(QWidget *parent = nullptr);
	~ControlsLayer();

	Plate* getPlate() { return mPlate; }
	DurationSlider* getDurationSlider() { return mDurationSlider; }
	VolumeSlider* getVolumeSlider() { return mVolumeSlider; }
	QLabel* getPositionLabel() { return mPositionLabel; }
	QLabel* getDurationLabel() { return mDurationLabel; }
	void setPausedState(bool paused) { mIsPaused = paused; this->update(); }
	void setPlayingState(bool playing) { mIsPlaying = playing; this->update(); mPlayAtimationTimer.start(5000); }
	void paintText(QPainter* painter, QString text, QPoint pos, QColor pen, QColor brush, int pixelSize = 14, QFont font = QFont("Times New Roman"));
	void setUpperLayerWidget(QWidget* widget) {mUpperLayerWidgets.push_back(widget); }
	void setTimeToHideControls(int time) { mTimeToHideControls = time; }

signals:
	void changedPlayingRow(int difference);
signals:
	void rewindBack();
signals:
	void rewindForward();
signals:
	void escapePressed();
signals:
	void spaceClicked();

protected:
	void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	Plate* mPlate{ nullptr };
	DurationSlider* mDurationSlider{ nullptr };
	VolumeSlider* mVolumeSlider{ nullptr };
	QLabel* mPositionLabel{ nullptr };
	QLabel* mDurationLabel{ nullptr };
	int mMargin{ 10 };
	int mScreenWidth{ 0 };
	int mScreenHeight{ 0 };
	bool mIsPaused{ false };
	bool mIsPlaying{ false };
	bool mLeftRectHovered{ false };
	bool mRightRectHovered{ false };
	bool mCenterRectHovered{ false };
	bool mRewindBack{ false };
	bool mRewindForward{ false };
	bool mIsMouseMoved{false};
	bool mIsMousePressed{ false };
	bool mIsControlsHovered{ false };
	int mTimeToHideControls{ 1000 };
	QTimer mPlayAtimationTimer;
	QTimer mMouseMoveEventTimer;
	QTimer mRewindTimer;
	QList<QWidget*> mUpperLayerWidgets;
};
