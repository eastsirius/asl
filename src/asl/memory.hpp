﻿/**
 * @file memory.hpp
 * @brief 内存工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include <cassert>

namespace ASL_NAMESPACE {
	/**
     * @brief 数据缓冲
     */
	class Buffer : public NoCopyable {
	public:
		Buffer();
		Buffer(size_t nSize);
		virtual ~Buffer();

	public:
		/**
		 * @brief 创建数据缓冲
		 * @param nSize 缓冲大小
		 * @return 返回执行结果
		 */
		bool Create(size_t nSize);

		/**
		 * @brief 重设数据缓冲
		 * @param nNewSize 缓冲新大小
		 * @return 返回执行结果
		 */
		bool Recreate(size_t nNewSize);

		/**
		 * @brief 释放数据缓冲
		 */
		void Release();

		/**
		 * @brief 追加数据
		 * @param nSize 数据大小
		 * @return 返回执行结果
		 */
		bool AppendData(size_t nSize);

		/**
		 * @brief 追加数据
		 * @param pData 数据缓存
		 * @param nSize 数据大小
		 * @return 返回执行结果
		 */
		bool AppendData(const uint8_t* pData, size_t nSize);

		/**
		 * @brief 跳过数据
		 * @param nSize 跳过大小
		 */
		void SkipData(size_t nSize);

		/**
		 * @brief 获取数据大小
		 * @return 返回数据大小
		 */
		size_t GetDataSize() const {
			return m_nDataSize;
		}

		/**
		 * @brief 获取缓冲大小
		 * @return 返回缓冲大小
		 */
		size_t GetBufferSize() const {
			return m_nBufSize;
		}

		/**
		 * @brief 获取缓冲剩余大小
		 * @return 返回缓冲剩余大小
		 */
		size_t GetFreeSize() const {
			assert(m_nBufSize >= m_nDataSize);
			return m_nBufSize - m_nDataSize;
		}

		/**
		 * @brief 获取缓冲指针
		 * @param nOffset 偏移大小
		 * @return 返回偏移后缓冲指针
		 */
		uint8_t* GetBuffer(size_t nOffset = 0) {
			assert(m_pBuffer != NULL);
			assert(nOffset < m_nBufSize);
			return m_pBuffer + nOffset;
		}

		/**
		 * @brief 判断是否已初始化
		 * @return 返回偏是否已初始化
		 */
		bool IsInited() const {
			return m_pBuffer != NULL;
		}

		operator bool() const {
			return IsInited();
		}

	private:
		size_t m_nDataSize; ///< 数据大小
		size_t m_nBufSize;  ///< 缓存大小
		uint8_t* m_pBuffer; ///< 缓存指针
	};

	/**
     * @brief 自增长数据缓冲
     */
	class GrowthBuffer : public Buffer {
	public:
		GrowthBuffer();
		GrowthBuffer(size_t nSize);
		virtual ~GrowthBuffer();

	public:
		/**
		 * @brief 请求剩余空间
		 * @param nSize 剩余空间大小
		 * @return 返回执行结果
		 */
		bool RequestFreeSize(size_t nSize);
	};
}
