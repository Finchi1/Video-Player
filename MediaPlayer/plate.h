#pragma once

#include "system.h"
#include "button.h"
#include <QWidget>
#include <Windows.h>
#include <QPushButton>
#include <QTimer>

class Plate : public QWidget
{
	Q_OBJECT

public:
	Plate(QWidget* parent = nullptr);
	~Plate();

	void changePlayPauseButtonState(State state);
	void previousNextSetEnabled(bool enabled) { mPrevious->setEnabled(enabled); mNext->setEnabled(enabled); }
	bool isHovered() { return mIsHovered; }

//---
signals:
	void playPauseClicked();
signals:
	void fullScreenClicked();
signals:
	void nextClicked();
signals:
	void previousClicked();
signals:
	void playlistClicked();
signals: 
	void hoverChanged();
signals:
	void spaceClicked();
//---

protected:
	void paintEvent(QPaintEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void leaveEvent(QEvent* event) override;

private:
	void setWindowBlur(HWND hWnd);

private:
	Button* mPlayPause{ nullptr };
	Button* mStop{ nullptr };
	Button* mNext{ nullptr };
	Button* mPrevious{ nullptr };
	Button* mFullScreen{ nullptr };
	Button* mPlaylist{ nullptr };
	int mVerticalContentMargin{ 5 };
	int mHorizontalContentMargin{ 10 };
	int mIconSize{ 25 };
	QColor mPanelColor{QColor(45,45,45,100)};
	QTimer mPreviousNextButtonTimer;
	bool mIsHovered{ false };
};
