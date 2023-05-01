#pragma once

#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QBrush>
#include <QString>
#include <QListView>
#include <QListWidget>
#include <QStringListModel>
#include "button.h"
#include "database.h"

class Playlist : public QWidget
{
	Q_OBJECT

public:
	Playlist(QWidget* parent = nullptr);
	~Playlist();

	QList<QString>* getContentList() { return &mContentList; }
	void setContentList(QList<QString> contentList);
	void clearContentList();
	QString getContent(int row);
	bool setPlayingContent(int row);
	int getPlayingRow() { return mRowPlaying; }
	QList<int> getSelected() { return mSelectedContentIndexes; }
	DataBase* getDataBase() { return mDataBase; }

signals:
	void contentChanged();
signals: 
	void contentDeleted(int index);
signals: 
	void playingContentChanged(int row);
signals:
	void escapePressed();
signals: 
	void spaceClicked();

protected:
	void closeEvent(QCloseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private:
	Button* mAddContent{ nullptr };
	QList<QString> mContentList;
	QList<QString> mCutedContentList;
	QListWidget* mContentListWidget{ nullptr };
	QList<int> mSelectedContentIndexes;
	int mRowPlaying{ -1 };
	QBrush mRowPlayingBackground{ QBrush(QColor(16, 78, 16, 255))};
	QBrush mContentBackground1{ QBrush(QColor(15, 15, 15, 255))};
	QBrush mContentBackground2{ QBrush(QColor(45, 45, 45, 255)) };
	QColor mTextColor{ QColor(218,218,218,255) };
	DataBase* mDataBase{ nullptr };
};
