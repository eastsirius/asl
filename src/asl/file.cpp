/**
 * @file file.cpp
 * @brief 文件系统
 * @author 程行通
 */

#include "file.hpp"
#include "time.hpp"
#include <fstream>
#include <cassert>
#include <cstdlib>
#ifdef WINDOWS
#  include <WinSock2.h>
#  include <Windows.h>
#  include <io.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <dirent.h>
#  include <unistd.h>
#  include <fcntl.h>
#endif

namespace ASL_NAMESPACE {
	struct asl_file_ctx_t {
#ifdef WINDOWS
		HANDLE ctx;
#else
		int ctx;
#endif
	};

	struct asl_filefinder_ctx_t {
#ifdef WINDOWS
		HANDLE find;
		WIN32_FIND_DATAA data;
#else
		DIR* dir;
		dirent* data;
#endif
	};

	struct asl_pidfile_ctx_t {
#ifdef WINDOWS
		HANDLE ctx;
#else
		int ctx;
#endif
	};


#ifdef WINDOWS
	static time_t _asl_file_time_to_time_t(LPFILETIME file_time) {
		uint64_t t = file_time->dwHighDateTime;
		t <<= 32;
		t |= file_time->dwLowDateTime;
		return (time_t)(t / 10000000);
	}
#endif


	File::File() : m_nOpenFlags(0), m_nCurPos(0) {
		m_hHandle = (asl_file_ctx_t*)malloc(sizeof(asl_file_ctx_t));
#ifdef WINDOWS
		m_hHandle->ctx = INVALID_HANDLE_VALUE;
#else
		m_hHandle->ctx = -1;
#endif
	}

	File::File(const char* filename, int flags) {
		File();
		Open(filename, flags);
	}

	File::~File() {
		Close();
		free(m_hHandle);
	}

	bool File::Open(const char* filename, int flags) {
		Close();
#ifdef WINDOWS
		int nAccess = 0;
		int nDisposition = 0;
		if(flags & OF_Out) {
			nAccess = GENERIC_WRITE;
			if(flags & OF_NoCreate) {
				nDisposition |= OPEN_EXISTING;
			} else {
				if(flags & OF_App) {
					nDisposition |= CREATE_NEW | OPEN_EXISTING;
				} else {
					nDisposition |= CREATE_ALWAYS;
				}
			}
		} else {
			nDisposition |= OPEN_EXISTING;
			nAccess = GENERIC_READ;
		}
		if(flags & OF_App) {
			nDisposition |= OPEN_EXISTING;
		}

		m_hHandle->ctx = CreateFileA(filename, nAccess, FILE_SHARE_READ, NULL,
			nDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
		bool bRet = m_hHandle->ctx != INVALID_HANDLE_VALUE;
#else
		int nOpenFlags = 0;
		if(flags & OF_Out) {
			nOpenFlags |= O_WRONLY;
			if((flags & OF_NoCreate) == 0) {
				nOpenFlags |= O_CREAT;
			}
			if((flags & OF_App) == 0) {
				nOpenFlags |= O_TRUNC;
			}
		} else {
			nOpenFlags |= O_RDONLY;
		}

		m_hHandle->ctx = open(filename, nOpenFlags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

		bool bRet = m_hHandle->ctx != -1;
#endif
		if(bRet) {
			m_nOpenFlags = flags;
			m_strFilename = filename;
			if(flags & OF_App) {
				Seek(GetFileSize());
			} else {
				m_nCurPos = 0;
			}
		}
		return bRet;
	}

	void File::Close() {
		m_nCurPos = 0;
		m_nOpenFlags = 0;
		m_strFilename = "";
#ifdef WINDOWS
		if(m_hHandle->ctx != INVALID_HANDLE_VALUE) {
			CloseHandle(m_hHandle->ctx);
			m_hHandle->ctx = INVALID_HANDLE_VALUE;
		}
#else
		if(m_hHandle->ctx != -1) {
			close(m_hHandle->ctx);
			m_hHandle->ctx = -1;
		}
#endif
	}

	int File::Write(const char* buf, int size) {
		assert(buf != NULL && size > 0);
		if(!IsWriteMode()) {
			return -1;
		}
#ifdef WINDOWS
		DWORD dwTrans = 0;
		if(!WriteFile(m_hHandle->ctx, buf, size, &dwTrans, NULL)) {
			return -1;
		}
		int nRet = dwTrans;
#else
		int nRet = write(m_hHandle->ctx, buf, size);
#endif
		if(nRet > 0) {
			m_nCurPos += nRet;
		}
		return nRet;
	}

	int File::Read(char* buf, int size) {
		assert(buf != NULL && size > 0);
		if(IsWriteMode()) {
			return -1;
		}
#ifdef WINDOWS
		DWORD dwTrans = 0;
		if(!ReadFile(m_hHandle->ctx, buf, size, &dwTrans, NULL)) {
			return -1;
		}
		int nRet = dwTrans;
#else
		int nRet = read(m_hHandle->ctx, buf, size);
#endif
		if(nRet > 0) {
			m_nCurPos += nRet;
		}
		return nRet;
	}

	size_t File::GetPos() {
		return m_nCurPos;
	}

	void File::Seek(size_t pos, SeekBase base) {
#ifdef WINDOWS
		int nBase = FILE_BEGIN;
		switch(base) {
		case SB_Begin:
			nBase = FILE_BEGIN;
			break;
		case SB_Cur:
			nBase = FILE_CURRENT;
			break;
		case SB_End:
			nBase = FILE_END;
			break;
		default:
			break;
		}
		LARGE_INTEGER src, dst = {0};
		src.QuadPart = pos;
		if(SetFilePointerEx(m_hHandle->ctx, src, &dst, nBase)) {
			m_nCurPos = (size_t)dst.QuadPart;
		}
#else
		int nBase = SEEK_SET;
		switch(base) {
		case SB_Begin:
			nBase = SEEK_SET;
			break;
		case SB_Cur:
			nBase = SEEK_CUR;
			break;
		case SB_End:
			nBase = SEEK_END;
			break;
		default:
			break;
		}
#  ifdef APPLE
		int64_t nRet = lseek(m_hHandle->ctx, pos, nBase);
#  else
		int64_t nRet = lseek64(m_hHandle->ctx, pos, nBase);
#  endif
		if(nRet >= 0) {
			m_nCurPos = nRet;
		}
#endif
	}

	bool File::IsWriteMode() {
		return (m_nOpenFlags & OF_Out) != 0;
	}

	size_t File::GetFileSize() {
#ifdef WINDOWS
		BY_HANDLE_FILE_INFORMATION info = {0};
		if(!GetFileInformationByHandle(m_hHandle->ctx, &info)) {
			return 0;
		}

		size_t nSize = info.nFileSizeHigh;
		nSize <<= 32;
		nSize |= info.nFileSizeLow;

		return nSize;
#else
		struct stat finfo;
		if(stat(m_strFilename.c_str(), &finfo) != 0) {
			return 0;
		}
		return finfo.st_size;
#endif
	}

	time_t File::GetLastAccessTime() {
#ifdef WINDOWS
		BY_HANDLE_FILE_INFORMATION info = {0};
		if(!GetFileInformationByHandle(m_hHandle->ctx, &info)) {
			return 0;
		}

		return _asl_file_time_to_time_t(&info.ftLastAccessTime);
#else
		struct stat finfo;
		if(stat(m_strFilename.c_str(), &finfo) != 0) {
			return 0;
		}
		return finfo.st_atime;
#endif
	}

	time_t File::GetLastWriteTime() {
#ifdef WINDOWS
		BY_HANDLE_FILE_INFORMATION info = {0};
		if(!GetFileInformationByHandle(m_hHandle->ctx, &info)) {
			return 0;
		}

		return _asl_file_time_to_time_t(&info.ftLastWriteTime);
#else
		struct stat finfo;
		if(stat(m_strFilename.c_str(), &finfo) != 0) {
			return 0;
		}
		return finfo.st_mtime;
#endif
	}

	bool File::Access(const char* path) {
#ifdef WINDOWS
		return _access(path, 0) == 0;
#else
		return access(path, 0) == 0;
#endif
	}

	bool File::RemoveFile(const char* filename) {
#ifdef WINDOWS
		return ::DeleteFileA(filename) != FALSE;
#else
		return ::remove(filename) == 0;
#endif
	}


	bool Directory::CreateDir(const char* path) {
#ifdef WINDOWS
		return ::CreateDirectoryA(path, NULL) != FALSE;
#else
		return ::mkdir(path, 0755) == 0;
#endif
	}

	bool Directory::DeleteDir(const char* path, bool recursive) {
		if(recursive) {
			PathFinder finder;
			if(!finder.GetFirstChild(path)) {
				return true;
			}

			do {
				if(finder.CurChild().strFilename == "."
					|| finder.CurChild().strFilename == "..") {
					continue;
				}

				std::string strChild = std::string(path) + "/" + finder.CurChild().strFilename;
				if(finder.CurChild().bIsDir) {
					DeleteDir(strChild.c_str());
				} else {
					File::RemoveFile(strChild.c_str());
				}
			} while(finder.GetNextChild());
		}

		return _DeleteDir(path);
	}

	bool Directory::MakeFilePath(const char* filename) {
		std::string strFilename(filename);
		int nPos = (int)strFilename.find_last_of("/");
		int nPos2 = (int)strFilename.find_last_of("\\");
		nPos = ASL_MAX(nPos, nPos2);
		if(nPos == -1) {
			return true;
		}

		return MakeDirPath(std::string(strFilename.begin(), strFilename.begin() + nPos).c_str());
	}

	bool Directory::MakeDirPath(const char* path) {
		if(File::Access(path)) {
			return true;
		}

		std::string strPath(path);

		if(strPath.length() == 0) {
			return true;
		}

		while(strPath[strPath.length() - 1] == '/' || strPath[strPath.length() - 1] == '\\') {
			strPath = std::string(strPath.begin(), strPath.begin() + (strPath.length() - 1));
		}
		if(strPath.length() == 0) {
			return true;
		}

		int nPos = (int)strPath.find_last_of("/");
		int nPos2 = (int)strPath.find_last_of("\\");
		nPos = ASL_MAX(nPos, nPos2);

		if(nPos != -1) {
			if(!MakeDirPath(std::string(strPath.begin(), strPath.begin() + nPos).c_str())) {
				return false;
			}
		}

		return CreateDir(strPath.c_str());
	}

	bool Directory::_DeleteDir(const char* path) {
#ifdef WINDOWS
		return ::RemoveDirectoryA(path) != FALSE;
#else
		return ::rmdir(path) == 0;
#endif
	}


	PathFinder::PathFinder() {
		m_hHandle = (asl_filefinder_ctx_t*)malloc(sizeof(asl_filefinder_ctx_t));
#ifdef WINDOWS
		m_hHandle->find = INVALID_HANDLE_VALUE;
#else
		m_hHandle->dir = NULL;
		m_hHandle->data = NULL;
#endif
	}

	PathFinder::~PathFinder() {
		Close();
		free(m_hHandle);
	}

	bool PathFinder::GetFirstChild(const char* path) {
		assert(path != NULL);
		Close();

#ifdef WINDOWS
		std::string filter(path + std::string("/*.*"));
		m_hHandle->find = FindFirstFileA(filter.c_str(), &m_hHandle->data);
		if(m_hHandle->find == INVALID_HANDLE_VALUE) {
			return false;
		}
#else
		m_hHandle->dir = opendir(path);
		if(m_hHandle->dir == NULL) {
			return false;
		}

		m_hHandle->data = readdir(m_hHandle->dir);
		if(m_hHandle->data == NULL) {
			Close();
			return false;
		}
#endif
		m_PathName = path;
		_LoadFileInfo();

		return true;
	}

	bool PathFinder::GetNextChild() {
#ifdef WINDOWS
		assert(m_hHandle->find != INVALID_HANDLE_VALUE);
		if(m_hHandle->find == INVALID_HANDLE_VALUE) {
			return false;
		}

		if(!FindNextFileA(m_hHandle->find, &m_hHandle->data)) {
			return false;
		}
#else
		assert(m_hHandle->dir != NULL);
		if(m_hHandle->dir == NULL) {
			return false;
		}

		m_hHandle->data = readdir(m_hHandle->dir);
		if(m_hHandle->data == NULL) {
			return false;
		}
#endif
		_LoadFileInfo();

		return true;
	}

	void PathFinder::Close() {
		m_PathName.clear();

#ifdef WINDOWS
		if(m_hHandle->find != INVALID_HANDLE_VALUE) {
			FindClose(m_hHandle->find);
			m_hHandle->find = INVALID_HANDLE_VALUE;
		}
#else
		m_hHandle->data = NULL;
		if(m_hHandle->dir != NULL) {
			closedir(m_hHandle->dir);
			m_hHandle->dir = NULL;
		}
#endif
	}

	void PathFinder::_LoadFileInfo() {
#ifdef WINDOWS
		m_CurChild.strFilename = m_hHandle->data.cFileName;
		m_CurChild.bIsDir = (m_hHandle->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		m_CurChild.bNormalFile = !m_CurChild.bIsDir;

		if(m_CurChild.bNormalFile) {
			//获取最后修改时间
			m_CurChild.nLastAccessTime = _asl_file_time_to_time_t(&m_hHandle->data.ftLastAccessTime);
			m_CurChild.nLastWriteTime = _asl_file_time_to_time_t(&m_hHandle->data.ftLastWriteTime);

			//获取文件大小
			std::ifstream fin((m_PathName + "/" + m_CurChild.strFilename).c_str(), std::ifstream::in | std::ifstream::binary);
			if(!fin) {
				assert(false);
				m_CurChild.nFileSize = 0;
			} else {
				fin.seekg(0, std::ifstream::end);
				m_CurChild.nFileSize = (size_t)fin.tellg();
				fin.close();
			}
		}
#else
		m_CurChild.strFilename = m_hHandle->data->d_name;
		m_CurChild.bIsDir = m_hHandle->data->d_type == DT_DIR;
		m_CurChild.bNormalFile = m_hHandle->data->d_type == DT_REG;

		if(m_CurChild.bNormalFile) {
			//获取最后修改时间、文件大小
			struct stat finfo;
			if(stat((m_PathName + "/" + m_CurChild.strFilename).c_str(), &finfo) == 0) {
				m_CurChild.nFileSize = finfo.st_size;
				m_CurChild.nLastAccessTime = finfo.st_atime;
				m_CurChild.nLastWriteTime = finfo.st_mtime;
			} else {
				assert(false);
				m_CurChild.nFileSize = 0;
				m_CurChild.nLastAccessTime = 0;
				m_CurChild.nLastWriteTime = 0;
			}
		}
#endif
	}


	std::string AppPath::GetAppFilename() {
		char acBuffer[1024];
#ifdef WINDOWS
		if(GetModuleFileNameA(NULL, acBuffer, 1024) <= 0) {
			return "";
		}
#elif APPLE
		assert(false);
		return "";
#else
		int count = readlink("/proc/self/exe", acBuffer, 1024);
		if(count <= 0 || count >= 1024) {
			return "";
		}
		acBuffer[count] = 0;
#endif
		for(int i = 0; i < 1024; ++i) {
			if(acBuffer[i] == 0) {
				break;
			}
			if(acBuffer[i] == '\\') {
				acBuffer[i] = '/';
			}
		}

		return acBuffer;
	}

	std::string AppPath::GetAppPath() {
		std::string strAppFullName = GetAppFilename();
		size_t sPos = strAppFullName.find_last_of("/");
		if(sPos == std::string::npos) {
			return "";
		}

		return std::string(strAppFullName.begin(), strAppFullName.begin() + sPos);
	}

	std::string AppPath::GetAppName() {
		std::string strAppFullName = GetAppFilename();
		size_t sPos = strAppFullName.find_last_of("/");
		if(sPos == std::string::npos) {
			return "";
		}

		return std::string(strAppFullName.begin() + sPos + 1, strAppFullName.end());
	}


	PidFile::PidFile() {
		m_hHandle = (asl_pidfile_ctx_t*)malloc(sizeof(asl_pidfile_ctx_t));
#ifdef WINDOWS
		m_hHandle->ctx = INVALID_HANDLE_VALUE;
#else
		m_hHandle->ctx = -1;
#endif
	}

	PidFile::~PidFile() {
#ifdef WINDOWS
		if(m_hHandle->ctx != INVALID_HANDLE_VALUE) {
			m_hHandle->ctx = INVALID_HANDLE_VALUE;
		}
#else
		if(m_hHandle->ctx != -1) {
			::close(m_hHandle->ctx);
			m_hHandle->ctx = -1;
		}
#endif
		free(m_hHandle);
	}

	bool PidFile::Acquire(const char* filename) {
#ifdef WINDOWS
		//TODO
		return true;
#else
		std::string pid_file = filename;

		// -rw-r--r--
		// 644
		int mode = S_IRUSR | S_IWUSR |  S_IRGRP | S_IROTH;
		int fd;
		// open pid file
		if((fd = ::open(pid_file.c_str(), O_WRONLY | O_CREAT, mode)) < 0) {
			return false;
		}

		// require write lock
		struct flock lock;
		lock.l_type = F_WRLCK; // F_RDLCK, F_WRLCK, F_UNLCK
		lock.l_start = 0; // type offset, relative to l_whence
		lock.l_whence = SEEK_SET;  // SEEK_SET, SEEK_CUR, SEEK_END
		lock.l_len = 0;
		if(fcntl(fd, F_SETLK, &lock) < 0) {
			return false;
		}

		// truncate file
		if(ftruncate(fd, 0) < 0) {
			return false;
		}

		int pid = (int)getpid();
		// write the pid
		char buf[512];
		snprintf(buf, sizeof(buf), "%d", pid);
		if(write(fd, buf, strlen(buf)) != (int)strlen(buf)) {
			return false;
		}

		// auto close when fork child process.
		int val;
		if((val = fcntl(fd, F_GETFD, 0)) < 0) {
			return false;
		}
		val |= FD_CLOEXEC;
		if(fcntl(fd, F_SETFD, val) < 0) {
			return false;
		}

		m_hHandle->ctx = fd;

		return true;
#endif
	}
}
