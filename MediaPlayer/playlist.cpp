#include "playlist.h"
#include <QFileDialog>
#include <QLayout>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QAbstractItemModel>
#include <QListWidgetItem>
#include <QString>
#include "mediaplayer.h"

Playlist::Playlist(QWidget* parent)
	: QWidget(parent)
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int screenWidth = screenRect.width();
	int screenHeight = screenRect.height();
	int iconSize = 0.03 * screenHeight;
	this->resize(0.22*screenWidth, parent->height());
	this->move(parent->pos().x() + parent->width(), parent->pos().y() + dynamic_cast<MediaPlayer*>(parent)->getTitleBarHeight());
	QString mainStyleSheet = QString
	{
		R"(
			background-color: rgba(30,30,30,255);
		)"
	};
	this->setStyleSheet(mainStyleSheet);
	setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	mAddContent = new Button(this);
	mAddContent->setFixedHeight(0.08 * screenHeight);
	mAddContent->setIcon(QIcon(":/plate-images/plus.png"));
	mAddContent->setIconSize(QSize(iconSize, iconSize));
	mAddContent->setFixedSize(mAddContent->height(), mAddContent->height());
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
	}.arg(mAddContent->height()/2);
	mAddContent->setStyleSheet(buttonStyleSheet);
	mContentListWidget = new QListWidget(this);
	QString listWidgetStyleSheet = QString
	{
		R"(
			background-color: rgba(%1,%2,%3,%4);
			color: rgba(%5, %6, %7, %8);
			font-size: 14px;
		)"
	}.arg(mContentBackground1.color().red()).arg(mContentBackground1.color().green()).arg(mContentBackground1.color().blue()).arg(mContentBackground1.color().alpha())
		.arg(mTextColor.red()).arg(mTextColor.green()).arg(mTextColor.blue()).arg(mTextColor.alpha());
	mContentListWidget->setStyleSheet(listWidgetStyleSheet);
	mContentListWidget->addItems(mCutedContentList);
	mContentListWidget->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
	mContentListWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	QItemSelectionModel* selectionModel = mContentListWidget->selectionModel();
	QAbstractItemModel* itemModel = mContentListWidget->model();

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 5);
	mainLayout->addWidget(mContentListWidget);
	QHBoxLayout* horizontalLayout = new QHBoxLayout;
	horizontalLayout->addStretch(1);
	horizontalLayout->addWidget(mAddContent);
	horizontalLayout->addStretch(1);
	mainLayout->addLayout(horizontalLayout);
	auto addContent = [=]()
	{
		QFileDialog fileDialog(this);
		fileDialog.setWindowFlag(Qt::WindowStaysOnTopHint);
		fileDialog.setFileMode(QFileDialog::ExistingFiles);
		QList<QString> fileNames = fileDialog.getOpenFileNames(this, "Choose file", QString(), "Video files (*.mp4 *.avi *.mkv)");
		if (fileNames.size() > 0)
		{
			mContentList.append(fileNames);
			mCutedContentList.clear();
			mCutedContentList = mContentList;
			for (auto& i : mCutedContentList)
			{
				int last_slash{ 0 };
				for (int c = 0; c < i.size(); c++)
				{
					if (i.at(c) == '/')
						last_slash = c;
				}
				i.erase(i.begin(), i.begin() + last_slash + 1);
			}
			mContentListWidget->clearSelection();
			mContentListWidget->clear();
			mContentListWidget->addItems(mCutedContentList);
			for (int row = 0; row < mCutedContentList.count(); row++)
			{
				if (row == mRowPlaying)
					mContentListWidget->item(row)->setBackground(mRowPlayingBackground);
				else
				{
					if (row % 2 != 0)
						mContentListWidget->item(row)->setBackground(mContentBackground1);
					else
						mContentListWidget->item(row)->setBackground(mContentBackground2);
				}
			}
			emit contentChanged();
			if (mRowPlaying == -1 && mContentList.size() > 0)
			{
				mRowPlaying = 0;
				setPlayingContent(mRowPlaying);
			}
		}
	};
	connect(mAddContent, &Button::clicked, addContent);
	connect(mAddContent, &Button::spaceClicked, [=]() {emit spaceClicked(); });

	auto itemSelected = [=]()
	{
		mSelectedContentIndexes.clear();
		for (auto& i : selectionModel->selectedIndexes())
		{
			mSelectedContentIndexes.push_back(i.row());
		}
	};
	connect(mContentListWidget, &QListWidget::itemSelectionChanged, itemSelected);

	auto itemMoved = [=](const QModelIndex& parent, int start, int end, const QModelIndex& destination, int row)
	{
		if (row > start)
		{
			if (start == mRowPlaying)
				mRowPlaying = row - 1;
			else if (start < mRowPlaying && row >= mRowPlaying)
				mRowPlaying--;
			mCutedContentList.insert(row, mCutedContentList.at(start));
			mCutedContentList.erase(mCutedContentList.begin() + start);
			mContentList.insert(row, mContentList.at(start));
			mContentList.erase(mContentList.begin() + start);
		}
		else
		{
			if (start == mRowPlaying)
				mRowPlaying = row;
			else if (start > mRowPlaying && row <= mRowPlaying)
				mRowPlaying++;
			QString cutedTemp = mCutedContentList.at(start);
			QString temp = mContentList.at(start);
			mCutedContentList.erase(mCutedContentList.begin() + start);
			mCutedContentList.insert(row, cutedTemp);
			mContentList.erase(mContentList.begin() + start);
			mContentList.insert(row, temp);
		}
	};
	connect(itemModel, &QAbstractItemModel::rowsMoved, itemMoved);

	auto itemDoubleClicked = [=](QListWidgetItem* item)
	{
		for (int row = 0; row < mContentListWidget->count(); row++)
			if (mContentListWidget->item(row) == item)
				setPlayingContent(row);
	};
	connect(mContentListWidget, &QListWidget::itemDoubleClicked, itemDoubleClicked);

	mDataBase = new DataBase;
}

Playlist::~Playlist()
{
	delete mDataBase;
}

void Playlist::setContentList(QList<QString> contentList)
{
	mContentList = contentList; 
	mCutedContentList = mContentList;
	for (auto& i : mCutedContentList)
	{
		int last_slash{ 0 };
		for (int c = 0; c < i.size(); c++)
		{
			if (i.at(c) == '/')
				last_slash = c;
		}
		i.erase(i.begin(), i.begin() + last_slash + 1);
	}
	mContentListWidget->clearSelection();
	mContentListWidget->clear();
	mContentListWidget->addItems(mCutedContentList);
	for (int row = 0; row < mCutedContentList.count(); row++)
	{
		if (row == mRowPlaying)
			mContentListWidget->item(row)->setBackground(mRowPlayingBackground);
		else
		{
			if (row % 2 != 0)
				mContentListWidget->item(row)->setBackground(mContentBackground1);
			else
				mContentListWidget->item(row)->setBackground(mContentBackground2);
		}
	}
	emit contentChanged();
	if (mRowPlaying == -1 && mContentList.size() > 0)
	{
		mRowPlaying = 0;
		setPlayingContent(mRowPlaying);
	}
}

void Playlist::clearContentList()
{
	mContentList.clear();
	mContentListWidget->clearSelection();
	mContentListWidget->clear();
}

QString Playlist::getContent(int row)
{
	if (row >= mContentList.size())
		return QString();
	else
		return mContentList.at(row);
}

bool Playlist::setPlayingContent(int row)
{
	if (row >= 0 && row < mContentList.size())
	{
		for (int i = 0; i < mContentListWidget->count(); i++)
		{
			if (i == row)
			{
				mContentListWidget->item(i)->setBackground(mRowPlayingBackground);
				mRowPlaying = i;
				emit playingContentChanged(mRowPlaying);
			}
			else
			{
				if (i % 2 != 0)
					mContentListWidget->item(i)->setBackground(mContentBackground1);
				else
					mContentListWidget->item(i)->setBackground(mContentBackground2);
			}
		}
		return true;
	}
	else
		return false;
}

void Playlist::closeEvent(QCloseEvent* event)
{
	event->ignore();
	this->hide();
}

void Playlist::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key::Key_Delete)
	{
		std::sort(mSelectedContentIndexes.rbegin(), mSelectedContentIndexes.rend());
		for (auto i : mSelectedContentIndexes)
		{
			if (i < mRowPlaying)
				mRowPlaying--;
			else if (i == mRowPlaying)
			{
				emit contentDeleted(i);
				mRowPlaying = -1;
			}
			mContentList.erase(mContentList.begin() + i);
			mCutedContentList.erase(mCutedContentList.begin() + i);
		}
		mSelectedContentIndexes.clear();
		mContentListWidget->clear();
		mContentListWidget->addItems(mCutedContentList);
		if (mRowPlaying >= 0 && mRowPlaying < mContentListWidget->count())
			for (int row = 0; row < mCutedContentList.count(); row++)
			{
				if (row == mRowPlaying)
					mContentListWidget->item(row)->setBackground(mRowPlayingBackground);
				else
				{
					if (row % 2 != 0)
						mContentListWidget->item(row)->setBackground(mContentBackground1);
					else
						mContentListWidget->item(row)->setBackground(mContentBackground2);
				}
			}
		emit contentChanged();
	}
	if(event->key() == Qt::Key::Key_Escape)
		emit escapePressed();
	if (event->key() == Qt::Key::Key_Space)
		emit spaceClicked();
}
