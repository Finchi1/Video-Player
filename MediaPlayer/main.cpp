#include "mediaplayer.h"
#include "playlist.h"
#include <QtWidgets/QApplication>
#include <QString>
#include <QSharedMemory>
#include <QByteArray>
#include <QTextCodec>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	QStringList arguments = QCoreApplication::arguments();
	QTextCodec* codec = QTextCodec::codecForName("Windows-1251");

	// Only one application can be runed
	// Write opened by application content to shared memory and set playlist content

	QSharedMemory shared;
	shared.setKey("Finchi");

	static int iteration = 0;
	static bool contentChanged = false;

	if (!shared.create(sizeof(QByteArray)) && shared.error() == QSharedMemory::AlreadyExists)
	{
		if (!shared.attach())
			return -1;
		else
		{
			shared.lock();
			QByteArray data(static_cast<const char*>(shared.constData()), shared.size());
			shared.unlock();
			for (int i = 0; i < data.size(); i++)
			{
				if (data.at(i) == '\u0000')
				{
					data.remove(i, 1);
					i--;
				}
			}
			if (arguments.size()>1 && arguments.at(1).size()>0)
			{
				QString stringRunnings;
				for (int i = 0; i < 3 && data.size() >= 3; i++)
					stringRunnings += data.at(i);
				int intRunnings = stringRunnings.toInt();
				intRunnings++;
				stringRunnings = QString("%1").arg(intRunnings);
				switch (stringRunnings.size())
				{
				case 1:
					stringRunnings.prepend("00");
					break;
				case 2:
					stringRunnings.prepend("0");
					break;
				default:
					break;
				}
				for (int i = 0; i < 3 && data.size() >= 3; i++)
					data[i] = stringRunnings.toStdString().at(i);
				if (data.size() >= 5)
					data[4] = '1'; // set contentChanged = true
				data.append(codec->fromUnicode(arguments.at(1)));
				data.append('\n');
				shared.lock();
				memcpy(shared.data(), data.constData(), data.size());
				shared.unlock();
			}
			return 0;
		}
	}
	else
	{
		QByteArray data;
		if (arguments.size() > 1 && arguments.at(1).size() > 0)
		{
			data.append("001"); // first string in list - it's application running counter with some content opened
			data.append('\n');
			data.append("1"); // second string in list - it's contentChanged variable (0 - false, 1 - true)
			data.append('\n');
			data.append(codec->fromUnicode(arguments.at(1)));
			data.append('\n');
			shared.lock();
			memcpy(shared.data(), data.constData(), data.size());
			shared.unlock();
			//contentChanged = true;
		}
		else
		{
			data.append("000");
			data.append('\n');
			data.append("0");
			data.append('\n');
			shared.lock();
			memcpy(shared.data(), data.constData(), data.size());
			shared.unlock();
		}
	}
	MediaPlayer w;
	QList<QString> content;
	QTimer checkTimer;
	auto checkArgs = [&w, &shared, &content, &codec]()
	{
		static int itr = 0;
		shared.lock();
		QByteArray data(static_cast<const char*>(shared.constData()), shared.size());
		shared.unlock();
		QString unicodeData = codec->toUnicode(data);
		QString stringRunnings;
		for (int i = 0; i < 3 && unicodeData.size() >= 3; i++)
			stringRunnings += unicodeData.at(i);
		if (unicodeData.at(4) == '1')
			contentChanged = true;
		int intRunnings = stringRunnings.toInt();
		QList<QString> playListContent;
		if (content.size() - 1 < intRunnings || contentChanged)
		{
			content.clear();
			QString dataString;
			for (int i = 0; i < unicodeData.size(); i++)
			{
				if (unicodeData.at(i) != '\u0000')
					dataString += unicodeData.at(i);
			}
			QString temp;
			for (int i = 0; i < dataString.size(); i++)
			{
				if (dataString.at(i) != '\n')
				{
					temp += dataString.at(i);
					continue;
				}
				else
				{
					content.push_back(temp);
					temp.clear();
				}
			}
			playListContent = content;
			// Delete from playListContent application running counter and contentChanged
			if (playListContent.size() > 0)
				playListContent.erase(playListContent.begin(), playListContent.begin() + 2);

			// Change playListContent addresses to playlist file addresses format

			for (auto& string : playListContent)
			{
				for (int i = 0; i < string.size(); i++)
				{
					if (i != string.size() - 1 && string.at(i + 1) == '\\' && string.at(i) == '\\')
					{
						string.erase(string.begin() + i, string.begin() + i);
						i--;
						continue;
					}
					else if (string.at(i) == '\\')
						string[i] = '/';
				}
			}
		}

		if (playListContent.size() > 0 && contentChanged)
		{
			//w.clearPlaylistContent();
			data[4] = '0';
			w.setPlaylistsContent(playListContent);
			QString contentSring;
			for (auto& i : playListContent)
			{
				contentSring += i;
				contentSring += '\n';
			}
			shared.lock();
			memcpy(shared.data(), data.constData(), data.size());
			shared.unlock();
			iteration = 0;
			contentChanged = false;
		}

		// Shared memory clear
		if (iteration < 10)
			++iteration;
		if (iteration == 10)
		{
			iteration++;
			data.clear();
			data.append("000");
			data.append('\n');
			data.append("0");
			data.append('\n');
			shared.lock();
			memset(shared.data(), '\0', shared.size());
			memcpy(shared.data(), data.constData(), data.size());
			shared.unlock();
		}
	};
	QObject::connect(&checkTimer, &QTimer::timeout, checkArgs);
	checkTimer.start(100);
	w.setWindowIcon(QIcon(":/window-icon/player_icon.ico"));
	w.show();
	if (content.size() > 1)
		w.play();

	return a.exec();
}
