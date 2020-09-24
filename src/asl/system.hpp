/**
 * @file system.hpp
 * @brief 系统工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include <vector>
#include <string>
#include <memory>
#include <exception>

namespace ASL_NAMESPACE {
	/**
	 * @brief 错误描述
	 */
	class ErrorDesc : public NoCopyable {
	public:
		virtual ~ErrorDesc(){}

		/**
		 * @brief 获取错误类型
		 * @return 返回错误类型
		 */
		virtual const char* GetType() const = 0;

		/**
		 * @brief 获取错误信息
		 * @param ec 错误码
		 * @return 返回错误信息
		 */
		virtual const char* GetMsg(int ec) const = 0;
	};
	typedef std::shared_ptr<ErrorDesc> ErrorDescPtr_t;

	/**
	 * @brief 自定义错误描述
	 */
	class CustomErrorDesc : public ErrorDesc {
	public:
		CustomErrorDesc(const std::string& msg);
		virtual ~CustomErrorDesc();

	public:
		/**
		 * @brief 获取错误类型
		 * @return 返回错误类型
		 */
		virtual const char* GetType() const {
			return "custom";
		}

		/**
		 * @brief 获取错误信息
		 * @param ec 错误码
		 * @return 返回错误信息
		 */
		virtual const char* GetMsg(int ec) const;

	private:
		const std::string m_strMessage;
	};

	/**
	 * @brief 系统错误描述
	 */
	class SystemErrorDesc : public ErrorDesc {
	public:
		SystemErrorDesc();
		virtual ~SystemErrorDesc();

	public:
		/**
		 * @brief 获取错误类型
		 * @return 返回错误类型
		 */
		virtual const char* GetType() const {
			return "system";
		}

		/**
		 * @brief 获取错误信息
		 * @param ec 错误码
		 * @return 返回错误信息
		 */
		virtual const char* GetMsg(int ec) const;
	};

	/**
	 * @brief ASL错误值
	 */
	enum AslErrorCodeValue_t {
		AECV_NoError	= 0,
		AECV_Error,
		AECV_AllocContextError,
		AECV_OpTimeout,
		AECV_Reconnect,
		AECV_ConnectFailed,
		AECV_BindSocketError,
		AECV_ParamError,
		AECV_AllocMemoryFailed,
		AECV_SerialFailed,
		AECV_ParseFailed,
		AECV_SendFailed,
		AECV_RecvFailed,
	};

	/**
	 * @brief ASL错误描述
	 */
	class AslErrorDesc : public ErrorDesc {
	public:
		AslErrorDesc();
		virtual ~AslErrorDesc();

	public:
		/**
		 * @brief 获取错误类型
		 * @return 返回错误类型
		 */
		virtual const char* GetType() const {
			return "asl";
		}

		/**
		 * @brief 获取错误信息
		 * @param ec 错误码
		 * @return 返回错误信息
		 */
		virtual const char* GetMsg(int ec) const;
	};

	/**
	 * @class ErrorCode
	 * @brief 错误代码
	 */
	class ErrorCode {
	public:
		ErrorCode();
		ErrorCode(const ErrorCode& ec);
		ErrorCode(int ec, ErrorDesc* desc);

	public:
		/**
		 * @brief 获取错误类型
		 * @return 返回错误类型
		 */
		const char* GetType() const;

		/**
		 * @brief 获取数字代码
		 * @return 返回数字代码
		 */
		int GetCode() const;

		/**
		 * @brief 获取错误消息
		 * @return 返回错误消息
		 */
		const char* GetMsg() const;

		/**
		 * @brief 获取最后一次系统错误码
		 * @return 返回错误码
		 */
		static ErrorCode GetLastSystemError();

		operator bool() const {
			return m_nCode != 0;
		}

	private:
		int m_nCode;
		ErrorDescPtr_t m_pDesc;
	};

	static inline ErrorCode SystemError(int ec) {
		return ErrorCode(ec, new SystemErrorDesc());
	}

	static inline ErrorCode AslError(AslErrorCodeValue_t ec) {
		return ErrorCode(ec, new AslErrorDesc());
	}

	/**
	 * @class Error
	 * @brief 错误
	 */
	class Error : public std::exception {
	public:
		Error(ErrorCode ec);
		virtual ~Error() throw();

	public:
		/**
		 * @brief 返回异常描述
		 * @return 返回异常描述
		 */
		const char* what() const throw();

		/**
		 * @brief 返回错误代码
		 * @return 返回错误代码
		 */
		const ErrorCode& GetCode() const throw();

	private:
		ErrorCode m_ecErrorCode; ///< 系统错误代码
	};

	/**
	 * @class System
	 * @brief 系统工具
	 */
	class System {
	public:
		/**
		 * @brief 获取内核数
		 * @return 返回内核数
		 */
		static int GetNumberOfProcessors();
	};

	struct asl_dlloader_ctx_t;

	/**
	 * @class DlLoader
	 * @brief DLL载入器
	 */
	class DlLoader : public NoCopyable {
	public:
		DlLoader();
		~DlLoader();

	public:
		/**
		 * @brief 载入DLL
		 * @param filename DLL文件名
		 * @return 返回执行结果
		 */
		bool Load(const char* filename);

		/**
		 * @brief 释放DLL
		 */
		void Release();

		/**
		 * @brief 获取函数指针
		 * @param proc_name 函数名
		 * @return 成功返回函数指针，失败返回NULL
		 */
		void* GetProc(const char* proc_name);

	private:
		asl_dlloader_ctx_t* m_hHandle; ///< DLL句柄
	};

	/**
	 * @class DllProcList
	 * @brief DLL接口函数表
	 */
	class DlProcList : public DlLoader {
	public:
		DlProcList();
		~DlProcList();

	public:
		/**
		 * @brief 载入DLL
		 * @param filename DLL文件名
		 * @return 返回执行结果
		 */
		bool Load(const char* filename);

		/**
		 * @brief 载入DLL
		 * @param filename DLL文件名
		 * @param proc_names 函数名列表
		 * @param proc_names_num 函数数
		 * @return 返回执行结果
		 */
		bool Load(const char* filename, const char** proc_names, int proc_names_num);

		/**
		 * @brief 释放DLL
		 */
		void Release();

		/**
		 * @brief 添加函数指针
		 * @param proc_name 函数名
		 * @return 成功返回函数指针索引，失败返回-1
		 */
		int AddProc(const char* proc_name);

		void* operator[](size_t index) const;

	private:
		std::vector<void*> m_lstDllProcs; ///< 函数指针列表
	};
}
