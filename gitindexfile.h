#pragma once
#include <QFile>
#include <QDir>
#include <sstream>

enum class EntryFileSignature{
    DirCache,
    Tree,
    RsolveUndoConflicts,
    Link,
};

const std::map<EntryFileSignature, std::string> EntryFileSignatureString{
    {EntryFileSignature::DirCache, "DIRC"},
    {EntryFileSignature::Tree, "TREE"},
    {EntryFileSignature::RsolveUndoConflicts, "REUC"},
    {EntryFileSignature::Link, "link"},
};

enum class EntryFileMode{
    Normal = 100644,
    Executable = 100755,
    SymLink = 120000
};

enum class EntryFileType{
    Blob,
    Tree,
    Link
};

struct EntryInfoTime{
    qint32 seconds;
    qint32 nanosecondsFraction;
    std::string toStdString(){
        std::string representation = "";
        representation = std::to_string(seconds) + ":" + std::to_string(nanosecondsFraction);// << ":" << frac;
        return representation;
    }
    EntryInfoTime(qint32 sec, qint32 nanofrac){
        seconds = sec;
        nanosecondsFraction = nanofrac;
    }
    EntryInfoTime(){
        std::chrono::time_point<std::chrono::system_clock> timestamp =
            std::chrono::system_clock::now();

        seconds = std::chrono::duration_cast<std::chrono::seconds>
                  (timestamp.time_since_epoch()).count();
        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>
                        (timestamp.time_since_epoch()).count();

        nanosecondsFraction = ns -  seconds * 1'000'000'000;

    }
};

///// first
//  git cat-file -p main
//  tree 9ad33400eb38dae9d283039edd7a21b2f2a61eac
//  author Arman Yavryan <armanyavryan@gmail.com> 1684685806 +0400
//  committer Arman Yavryan <armanyavryan@gmail.com> 1684685806 +0400

//  initial commit
////then
//$ git cat-file -p 9ad33400eb38dae9d283039edd7a21b2f2a61eac
//  100644 blob e69de29bb2d1d6434b8b29ae775ad8c2e48c5391    index.html
//  040000 tree 8ede28eed8c7a5113ba5a8aa9704fa6017a996c7    lib

struct EntryInfo{

    char signature[4];
    quint32 version = 0;
    quint32 entriesCount = 0;

    quint32 ctime_sec  = 0;
    quint32 ctime_nano = 0;
    quint32 mtime_sec  = 0;
    quint32 mtime_nano = 0;

    quint32 dev  = 0;
    quint32 ino  = 0;
    quint32 mode = 0;
    quint16 object_type_and_permission = 0; //  use 4 bits, skip 3bits, 9bit unix permission . 0755 and 0644 files, 0000 for Symbolic links and gitlinks
    quint32 uid = 0;
    quint32 gid = 0;
    quint32 file_size = 0;
    QByteArray object_name;
    quint16 flags = 0;   // 1-bit assume-valid flag, 1-bit extended flag (must be zero in version 2),
        // 2-bit stage (during merge), 12-bit name length if the length is less than 0xFFF; otherwise 0xFFF
        // is stored in this field.
    std::string entry_path_name = ""; // (variable length) relative to top level directory

    quint16 getFileNameLength(){
        int sub_bytes = 1 << 12;
        quint16 file_path_length = flags;
        for(size_t k = 0; k < 4; k++){
            if(flags & sub_bytes){
                file_path_length -= sub_bytes;
                sub_bytes = sub_bytes << 1;
            }
        }
        return file_path_length;
    }

    std::string toString() {
        std::stringstream ss;
        ss << entry_path_name << std::endl;
        ss << " ctime: " << ctime_sec << ":" << ctime_nano << std::endl;
        ss << " mtime: " << mtime_sec << ":" << mtime_nano << std::endl;
        ss << std::left << std::setw(15) << "  dev: " << dev;
        ss << std::left << std::setw(15) << " ino: " << ino << std::endl;
        ss << std::left << std::setw(15) << "  uid: " << uid;
        ss << std::left << std::setw(15) << " gid: " << gid << std::endl;
        ss << std::left << std::setw(15) << "  size: " << file_size;
        ss << std::left << std::setw(15) << " flags: " << flags << std::endl;

        return ss.str();
    }
};

struct Person{
    std::string name;
    std::string email;
    EntryInfoTime cTime;
};

struct IndexFile{
    std::vector<EntryInfo> entries;
    Person author;
    Person committer;
};


enum class FileStatus {
    UpToDate,
    Modified,
    Added,
    Deleted
};

class GitIndexFile
{
public:
    GitIndexFile();

    void test_write(const QDir& dir, const QStringList& filesNames);
    void xxd_index(const QDir &dir);
    void dumpDebug(const QDir &dir);
    void dumpStage(const QDir &dir);
    void remove(const QDir& dir, const QStringList& filesNames);

    FileStatus fileStatus(const QDir& dir, const QString& fileName);

    QStringList getStagedFiles(const QDir& dir);

private:
    static std::string getTimeString();
    QByteArray open(const QDir& dir);
    bool parse(const QString& content);

    void beautyPrintIndex(const QByteArray& data);
    void parseIndex(const QByteArray& data);
    void index(QByteArray& stream, std::vector<EntryInfo> infos);
    char* intToBits(quint32 n);
    std::string hexToBinary(const QByteArray& arr, bool split = false);
    std::string decimalToHexString(const qint32& num);

    std::string uint32ToBinaryString(const qint32& num);
    void pushInt16ToByteArray(QByteArray& stream, const quint16& d);
    void pushInt32ToByteArray(QByteArray& stream, const quint32& d);

private:
    QFile m_file;
    std::vector<EntryInfo> infos;
};
