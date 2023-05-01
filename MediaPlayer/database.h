#pragma once
#include <QtSql/QtSql>
#include <QList>
#include <QString>

class DataBase
{
public:
	DataBase();
	~DataBase();

	void clearPlaylist();
	void setPlaylist(QList<QString> playlist);
	void setPlayingRow(int row);
	void setVolume(int volume);
	void setPosition(qint64 position);
	QList<QString> getPlaylist();
	qint64 getPosition();
	int getVolume();
	int getPlayingRow();

private:
	QSqlDatabase mDataBase;
};

