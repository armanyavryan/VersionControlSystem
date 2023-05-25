#include "gitindexfile.h"
#include <iostream>
#include <QByteArray>

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QtCore>

#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <iostream>
#include <bitset>
#include <iomanip>


using namespace std;


// command for binary representation
//$ xxd -b -c 4 .git/index

// command for hex representation
//$ xxd .git/index


QString HexToAscii(QString String)

{

	QByteArray ByteArray1 = String.toUtf8();

	const char* chArr1 = ByteArray1.constData();

	QByteArray ByteArray2 = QByteArray::fromHex(chArr1);

	const char* chArr2 = ByteArray2.constData();

	return QString::fromUtf8(chArr2);

}


GitIndexFile::GitIndexFile()
{
}


void GitIndexFile::test_write(const QDir& dir, const QStringList& filesNamesList)
{
	open(dir);

	std::string hexStream = "";
	
	QStringList filesNames;

	for (size_t i = 1; i < infos.size(); i++)
	{
		filesNames.append(infos[i].entry_path_name.c_str());
	}
	for (size_t i = 0; i < filesNamesList.size(); i++)
	{
		if (!filesNames.contains(filesNamesList[i])) {
			filesNames.append(filesNamesList[i]);
		}
	}

	infos.clear();

	if (infos.size() == 0) {
		QByteArray stream;

		EntryInfo dirc;
		dirc.signature[0] = 'D';
		dirc.signature[1] = 'I';
		dirc.signature[2] = 'R';
		dirc.signature[3] = 'C';
		dirc.version = 2;
		//    dirc.version = 2'147'483'648;
		//    dirc.entriesCount = qint32(files.count());
		dirc.entriesCount = filesNames.count();

		infos.push_back(dirc);


		for (int i = 0; i < filesNames.count(); ++i) {
			QFile file(dir.absoluteFilePath(filesNames[i]));
			if (file.exists()) {
				if (file.open(QFile::OpenModeFlag::ReadOnly | QIODevice::Text)) {
					QCryptographicHash hash(QCryptographicHash::Sha1);
					auto strbyt = file.readAll();
					auto str = strbyt.toStdString();
					auto content = str.data();
					// text hello hash = b6fc4c620b67d95f953a5c1c1230aaab5db5a1b0
					//                    auto content = "hello";
					auto size = strlen(content);
					QByteArray header = QString("blob %1").arg(size).toLatin1(); //    QByteArray header = QString("blob %1").arg(file.size()).toUtf8();
					hash.addData(header.data(), header.size() + 1);
					hash.addData(content);
					auto shaFile = hash.result().toHex();

					//                   qDebug() << "100644" << shaFile.toStdString().data() << 0 << files[i];
					file.close();

					// add object
					{
						bool ok;
						QDir d = dir;
						ok = d.cd(".git");
						ok = d.cd("objects");
						if (!ok) {
							ok = d.mkdir("objects");
							if (!ok) {
								std::cout << "could not create object file " << std::endl;
							}
							else {
								ok = d.cd("objects");
							}
						}
						if (!ok) {
							std::cout << "could not create directory for object file " << std::endl;
						}
						else {

							auto shastr = shaFile.toStdString();
							QString folder = QString::fromStdString(shastr.substr(0, 2));
							QString file_name = QString::fromStdString(shastr.substr(2, 38));

							ok = d.cd(folder);

							if (!ok) {
								ok = d.mkdir(folder);
								if (!ok) {
									std::cout << "could not create object file folder " << std::endl;
								}
								else {
									ok = d.cd(folder);
									if (ok) {
										QFile hashFile(d.absoluteFilePath(file_name));
										if (hashFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
											hashFile.write(content);
										}
									}
									else {

									}
								}
							}

						}

					}


					struct stat sb;
					stat(dir.absoluteFilePath(filesNames[i]).toStdString().c_str(), &sb);


					EntryInfo entry;
					entry.signature[0] = 'F';
					entry.signature[1] = 'I';
					entry.signature[2] = 'L';
					entry.signature[3] = 'E';
					entry.version = 2;
					entry.file_size = size;
					entry.entry_path_name = filesNames[i].toStdString();
					entry.ctime_sec = sb.st_ctime;
					entry.mtime_sec = sb.st_mtime;
					entry.dev = sb.st_dev;
					entry.gid = sb.st_gid;
					entry.ino = sb.st_ino;
					entry.uid = sb.st_uid;
					entry.mode = sb.st_mode;

					entry.object_name = hash.result();

					{
						auto object_type = 0b1000'0000'0000'0000;
						auto permission = 0b0000'0001'1111'1111;

						entry.object_type_and_permission = object_type | permission;
					}
					{
						auto assume_valid	= 0b1000'0000'0000'0000;
						auto extended_flag	= 0b0100'0000'0000'0000;
						auto stage			= 0b0011'0000'0000'0000;
						auto name_length = entry.entry_path_name.size();
						if (name_length < 0xFFF) {
							entry.flags = assume_valid | extended_flag | stage | name_length;
						}
						else {
							entry.flags = assume_valid | extended_flag | stage | 0xFFF;
						}
					}

					std::cout << "Added Entry " << std::endl;
					std::cout << entry.toString() << std::endl;

					infos.push_back(entry);

				}

			}
		}

		index(stream, infos);
		QFile file(dir.absoluteFilePath(".git\\index"));
		if (file.open(QIODevice::OpenModeFlag::ReadWrite)) {
			file.write(stream);
			file.close();
		}
		else {
			std::cout << "Cant create file " << file.fileName().toStdString() << std::endl;
		}
	}

	else {

		for (int i = 0; i < filesNames.count(); ++i) {
			QFile file(dir.absoluteFilePath(filesNames[i]));
			if (file.exists()) {
				if (file.open(QFile::OpenModeFlag::ReadOnly | QIODevice::Text)) {
					QCryptographicHash hash(QCryptographicHash::Sha1);
					auto strbyt = file.readAll();
					auto str = strbyt.toStdString();
					auto content = str.data();
					// text hello hash = b6fc4c620b67d95f953a5c1c1230aaab5db5a1b0
					//                    auto content = "hello";
					auto size = strlen(content);
					QByteArray header = QString("blob %1").arg(size).toLatin1(); //    QByteArray header = QString("blob %1").arg(file.size()).toUtf8();
					hash.addData(header.data(), header.size() + 1);
					hash.addData(content);
					auto shaFile = hash.result().toHex();

					//                   qDebug() << "100644" << shaFile.toStdString().data() << 0 << files[i];
					file.close();

					// add object
					{
						bool ok;
						QDir d = dir;
						ok = d.cd(".git");
						ok = d.cd("objects");
						if (!ok) {
							ok = d.mkdir("objects");
							if (!ok) {
								std::cout << "could not create object file " << std::endl;
							}
							else {
								ok = d.cd("objects");
							}
						}
						if (!ok) {
							std::cout << "could not create directory for object file " << std::endl;
						}
						else {

							auto shastr = shaFile.toStdString();
							auto shastrsub = shastr.substr(0, 2);
							auto shastrsubdata = shastrsub.c_str();
							QString s1 = QString::fromStdString(shastrsub);
							QString s2 = QString("%0").arg(shastrsub.c_str());

							QString folder = QString::fromStdString(shastr.substr(0, 2));
							QString file_name = QString::fromStdString(shastr.substr(2, 38));

							/*std::stringstream ss;
							ss << std::right << std::hex << shaFile;
							QString folder = ss.str().substr(0, 2).data();
							QString file_name = ss.str().substr(2, 38).data();*/

							ok = d.cd(folder);
							auto pp = d.absolutePath();

							if (!ok) {
								ok = d.mkdir(folder);
								if (!ok) {

									std::cout << "could not create object file folder " << std::endl;
								}
								else {
									ok = d.cd(folder);
								}
							}
							if (!ok) {

								std::cout << "could not create object file folder " << std::endl;
							}
							else {
								QFile hashFile(d.absoluteFilePath(file_name));
								if (hashFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
									hashFile.write(content);
								}
							}


						}

					}


					struct stat sb;
					stat(dir.absoluteFilePath(filesNames[i]).toStdString().c_str(), &sb);
					EntryInfo entry;
					entry.signature[0] = 'F';
					entry.signature[1] = 'I';
					entry.signature[2] = 'L';
					entry.signature[3] = 'E';
					entry.version = 2;
					entry.file_size = size;
					entry.entry_path_name = filesNames[i].toStdString();
					entry.ctime_sec = sb.st_ctime;
					entry.mtime_sec = sb.st_mtime;
					entry.dev = sb.st_dev;
					entry.gid = sb.st_gid;
					entry.ino = sb.st_ino;
					entry.uid = sb.st_uid;
					entry.mode = sb.st_mode;

					entry.object_name = hash.result();


					{
						auto object_type = 0b1000'0000'0000'0000;
						auto permission = 0b0000'0001'1111'1111;

						entry.object_type_and_permission = object_type | permission;
					}
					{
						auto assume_valid	= 0b1000'0000'0000'0000;
						auto extended_flag	= 0b0100'0000'0000'0000;
						auto stage			= 0b0010'0000'0000'0000;
						auto name_length = strlen(entry.entry_path_name.c_str());
						if (name_length < 0xFFF) {
							entry.flags = assume_valid | extended_flag | stage | name_length;
						}
						else {
							entry.flags = assume_valid | extended_flag | stage | 0xFFF;
						}
					}
					infos.push_back(entry);

					std::cout << "Added Entry " << std::endl;
					std::cout << entry.toString() << std::endl;
				}

			}
		}

		QByteArray stream;

		index(stream, infos);
		QFile file(dir.absoluteFilePath(".git\\index"));
		if (file.open(QIODevice::OpenModeFlag::ReadWrite)) {
			file.write(stream);
			file.close();
		}
		else {
			std::cout << "Cant create file " << file.fileName().toStdString() << std::endl;
		}

	}
}

void GitIndexFile::xxd_index(const QDir& dir)
{
		QByteArray data = open(dir);
		if (!data.isEmpty()) {
			beautyPrintIndex(data);
		}
}

void GitIndexFile::dumpStage(const QDir& dir)
{
	auto data = open(dir);
	stringstream ss;

	for (size_t i = 1; i < infos.size(); i++)
	{
		int merge = infos[i].flags > 12;
		merge = infos[i].flags & 0b11;
		auto shastr = infos[i].object_name.toHex().toStdString();
		//QString folder = QString::fromStdString(shastr.substr(0, 2));
		//QString file_name = QString::fromStdString(shastr.substr(2, 38));
		ss  << std::oct << infos[i].mode << " "
			<< std::hex << shastr << " "
			<< std::dec << merge  << "     " << infos[i].entry_path_name << std::endl;
	}

	std::cout << ss.str();
}
void GitIndexFile::dumpDebug(const QDir& dir)
{

	auto data = open(dir);

	for (size_t i = 1; i < infos.size(); i++)
	{
		std::cout << infos[i].entry_path_name << std::endl;
		std::cout << " ctime: " << infos[i].ctime_sec << ":" << infos[i].ctime_nano << std::endl;
		std::cout << " mtime: " << infos[i].mtime_sec << ":" << infos[i].mtime_nano << std::endl;
		std::cout << std::setw(15) << " dev: " << infos[i].dev;
		std::cout << std::setw(15) << " ino: " << infos[i].ino << std::endl;
		std::cout << std::setw(15) << " uid: " << infos[i].uid;
		std::cout << std::setw(15) << " gid: " << infos[i].gid << std::endl;
		std::cout << std::setw(15) << " size: " << infos[i].file_size;
		std::cout << std::setw(15) << " flags: " << infos[i].flags << std::endl;
	}
}

void GitIndexFile::remove(const QDir& dir, const QStringList& filesNames)
{
	if (infos.size() == 0) {
		open(dir);
	}
	bool cacheOnly = false;
	if (filesNames.contains("--cached")) {
		cacheOnly = true;
	}

	bool indexFileChanged = false;

	for (size_t i = 0; i < filesNames.size(); i++)
	{
		if (filesNames[i] == "--cached") {
			continue;
		}
		// remove from working dir
		if (!cacheOnly) {
			QFile file(dir.absoluteFilePath(filesNames[i]));
			if (file.exists()) {
				bool ok = file.remove();
			}
		}
		// remove from index file
		if (infos.size() > 1) {
			for (size_t j = 1; j < infos.size(); j++)
			{
				auto s1 = infos[j].entry_path_name;
				auto s2 = filesNames[i].toStdString();
				if (infos[j].entry_path_name == filesNames[i].toStdString()) {
					infos.erase(infos.begin() + j);
					infos[0].entriesCount--;
					indexFileChanged = true;
					//b_removed_from_index = true;
					break;
				}
			}
		}
	}

	if (indexFileChanged) {
		if (infos.size() > 1) {
			QByteArray stream;
			index(stream, infos);
			auto str = dir.absoluteFilePath(".git\\index");
			QFile file(str);

			if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
				file.write(stream);
				file.close();
			}
		}
	}
}

QByteArray GitIndexFile::open(const QDir& dir)
{
	QByteArray data;
	QFile file(dir.absoluteFilePath(".git\\index"));
	if (file.open(QIODevice::OpenModeFlag::ReadOnly)) {
		auto data = file.readAll();
		parseIndex(data);
		file.close();
		return data;
	}
	return data;
}

FileStatus GitIndexFile::fileStatus(const QDir& dir, const QString& fileName)
{
	FileStatus status = FileStatus::Added;
	
	auto data = open(dir);

	for (size_t i = 1; i < infos.size(); i++)
	{
		QString name = QString::fromStdString(infos[i].entry_path_name);
		if (name == fileName) {
			QFile file(dir.absoluteFilePath(fileName));
			if (file.open(QFile::OpenModeFlag::ReadOnly | QIODevice::Text)) {
				QCryptographicHash hash(QCryptographicHash::Sha1);
				auto strbyt = file.readAll();
				auto str = strbyt.toStdString();
				auto content = str.data();
				// text hello hash = b6fc4c620b67d95f953a5c1c1230aaab5db5a1b0
				//                    auto content = "hello";
				auto size = strlen(content);
				QByteArray header = QString("blob %1").arg(size).toLatin1(); //    QByteArray header = QString("blob %1").arg(file.size()).toUtf8();
				hash.addData(header.data(), header.size() + 1);
				hash.addData(content);
				if (hash.result() == infos[i].object_name) {
					status = FileStatus::UpToDate;
				}
				else {
					status = FileStatus::Modified;
				}
				return status;
			}
		}
	}

	status = FileStatus::Added;
	return  status;
}

QStringList GitIndexFile::getStagedFiles(const QDir& dir)
{
	QStringList list;
	if (infos.size() < 2) {
		open(dir);
	}
	for (size_t i = 1; i < infos.size(); i++)
	{
		list.append(QString::fromStdString(infos[i].entry_path_name));
	}

	return list;

}

std::string GitIndexFile::getTimeString()
{
	std::chrono::time_point<std::chrono::system_clock> timestamp =
		std::chrono::system_clock::now();

	const auto sec = std::chrono::duration_cast<std::chrono::seconds>
		(timestamp.time_since_epoch()).count();
	const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>
		(timestamp.time_since_epoch()).count();

	const auto frac = ns - sec * 1'000'000'000;
	std::string representation = "";
	representation = std::to_string(sec) + ":" + std::to_string(frac);// << ":" << frac;
	return representation;
}

bool GitIndexFile::parse(const QString& content)
{

	return "";
}


std::string GitIndexFile::uint32ToBinaryString(const qint32& num) {
	std::bitset<32> y(num);
	return y.to_string();
}

std::string GitIndexFile::decimalToHexString(const qint32& num) {

	std::stringstream ss;
	ss << std::right << std::setw(8) << std::setfill('0') << std::hex << num;
	auto res = ss.str();
	return res;
}

std::string GitIndexFile::hexToBinary(const QByteArray& arr, bool split) {
	std::string to_str;
	for (int i = 0; i < arr.size(); ++i) {
		std::bitset<8> y(arr[i]);

		to_str.append(y.to_string());
	}

	return to_str;

	{

		//    auto size = arr.size(); //16 ,  16*4 || 16/2 * 8
		//    int length = arr.size() / 2 * (8);

		//    if(split){
		//        length = arr.size() / 2 * (9);
		//    }
		//    std::string data = std::string(length, ' ');

		//    data.reserve(length);
		//    for (int i = 0; i < arr.size(); ++i) {
		//        auto c = arr[i];
		//        bool evenindex = i % 2 == 0;
		////        std::cout << c;
		//        auto delta = c - '0';
		//        int digit = -1;
		//        if(delta >=0 && delta <= 9){
		//            digit = delta;
		//        }
		//        else if(c >= 'a' && c <= 'f'){
		//            digit = 10 + (c - 'a');
		//        }
		//        int p = 1;
		//        for (uint8_t j = 0; j < 4; ++j) {

		//            auto b_and = digit & p;
		//            if(b_and == p){
		//                auto idx = i * 4 + 3 - j;
		//                if(split) {
		//                    idx += i/2;
		//                }
		//                data[idx] = '1';
		//            }
		//            else{
		//                auto idx = i * 4 + 3 - j;
		//                if(split) {
		//                    idx += i/2;
		//                }
		//                data[idx] = '0';
		//            }
		//            p = p << 1;
		//        }

		//        if(evenindex && split){
		//            auto idx = i * 4 + i /2 + 8;
		////            if(split) {
		////                idx += i/2;
		////            }
		//            data[idx] = '_';
		//        }



		//    }

		//    return data;

	}
}

char* GitIndexFile::intToBits(quint32 n) {

	size_t i = 1;
	static char c[32];
	int nth = 31;
	while (nth >= 0) {
		if (n & i) {
			c[nth] = 1;
		}
		else {
			c[nth] = 0;
		}
		i = i << 1;
		--nth;
	}
	return c;
}

void GitIndexFile::pushInt32ToByteArray(QByteArray& stream, const quint32& var) {

	char a = (var >> 24) & 0XFF;
	char b = (var >> 16) & 0XFF;
	char c = (var >> 8) & 0XFF;
	char d = var & 0XFF;

	stream.append(a);
	stream.append(b);
	stream.append(c);
	stream.append(d);
}

void GitIndexFile::pushInt16ToByteArray(QByteArray& stream, const quint16& var) {
	char c = (var >> 8) & 0XFF;
	char d = var & 0XFF;
	stream.append(c);
	stream.append(d);
}





void GitIndexFile::index(QByteArray& stream, std::vector<EntryInfo> infos) {
	
	std::string hexstream = "";

	if (infos.size() < 2) {
		return;
	}
	for (int _ = 0; _ < infos.size(); ++_) {
		auto i = infos[_];
		if (
			i.signature[0] == 'D' &&
			i.signature[1] == 'I' &&
			i.signature[2] == 'R' &&
			i.signature[3] == 'C'
			) {
			//            std::cout << "size: " << sizeof(i.signature[0]) << std::endl;
			stream.append(i.signature[0]);
			stream.append(i.signature[1]);
			stream.append(i.signature[2]);
			stream.append(i.signature[3]);

			hexstream = stream.toHex(' ');


			unsigned char* pointer = (unsigned char*)&(i.version);
			//            std::cout << "size: " << sizeof(pointer[0]) << std::endl;
			pushInt32ToByteArray(stream, i.version);
			pushInt32ToByteArray(stream, i.entriesCount);
			
			hexstream = stream.toHex(' ');

			//
			//stream.append((char)(pointer[3]));
			//stream.append((char)(pointer[2]));
			//stream.append((char)(pointer[1]));
			//stream.append((char)(pointer[0]));
			//pointer = (unsigned char*)&(i.entriesCount);
			//stream.append((char)(pointer[3]));
			//stream.append((char)(pointer[2]));
			//stream.append((char)(pointer[1]));
			//stream.append((char)(pointer[0]));


		}
		else {
			pushInt32ToByteArray(stream, i.ctime_sec);
			pushInt32ToByteArray(stream, i.ctime_nano);
			hexstream = stream.toHex(' ');

			pushInt32ToByteArray(stream, i.mtime_sec);
			hexstream = stream.toHex(' ');
			pushInt32ToByteArray(stream, i.mtime_nano);
			hexstream = stream.toHex(' ');

			pushInt32ToByteArray(stream, i.dev);
			hexstream = stream.toHex(' ');
			pushInt32ToByteArray(stream, i.ino);
			hexstream = stream.toHex(' ');
			pushInt32ToByteArray(stream, i.mode);
			hexstream = stream.toHex(' ');
			pushInt32ToByteArray(stream, i.uid);
			hexstream = stream.toHex(' ');
			pushInt32ToByteArray(stream, i.gid);
			hexstream = stream.toHex(' ');
			pushInt32ToByteArray(stream, i.file_size);
			hexstream = stream.toHex(' ');
			stream.append(i.object_name);
			pushInt16ToByteArray(stream, i.flags);
			hexstream = stream.toHex(' ');

			stream.append(i.entry_path_name);
			hexstream = stream.toHex(' ');
			quint8 zero = 0;
			//stream.append(zero);

			// 
			//int entry_byte_size_total_to_path_name = 62;

			auto fillBytesCount = (i.entry_path_name.size() % 2);
			for (size_t i = 0; i < fillBytesCount; i++)
			{
				stream.append(zero);
				hexstream = stream.toHex(' ');
			}

		}

	}
}

void GitIndexFile::parseIndex(const QByteArray& data) {

	auto hexadecimal = data.toHex();
	const char* cp = data.data();
	infos.clear();
	bool parse_signature = true;

	std::string str_vers;
	bool parse_version = false;
	bool parse_entries_count = false;
	bool parse_ctime = false;
	bool parse_mtime = false;
	bool parse_dev = false;
	bool parse_ino = false;
	bool parse_mode = false;
	bool parse_uid = false;
	bool parse_gid = false;
	bool parse_file_size = false;
	bool parse_hash = false;
	bool parse_flags = false;
	bool parse_file_path = false;
	bool parse_checksum = false;

	int entryIndex = 0;
	infos.push_back(EntryInfo());
	int currentEntryBytesStart = 0;

	for (int i = 0; i < data.size(); ++i) {

		auto hexI = hexadecimal.mid(i*2, 4*2);
		int xxx = i;
		int l = data.size();

		if (parse_signature) {

			hexI = hexadecimal.mid(i * 2, 4 * 2);
			infos[0].signature[0] = data[i++];
			infos[0].signature[1] = data[i++];
			infos[0].signature[2] = data[i++];
			infos[0].signature[3] = data[i];

			parse_signature = false;
			parse_version = true;
		}
		else if (parse_version) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			parse_version = false;
			parse_entries_count = true;
			infos[0].version = ver;
		}
		else if (parse_entries_count) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			for (int nth = 0; nth < ver; ++nth) {
				infos.push_back(EntryInfo());
			}
			parse_entries_count = false;
			infos[0].entriesCount = ver;
			if (ver > 0) {
				parse_ctime = true;
			}
			else {}
		}
		else if (parse_ctime) {
			hexI = hexadecimal.mid(i * 2, 8 * 2);// 84?
			currentEntryBytesStart = i;
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				auto arjeq = *(cp + i + k);
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 4;
			infos[entryIndex + 1].ctime_sec = ver;
			ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].ctime_nano = ver;
			parse_ctime = false;
			parse_mtime = true;
		}
		else if (parse_mtime) {
			hexI = hexadecimal.mid(i * 2, 8 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 4;
			infos[entryIndex + 1].mtime_sec = ver;
			ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].mtime_nano = ver;
			parse_mtime = false;
			parse_dev = true;
		}
		else if (parse_dev) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].dev = ver;

			parse_dev = false;
			parse_ino = true;
		}
		else if (parse_ino) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].ino = ver;

			parse_ino = false;
			parse_mode = true;
		}
		else if (parse_mode) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].mode = ver;

			parse_mode = false;
			parse_uid = true;
		}
		else if (parse_uid) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].uid = ver;

			parse_uid = false;
			parse_gid = true;
		}
		else if (parse_gid) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			int ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].gid = ver;

			parse_gid = false;
			parse_file_size = true;
		}
		else if (parse_file_size) {
			hexI = hexadecimal.mid(i * 2, 4 * 2);
			quint32 ver = 0;
			for (size_t k = 0; k < 4; k++) {
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 3;
			infos[entryIndex + 1].file_size = ver;

			parse_file_size = false;
			parse_hash = true;
		}
		else if (parse_hash) {
			hexI = hexadecimal.mid(i * 2, 20 * 2);

			infos[entryIndex + 1].object_name = data.mid(i, 20);
			i += 19;

			parse_hash = false;
			parse_flags = true;


		}
		else if (parse_flags) {
			hexI = hexadecimal.mid(i, 2);
			quint16 ver = 0;
			for (size_t k = 0; k < 2; k++) {
				auto arjeq = *(cp + i + k);
				ver = (ver << 8) + (*(cp + i + k) & 0xFF);
			}
			i += 1;

			infos[entryIndex + 1].flags = ver;

			parse_flags = false;
			parse_file_path = true;
		}
		else if (parse_file_path) {
			auto hashstr = infos[entryIndex + 1].object_name.toHex().toStdString();
			auto length = infos[entryIndex + 1].getFileNameLength();
			hexI = hexadecimal.mid(i *2, length);

			auto name = data.mid(i, length).toStdString();
			infos[entryIndex + 1].entry_path_name = name;
			i += length - 1;
			
			/*auto delta = i - currentEntryBytesStart + 1;
			auto bytes_to_skip = delta % 8;
			if (bytes_to_skip > 0) {
				bytes_to_skip = 8 - bytes_to_skip;
			}*/
			auto bytes_to_skip = name.length() % 2;
			
			i += bytes_to_skip;
			parse_file_path = false;
			entryIndex++;
			if (entryIndex < infos[0].entriesCount) {
				parse_ctime = true;
			}
			else {
				parse_checksum = true;
			}
		}
		else if (parse_checksum) {
			auto sub = data.mid(i, 20);
			i += 19;
			auto checksum = sub.toStdString();

		}
	}

}









void GitIndexFile::beautyPrintIndex(const QByteArray& data) {
	//    qDebug()<<"data: " << data;
	//    qDebug()<<"data: " << data.toHex(' ');

	auto size = data.size();
	quint32 address = 0;

	auto hex = data.toHex();
	int i = 0;
	int nth = 0;

	for (int r = 0; (i < hex.size()); ++r) {

		for (int c = 0; c < 8 * 4; ++c) {
			if (c == 0) {
				std::cout << decimalToHexString(address) << ":";
			}
			if (c % 4 == 0) {
				std::cout << " ";
			}

			i = r * 8 * 4 + c;
			if (i < hex.size()) {
				std::cout << hex[i];
			}
			else {
				std::cout << " ";
			}
		}


		std::cout << "  ";
		auto mid = QString::fromUtf8(data.mid(nth++, 16));
		mid = HexToAscii(mid);
		std::cout << mid.toStdString();
		//        for (int _ = 0; _ < 16 && nth < data.size(); ++_) {
		//            auto dd = data.data()[nth++];
		//            std::cout  << dd;
		//        }

		std::cout << std::endl;
		address += 16;
	}


}


