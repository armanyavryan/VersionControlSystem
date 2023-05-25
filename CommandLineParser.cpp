#include "CommandLineParser.h"

#include <QDir>
#include <QFile>
#include <QProcess>

#include <iostream>
#include <string>
#include <windows.h>   

using namespace std;

CommandLineParser::CommandLineParser(QObject* parent)
	: QObject{ parent },
	m_sha1{ QCryptographicHash(QCryptographicHash::Algorithm::Sha1) }

{
	pwd = QDir(".");
	pwd.cdUp();
	pwd.cd("build-VersionControllSystem-Desktop_Qt_6_2_4_MSVC2019_64bit-Debug");
	pwd.cd("project");
}

std::string CommandLineParser::process(const std::string& s, bool tabPressed) {

	//    qDebug() << "/t | processing " << s.data() << " |/t";
	QString str(s.c_str());
	static auto regexp = QRegularExpression("\\s+");
	auto list = str.split(regexp, Qt::SplitBehaviorFlags::SkipEmptyParts);
	if (list.size() > 0) {
		if (list[0] == "git")
		{
			list.removeFirst();
			if (list.size() > 0) {
				if (list[0] == "init") {
					list.removeFirst();
					init(list);
				}
				else if (list[0] == "status") {
					list.removeFirst();
					status(list);
				}
				else if (list[0] == "--help" || list[0] == "-h") {
					help();
				}
				else if (list[0] == "ls-files") {
					list.removeFirst();
					ls_files(list);
				}
				else if (list[0] == "add") {
					list.removeFirst();
					add(list);
				}
				else if (list[0] == "rm") {
					list.removeFirst();
					remove(list);
				}
			}
			else {
				std::cout << std::endl << "Run git --help for more." << std::endl << std::endl;
			}
		}

		else if (list[0] == "cd") {
			list.removeFirst();
			return cd(list, tabPressed);
		}
		else if (list[0] == "ls") {
			list.removeFirst();
			ls(list);
		}
		else if (list[0] == "mkdir") {
			list.removeFirst();
			mkdir(list);
		}
		else if (list[0] == "touch") {
			list.removeFirst();
			touch(list);
		}
		else if (list[0] == "show") {

			auto path = pwd.absolutePath().toLatin1() + "/release";
			qDebug() << "openning path " << path;
			QProcess::startDetached("explorer.exe", { path });
		}
		else if (list[0] == "xxd") {
			list.removeFirst();
			if (list.size() > 0 && list[0] == ".git/index") {
				m_indexFile.xxd_index(pwd);
			}
		}

		else {
			std::cout << std::endl << "No such a command." << std::endl << std::endl;
		}

	}


	return emptystring;
}


void CommandLineParser::mkdir(const QStringList& args)
{
	if (args.size() == 0) {
		std::cout << "\t Please specify a directory name." << std::endl;
		return;
	}
	if (!pwd.mkdir(args[0])) {
		std::cout << "\t No such directory." << std::endl;
	}
}
std::string CommandLineParser::cd(const QStringList& args, bool tabPressed)
{
	if (args.size() == 0) {
		std::cout << "\t Please specify a directory name." << std::endl;
		return emptystring;
	}
	if (!pwd.cd(args[0])) {
		// check if tab pressed
		if (tabPressed) {

			auto list = pwd.entryList();
			std::vector<int> indices;
			for (int i = 0; i < list.count(); ++i) {
				if (list[i].startsWith(args[0])) {
					indices.push_back(i);
				}
			}
			if (indices.empty()) {
				std::cout << "\t Not a directory." << std::endl;
			}
			else {
				auto idx = m_nthTabSelectionIndex % (indices.size());
				return list[indices[idx]].toLatin1().data();
			}
		}
		else {
			std::cout << "\t No such directory." << std::endl;
		}
	}
	return emptystring;
}

void CommandLineParser::ls_files(const QStringList& args)
{
	if (args.count() == 0) {
		// print names only
	}
	if (args.count() > 0) {
		if (args[0] == "--stage") {
			m_indexFile.dumpStage(pwd);
		}
		else if (args[0] == "--debug") {
			m_indexFile.dumpDebug(pwd);
		}
	}

}

void CommandLineParser::ls(const QStringList& args)
{
	auto list = pwd.entryList(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDot | QDir::Filter::NoDotAndDotDot | QDir::Filter::NoDotDot);
	for (int i = 0; i < list.count(); ++i) {
		std::cout << "\t\t" << list[i].toStdString().c_str();
		{
			std::cout << std::endl;
		}
	}
}

void CommandLineParser::init(const QStringList& args)
{
	QDir dir = pwd;

	bool reinitialized = false;
	if (!dir.exists(".git")) {
		dir.mkdir(".git");
	}
	else {
		reinitialized = true;
	}

	auto b1 = dir.cd(".git");
	auto b2 = true;
	auto b3 = true;
	auto b4 = true;
	auto b5 = true;
	auto b6 = true;
	auto b7 = true;
	auto b8 = true;
	auto b9 = true;
	auto configFilePath = dir.absoluteFilePath("config");
	QFile configFile(configFilePath);
	if (!configFile.exists()) {
		b9 = configFile.open(QFile::OpenModeFlag::WriteOnly);
		if (!b9) {
			std::cout << configFile.errorString().toStdString() << " " << configFilePath.toStdString() << std::endl;
		}
	}


	if (!dir.exists("objects")) {
		b2 = dir.mkdir("objects");
	}

	if (!dir.exists("refs")) {
		b3 = dir.mkdir("refs");
	}

	b4 = dir.cd("refs");

	if (!dir.exists("heads")) {
		b5 = dir.mkdir("heads");
	}
	if (!dir.exists("remotes")) {
		b6 = dir.mkdir("remotes");
	}
	if (!dir.exists("tags")) {
		b7 = dir.mkdir("tags");
	}

	b8 = dir.cd("../");
	if (b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9) {
		if (reinitialized) {
			std::cout << "Reinitialized existing Git repository in " << dir.absolutePath().toStdString() << std::endl;
		}
		else {
			std::cout << "Initialized empty Git repository in " << dir.absolutePath().toStdString() << std::endl;
		}
	}
	else {
		std::cout << "failed to create repository." << std::endl;
	}

}

void CommandLineParser::add(const QStringList& args)
{
	static bool recursive = true;

	if (args.count() == 0) {

	}
	else {

		m_indexFile.test_write(pwd, args);

		//        for (int i = 0; i < args.count(); ++i) {
		//            auto f = args[i];
		//            if(f == "."){
		//                f = "./";
		//            }
		//            auto filePath = pwd.absoluteFilePath(f);
		//            QFile file(filePath);

		//            auto exists = file.exists();
		//            if(!exists){
		//                continue;
		//            }
		//            auto opened = file.open(QFile::OpenModeFlag::ReadWrite);
		//            if(opened){
		//                file.close();
		//            }
		//            else {

		//                QDir dir = pwd;
		//                if(!dir.cd(f)){
		//                    std::cout << "wrong argument " << f.toStdString() << std::endl;
		//                    continue;
		//                }
		//                else if(recursive){
		//                    recursive = false;
		//                    auto list = dir.entryList(QDir::Filter::Files);
		//                    add(list);
		//                }
		//            }

		//        }

	}

	if (!recursive) {
		recursive = true;
	}
}

void CommandLineParser::remove(const QStringList& args)
{
	if (args.count() != 0) {

		m_indexFile.remove(pwd, args);

	}
}
void CommandLineParser::diff(const QStringList& args)
{

}

void CommandLineParser::touch(const QStringList& args)
{
	if (args.size() == 0) {
		std::cout << "touch: missing file operand" << std::endl;
		std::cout << "Try 'touch --help' for more information." << std::endl << std::endl;
		return;
	}
	for (int i = 0; i < args.count(); ++i) {
		auto filePath = pwd.absoluteFilePath(args[i]);
		QFile f(filePath);
		if (!f.exists()) {
			auto success = f.open(QFile::OpenModeFlag::WriteOnly);
			if (!success) {
				std::cout << f.errorString().toStdString() << " " << filePath.toStdString() << std::endl;
			}
			else {
				f.close();
			}
		}

	}

}

void CommandLineParser::commit(const QStringList& args)
{

}

void CommandLineParser::push(const QStringList& args)
{

}


enum Color {
	Black = 0,
	Blue = 1,
	Green = 2,
	Aqua = 3,
	Red = 4,
	Purple = 5,
	Yellow = 6,
	White = 7

};

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void CommandLineParser::status(const QStringList& args)
{
	//0 = Black 8 = Gray
	//1 = Blue 9 = Light Blue
	//2 = Green a = Light Green
	//3 = Aqua b = Light Aqua
	//4 = Red c = Light Red
	//5 = Purple d = Light Purple
	//6 = Yellow e = Light Yellow
	//7 = White f = Bright White

	{
		// new file created

		//on branch main
		//
		//no commits yet
		//
		//Untracked files:
		//  (use "git add <file>..." to include in what will be commited)
		//      index.html {in red}
		//
		//nothing added to commit but untracked files present (use "git add" to track)
	}

	{
		// new file added

		//on branch main

		//no commits yet

		//Changes to be commited:
		//  (use "git rm --cached <file>..." to unstage)
		//      new file:   index.html {in green}
		//
		//nothing added to commit but untracked files present (use "git add" to track)
	}
	QDir dir = pwd;
	m_statusHelper.setDir(dir);

	auto staged_files = m_indexFile.getStagedFiles(dir);

	auto list = dir.entryList(QDir::Filter::Files);
	while (list.count() == 0) {
		if (!dir.cdUp()) {
			std::cout << "fatal: not a git repository (or any of the parent directories): .git" << std::endl;
			return;
		}
		list = dir.entryList(QDir::Filter::Files);
	}

	// Added means it is not staged
	std::vector<QString> statusAdded;
	std::vector<QString> statusModified;
	std::vector<QString> statusDeleted;

	for (size_t i = 0; i < list.size(); i++)
	{
		auto s = m_indexFile.fileStatus(dir, list[i]);
		if (s == FileStatus::Added) {
			statusAdded.push_back(list[i]);
		}
		else if (s == FileStatus::Modified) {
			statusModified.push_back(list[i]);
		}
	    /*else if (s == FileStatus::Deleted) {
			statusDeleted.push_back(list[i]);
		}*/
	}

	for (size_t i = 0; i < staged_files.size(); i++)
	{
		if (!list.contains(staged_files[i])) {
			statusDeleted.push_back(staged_files[i]);
		}
	}

	SetConsoleTextAttribute(hConsole, Color::White);

	auto branch_name = "main";
	bool has_commit = false;
	bool nothing_to_commit = true;
	std::cout << "On branch " << branch_name << std::endl << std::endl;

	if (statusModified.size() > 0 || statusDeleted.size() > 0) {
		SetConsoleTextAttribute(hConsole, Color::White);

		std::cout << "Changes not staged for commit:" << std::endl;
		std::cout << "\t\t" << "(use \"git add <file>...\" to update what will be committed)" << std::endl;
		std::cout << "\t\t" << "(use \"git checkout -- <file>...\" to discard changes in working directory)" << std::endl << std::endl;
		
		SetConsoleTextAttribute(hConsole, Color::Green);
		for (size_t i = 0; i < statusModified.size(); i++)
		{
			std::cout << "\t\t\t" << "modified:\t" << statusModified[i].toStdString() << std::endl;
		}
		std::cout << std::endl;

		SetConsoleTextAttribute(hConsole, Color::Red);

		for (size_t i = 0; i < statusDeleted.size(); i++)
		{
			std::cout << "\t\t\t" << "deleted:\t" << statusDeleted[i].toStdString() << std::endl;
		}
		std::cout << std::endl;

		SetConsoleTextAttribute(hConsole, Color::White);
	}


	if (statusAdded.size() > 0) {
		std::cout << "Untracked files:" << std::endl;
		std::cout << "\t\t" << "(use \"git add <file>...\" to include in what will be committed)" << std::endl << std::endl;

		SetConsoleTextAttribute(hConsole, Color::Red);
		for (size_t i = 0; i < statusAdded.size(); i++)
		{
			std::cout << "\t\t\t" << "new file:\t" << statusAdded[i].toStdString() << std::endl;
		}
		std::cout << std::endl;

		SetConsoleTextAttribute(hConsole, Color::White);
	}

	if (!has_commit) {
		std::cout << "No commits yet" << std::endl << std::endl;
	}
	if (nothing_to_commit) {
		std::cout << "nothing to commit (create/copy files and use \"git add\" to track)" << std::endl << std::endl;
	}

}

void CommandLineParser::help()
{
	const char* git_help = R"HELP(
These are common Git commands used in various situations:

start a working area (see also: git help tutorial)
   clone     Clone a repository into a new directory
   init      Create an empty Git repository or reinitialize an existing one

work on the current change (see also: git help everyday)
   add       Add file contents to the index
   mv        Move or rename a file, a directory, or a symlink
   restore   Restore working tree files
   rm        Remove files from the working tree and from the index

examine the history and state (see also: git help revisions)
   bisect    Use binary search to find the commit that introduced a bug
   diff      Show changes between commits, commit and working tree, etc
   grep      Print lines matching a pattern
   log       Show commit logs
   show      Show various types of objects
   status    Show the working tree status

grow, mark and tweak your common history
   branch    List, create, or delete branches
   commit    Record changes to the repository
   merge     Join two or more development histories together
   rebase    Reapply commits on top of another base tip
   reset     Reset current HEAD to the specified state
   switch    Switch branches
   tag       Create, list, delete or verify a tag object signed with GPG

collaborate (see also: git help workflows)
   fetch     Download objects and refs from another repository
   pull      Fetch from and integrate with another repository or a local branch
   push      Update remote refs along with associated objects

'git help -a' and 'git help -g' list available subcommands and some
concept guides. See 'git help <command>' or 'git help <concept>'
to read about a specific subcommand or concept.
See 'git help git' for an overview of the system.
)HELP";

	std::cout << git_help << std::endl;
}

void CommandLineParser::println(const char* data)
{
	std::cout << data << std::endl;
}
void CommandLineParser::print(const char* data)
{
	std::cout << data;
}

void CommandLineParser::printWorkingDirectory()
{
	SetConsoleTextAttribute(hConsole, Color::Green);

	std::cout << "Stealth@MSI " ;
	SetConsoleTextAttribute(hConsole, Color::Purple);
	std::cout << "YAV " ;
	SetConsoleTextAttribute(hConsole, Color::Yellow);
	std::cout << "~/" << pwd.dirName().toStdString();
	SetConsoleTextAttribute(hConsole, Color::Aqua);
	std::cout << " (main)" << std::endl;
	SetConsoleTextAttribute(hConsole, Color::White);
	std::cout << "$ ";

	//std::cout << pwd.absolutePath().toLatin1().data() << std::endl << " $ ";
}

void CommandLineParser::setNthTabSelectionIndex(int index)
{
	m_nthTabSelectionIndex = index;
}

