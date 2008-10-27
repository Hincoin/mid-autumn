#ifndef MA_FILE_SYSTEM_WIN32_HPP
#define MA_FILE_SYSTEM_WIN32_HPP


#include "FileSystem.hpp"

namespace ma{


	//remaind to be implemented
	template<typename Configure>
	class MAFileSystemWin32:public FileSystem<MAFileSystemWin32<Configure> >
	{
		typedef typename Configure::ReadFilePtr ReadFilePtr;
		typedef typename Configure::WriteFilePtr WriteFilePtr;
		typedef typename Configure::FileListPtr FileListPtr;

		typedef typename Configure::ReadFile ReadFile;
		typedef typename Configure::WriteFile WriteFile;
		typedef typename Configure::FileList FileList;
	public:
		//Adds an unzipped archive ( or basedirectory with subdirectories..) to the file system. 
		bool  addFolderFileArchive (const char* filename, bool ignoreCase, bool ignorePaths) 
		{
			return false;
		}

		//Adds an pak archive to the file system. 
		bool  addPakFileArchive (const char* filename, bool ignoreCase, bool ignorePaths)
		{return false;}

		//Adds an zip archive to the file system. 
		bool  addZipFileArchive (const char* filename, bool ignoreCase, bool ignorePaths) 
		{
			return false;
		}
		//Changes the current working directory. 
		bool  changeWorkingDirectoryTo (const char* newDirectory) 
		{
			return false;
		}

		//Opens a file for read access. 
		ReadFilePtr  createAndOpenFile (const char* filename) 
		{
			return ReadFilePtr(new ReadFile);
		}

		//Opens a file for write access. 
		WriteFilePtr  createAndWriteFile (const char* filename, bool append) 
		{
			return WriteFilePtr(new WriteFile);
		}


		//IAttributes *  createEmptyAttributes (video::IVideoDriver *driver) 
		//Creates a new empty collection of attributes, usable for serialization and more. 

		//Creates a list of files and directories in the current working directory and returns it. 
		FileListPtr  createFileList () const  {
			return FileListPtr(new FileList);
		}

		//Creates an IReadFile interface for accessing memory like a file. 
		ReadFilePtr  createMemoryReadFile (void *memory, int len, const char* fileName, bool deleteMemoryWhenDropped) 
		{
			return ReadFilePtr(new ReadFile);
		}

		//Determines if a file exists and could be opened.
		bool  existFile (const char *filename) const  {
			return false;
		}

		//Converts a relative path to an absolute (unique) path, resolving symbolic links if required. 
		std::string  getAbsolutePath (const std::string &filename) const  {return "";}

		//Returns the directory a file is located in. 
		std::string  getFileDir (const std::string &filename) const  
		{
			return "";
		}
		//Get the current working directory.
		const char*  getWorkingDirectory () {return "";}
	};

	//sample config
	
}

#endif