#include "database.h"

DataBase::DataBase()
{
	mDataBase = QSqlDatabase::addDatabase("QSQLITE");
	mDataBase.setDatabaseName(qApp->applicationDirPath()+"/mediaplayer.db");
	qDebug() << mDataBase.databaseName();
	if (!mDataBase.open())
		return;
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			CREATE TABLE IF NOT EXISTS playlist (
				row TEXT
			);
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
	stringQuery = QString
	{
		R"(
			CREATE TABLE IF NOT EXISTS row (
				row INTEGER
			);
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
	stringQuery = QString
	{
		R"(
			CREATE TABLE IF NOT EXISTS position (
				position INTEGER
			);
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
	stringQuery = QString
	{
		R"(
			CREATE TABLE IF NOT EXISTS volume (
				volume INTEGER
			);
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
}

DataBase::~DataBase()
{
	mDataBase.close();
}

void DataBase::clearPlaylist()
{
	if (!mDataBase.open())
		return;
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			DELETE FROM playlist;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
}

void DataBase::setPlaylist(QList<QString> playlist)
{
	if (!mDataBase.open())
		return;
	clearPlaylist();
	QSqlQuery query;
	for (auto& row : playlist)
	{
		QString stringQuery = QString
		{
			R"(
			INSERT INTO playlist (row)
			VALUES ('%1');
		)"
		}.arg(row);
		query.prepare(stringQuery);
		if (!query.exec())
		{
			qDebug() << "Insertion error! - " << row;
			continue;
		}
	}
}

void DataBase::setPlayingRow(int row)
{
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			DELETE FROM row;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
	stringQuery = QString
	{
		R"(
			INSERT INTO row (row)
			VALUES (%1);
		)"
	}.arg(row);
	query.prepare(stringQuery);
	if (!query.exec())
		return;
}

void DataBase::setVolume(int volume)
{
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			DELETE FROM volume;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
	stringQuery = QString
	{
		R"(
			INSERT INTO volume (volume)
			VALUES (%1);
		)"
	}.arg(volume);
	query.prepare(stringQuery);
	if (!query.exec())
		return;
}

void DataBase::setPosition(qint64 position)
{
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			DELETE FROM position;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return;
	stringQuery = QString
	{
		R"(
			INSERT INTO position (position)
			VALUES (%1);
		)"
	}.arg(position);
	query.prepare(stringQuery);
	if (!query.exec())
		return;
}

QList<QString> DataBase::getPlaylist()
{
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			SELECT * FROM playlist;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return QList<QString>();
	else
	{
		QList<QString> playlist;
		while (query.next())
			playlist.push_back(qvariant_cast<QString>(query.value(0)));
		return playlist;
	}
}

qint64 DataBase::getPosition()
{
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			SELECT * FROM position;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return qvariant_cast<qint64>(0);
	else
	{
		query.next();
		return qvariant_cast<qint64>(query.value(0));
	}
}

int DataBase::getVolume()
{
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			SELECT * FROM volume;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return 50;
	else
	{
		query.next();
		return qvariant_cast<int>(query.value(0));
	}
}

int DataBase::getPlayingRow()
{
	QSqlQuery query;
	QString stringQuery = QString
	{
		R"(
			SELECT * FROM row;
		)"
	};
	query.prepare(stringQuery);
	if (!query.exec())
		return -1;
	else
	{
		query.next();
		return qvariant_cast<int>(query.value(0));
	}
}
