/**
 * @file file.hpp
 * @brief 文件系统
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include <string>
#include <stdint.h>
#include <time.h>

namespace ASL_NAMESPACE {
	struct asl_file_ctx_t;
	struct asl_filefinder_ctx_t;
	struct asl_pidfile_ctx_t;

	/**
	 * @class File
	 * @brief 文件类
	 */
	class File : public NoCopyable {
	public:
		File();
		File(const char* filename, int flags);
		~File();

		/**
		 * @brief 文件打开选项
		 */
		enum OpenFlags {
			OF_In		= 0x00, ///< 以只读方式打开
			OF_Out		= 0x01, ///< 以只写方式打开
			OF_App		= 0x02, ///< 以追加方式打开
			OF_NoCreate	= 0x04, ///< 当文件不存在时不创建
		};

		/**
		 * @brief 跳转基地址
		 */
		enum SeekBase {
			SB_Begin,	///< 文件开始
			SB_Cur,		///< 当前位置
			SB_End,		///< 文件结尾
		};

	public:
		/**
		 * @brief 打开文件
		 * @param filename 文件名
		 * @param flags 选项
		 * @return 返回是否打开成功
		 */
		bool Open(const char* filename, int flags);

		/**
		 * @brief 关闭文件
		 */
		void Close();

		/**
		 * @brief 写入数据
		 * @param buf 写入缓存
		 * @param size 写入大小
		 * @return 成功返回写入大小，失败返回负数
		 */
		int Write(const char* buf, int size);

		/**
		 * @brief 读取数据
		 * @param buf 读取缓存
		 * @param size 读取大小
		 * @return 成功返回读取大小，失败返回负数
		 */
		int Read(char* buf, int size);

		/**
		 * @brief 获取当前位置
		 * @return 返回当前位置
		 */
		size_t GetPos();

		/**
		 * @brief 跳转当前位置
		 * @param pos 当前位置
		 * @param base 当前位置基地址
		 */
		void Seek(size_t pos, SeekBase base = SB_Begin);

		/**
		 * @brief 获取是否可写
		 * @return 返回是否可写
		 */
		bool IsWriteMode();

		/**
		 * @brief 获取文件大小
		 * @return 返回文件大小
		 */
		size_t GetFileSize();

		/**
		 * @brief 重设文件大小
		 * @param nSize 文件大小
		 * @return 返回执行结果
		 */
		bool ResetFileSize(size_t nSize);

		/**
		 * @brief 获取最后读取时间
		 * @return 返回最后读取时间
		 */
		time_t GetLastAccessTime();

		/**
		 * @brief 获取最后写入时间
		 * @return 返回最后写入时间
		 */
		time_t GetLastWriteTime();

	public:
		/**
		 * @brief 检测文件是否存在
		 * @param path 文件路径
		 * @return 返回文件状态
		 */
		static bool Access(const char* path);

		/**
		 * @brief 移除文件
		 * @param filename 文件名
		 * @return 返回操作结果
		 */
		static bool RemoveFile(const char* filename);

	private:
		int m_nOpenFlags;			///< 打开选项
		size_t m_nCurPos;			///< 当前读写位置
		std::string m_strFilename;	///< 文件名
		asl_file_ctx_t* m_hHandle;	///< 文件句柄
	};

	/**
	 * @class Directory
	 * @brief 目录类
	 */
	class Directory {
	public:
		/**
		 * @brief 创建目录
		 * @param path 目录名
		 * @return 返回操作结果
		 */
		static bool CreateDir(const char* path);

		/**
		 * @brief 删除目录及子节点
		 * @param path 目录名
		 * @param recursive 是否递归创建上级目录
		 * @return 返回操作结果
		 */
		static bool DeleteDir(const char* path, bool recursive = false);

		/**
		 * @brief 循环创建文件所在目录
		 * @param filename 文件名
		 * @return 返回操作结果
		 */
		static bool MakeFilePath(const char* filename);

		/**
		 * @brief 循环创建目录
		 * @param path 目录名
		 * @return 返回操作结果
		 */
		static bool MakeDirPath(const char* path);

	private:
		/**
		 * @brief 删除目录
		 * @param path 目录名
		 * @return 返回操作结果
		 */
		static bool _DeleteDir(const char* path);
	};

	/**
	 * @class PathFinder
	 * @brief 路径检索类
	 */
	class PathFinder : public NoCopyable {
	public:
		PathFinder();
		~PathFinder();

		/**
		 * @struct FileInfo_t
		 * @brief 文件信息
		 */
		struct FileInfo_t {
			std::string strFilename;	///< 文件名
			bool bIsDir;				///< 是否为目录
			bool bNormalFile;			///< 是否为普通文件
			size_t nFileSize;		///< 文件大小
			time_t nLastAccessTime; ///< 最后读取时间
			time_t nLastWriteTime;	///< 最后写入时间
		};

	public:
		/**
		 * @brief 获取第一个子节点
		 * @param path 目录路径
		 * @return 返回操作结果
		 */
		bool GetFirstChild(const char* path);

		/**
		 * @brief 获取下一个子节点
		 * @return 返回操作结果
		 */
		bool GetNextChild();

		/**
		 * @brief 关闭检索句柄
		 */
		void Close();

		/**
		 * @brief 获取当前节点信息
		 * @return 返回当前节点信息
		 */
		const FileInfo_t& CurChild() const {
			return m_CurChild;
		}

	private:
		/**
		 * @brief 获取载入当前节点信息
		 */
		void _LoadFileInfo();

	private:
		std::string m_PathName;		///< 当前目录路径
		FileInfo_t m_CurChild;		///< 当前节点信息
		asl_filefinder_ctx_t* m_hHandle; ///< 查找句柄
	};

	/**
	 * @class AppPath
	 * @brief 程序路径
	 */
	class AppPath {
	public:
		/**
		 * @brief 获取程序文件路径名
		 * @return 返回程序文件路径名
		 */
		static std::string GetAppFilename();

		/**
		 * @brief 获取程序目录
		 * @return 返回程序目录
		 */
		static std::string GetAppPath();

		/**
		 * @brief 获取程序文件名
		 * @return 返回程序文件名
		 */
		static std::string GetAppName();
	};

	/**
	 * @class PidFile
	 * @brief 进程文件
	 */
	class PidFile {
	public:
		PidFile();
		~PidFile();

	public:
		/**
		 * @brief 捕获进程文件
		 * @param filename 进程文件名
		 * @return 返回执行结果
		 */
		bool Acquire(const char* filename);

	private:
		asl_pidfile_ctx_t* m_hHandle;
	};
}
