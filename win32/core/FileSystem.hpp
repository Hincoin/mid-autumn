#ifndef INCLUDE_FILESYSTEM_HPP
#define INCLUDE_FILESYSTEM_HPP

#include <string>
#include "CRTPInterfaceMacro.hpp"

namespace ma{
	MA_DECLARE_TYPEDEF_TRAITS_TYPE(ReadFilePtr)
	MA_DECLARE_TYPEDEF_TRAITS_TYPE(WriteFilePtr)
	MA_DECLARE_TYPEDEF_TRAITS_TYPE(FileListPtr)

	MA_DECLARE_TYPEDEF_TRAITS_TYPE(ReadFile)
	MA_DECLARE_TYPEDEF_TRAITS_TYPE(WriteFile)
	MA_DECLARE_TYPEDEF_TRAITS_TYPE(FileList)

	template<typename Derived>
	class FileSystem{
		ADD_CRTP_INTERFACE_TYPEDEF(ReadFilePtr);
		ADD_CRTP_INTERFACE_TYPEDEF(WriteFilePtr);
		ADD_CRTP_INTERFACE_TYPEDEF(FileListPtr);

		ADD_CRTP_INTERFACE_TYPEDEF(ReadFile)
			ADD_CRTP_INTERFACE_TYPEDEF(WriteFile)
			ADD_CRTP_INTERFACE_TYPEDEF(FileList)
	private:
		FileSystem& derived(){return static_cast<Derived&>(*this);}
		const FileSystem& derived()const{return static_cast<Derived&>(*this);}
	public:
		//Adds an unzipped archive ( or basedirectory with subdirectories..) to the file system. 
		bool  addFolderFileArchive (const char* filename, bool ignoreCase, bool ignorePaths) 
		{
			return derived().addFolderFileArchive(filename,ignoreCase,ignorePaths);
		}

		//Adds an pak archive to the file system. 
		bool  addPakFileArchive (const char* filename, bool ignoreCase, bool ignorePaths)
		{return derived().addPakFileArchive(filename,ignoreCase,ignorePaths);}

		//Adds an zip archive to the file system. 
		bool  addZipFileArchive (const char* filename, bool ignoreCase, bool ignorePaths) 
		{
			return derived().addZipFileArchive(filename,ignoreCase,ignorePaths);
		}
		//Changes the current working directory. 
		bool  changeWorkingDirectoryTo (const char* newDirectory) 
		{
			return derived().changeWorkingDirectoryTo(newDirectory);
		}

		//Opens a file for read access. 
		ReadFilePtr  createAndOpenFile (const char* filename) 
		{
			return derived().createAndOpenFile(filename);
		}

		//Opens a file for write access. 
		WriteFilePtr  createAndWriteFile (const char* filename, bool append) 
		{
			return derived().createAndWriteFile(filename,append);
		}


		//IAttributes *  createEmptyAttributes (video::IVideoDriver *driver) 
		//Creates a new empty collection of attributes, usable for serialization and more. 

		//Creates a list of files and directories in the current working directory and returns it. 
		FileListPtr  createFileList () const  {
			return derived().createFileList();
		}

		//Creates an IReadFile interface for accessing memory like a file. 
		ReadFilePtr  createMemoryReadFile (void *memory, int len, const char* fileName, bool deleteMemoryWhenDropped) 
		{
			return derived().createMemoryReadFile(memory,len,fileName,deleteMemoryWhenDropped);
		}

		//Determines if a file exists and could be opened.
		bool  existFile (const char *filename) const  {
			return derived().existFile(filename);
		}

		//Converts a relative path to an absolute (unique) path, resolving symbolic links if required. 
		std::string  getAbsolutePath (const std::string &filename) const  {return derived().getAbsolutePath(filename);}

		//Returns the directory a file is located in. 
		std::string  getFileDir (const std::string &filename) const  
		{
			return derived().getFileDir(filename);
		}
		//Get the current working directory.
		const char*  getWorkingDirectory () {return derived().getWorkingDirectory();}


	protected:
		~FileSystem () {}
	};
}
#endif