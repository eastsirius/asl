/**
 * @file memory.hpp
 * @brief 内存工具
 * @author 程行通
 */

#pragma once

#include "asldef.hpp"
#include "utils.hpp"
#include "thread.hpp"
#include "time.hpp"
#include <cassert>
#include <cstdint>
#include <map>

namespace ASL_NAMESPACE {
	/**
     * @brief 数据缓冲
     */
	class Buffer : public NoCopyable {
	public:
		Buffer();
		Buffer(size_t size);
		virtual ~Buffer();

	public:
		/**
		 * @brief 创建数据缓冲
		 * @param size 缓冲大小
		 * @return 返回执行结果
		 */
		bool Create(size_t size);

		/**
		 * @brief 重设数据缓冲
		 * @param new_size 缓冲新大小
		 * @return 返回执行结果
		 */
		bool Recreate(size_t new_size);

		/**
		 * @brief 释放数据缓冲
		 */
		void Release();

		/**
		 * @brief 追加数据
		 * @param size 数据大小
		 * @return 返回执行结果
		 */
		bool AppendData(size_t size);

		/**
		 * @brief 追加数据
		 * @param data 数据缓存
		 * @param size 数据大小
		 * @return 返回执行结果
		 */
		bool AppendData(const uint8_t* data, size_t size);

		/**
		 * @brief 跳过数据
		 * @param size 跳过大小
		 */
		void SkipData(size_t size);

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
		 * @param offset 偏移大小
		 * @return 返回偏移后缓冲指针
		 */
		uint8_t* GetBuffer(size_t offset = 0) {
			assert(m_pBuffer != NULL);
			assert(offset < m_nBufSize);
			return m_pBuffer + offset;
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
		GrowthBuffer(size_t size);
		virtual ~GrowthBuffer();

	public:
		/**
		 * @brief 请求剩余空间
		 * @param size 剩余空间大小
		 * @return 返回执行结果
		 */
		bool RequestFreeSize(size_t size);
	};

	/**
     * @brief 带超时的上下文表实现
     */
	template <class KeyT, class ValueT, class MutexT>
	class TimedSessionMapImpl : public NoCopyable {
	public:
        TimedSessionMapImpl(){}

		struct ValueSession {
		    ValueT value;
		    Timer timer;
		    int timeout;
		};

	public:
        /**
         * @brief 放入项
         * @param key 键值
         * @param value 项值
         * @param timeout 毫秒超时时间
         */
		void Put(const KeyT key, ValueT value, int timeout) {
		    AutoLocker<MutexT> lock(m_mtxLock);
            ValueSession session;
            session.value = value;
            session.timer.Restart();
            session.timeout = timeout;
		    m_mpValueMap[key] = session;
		}

        /**
         * @brief 获取项
         * @param key 键值
         * @param reset_timer 是否重置计时器
         * @return 返回项值
         */
		ValueT Get(const KeyT key, bool reset_timer = false) {
            AutoLocker<MutexT> lock(m_mtxLock);
            auto iter = m_mpValueMap.find(key);
            if(iter == m_mpValueMap.end()) {
                return ValueT();
            } else if(iter->second.timer.MillisecTime() > iter->second.timeout) {
                m_mpValueMap.erase(iter);
                return ValueT();
            } else {
                return iter->second.value;
            }
		}

        /**
         * @brief 删除项
         */
		void Delete(const KeyT key) {
            AutoLocker<MutexT> lock(m_mtxLock);
            m_mpValueMap.erase(key);
		}

        /**
         * @brief 检测项是否存在
         * @return 返回检测结果
         */
		bool Exist(const KeyT key) {
            AutoLocker<MutexT> lock(m_mtxLock);
		    auto iter = m_mpValueMap.find(key);
		    if(iter == m_mpValueMap.end()) {
		        return false;
		    } else if(iter->second.timer.MillisecTime() > iter->second.timeout) {
		        m_mpValueMap.erase(iter);
		        return false;
		    } else {
		        return true;
		    }
		}

		/**
		 * @brief 执行生命周期检查
		 */
		void WorkProc() {
            AutoLocker<MutexT> lock(m_mtxLock);
            for(auto iter = m_mpValueMap.begin(); iter != m_mpValueMap.end();) {
                if(iter->second.timer.MillisecTime() > iter->second.timeout) {
                    iter = m_mpValueMap.erase(iter);
                    continue;
                }
                ++iter;
            }
		}

	private:
	    MutexT m_mtxLock;
	    std::map<KeyT, ValueT> m_mpValueMap;
	};

	/**
     * @brief 带超时的上下文表
     */
	template <class KeyT, class ValueT>
	class TimedSessionMap : public TimedSessionMapImpl<KeyT, ValueT, NullMutex> {
	public:
        TimedSessionMap(){}
	};

	/**
     * @brief 线程安全的带超时的上下文表
     */
	template <class KeyT, class ValueT>
	class ThreadSafeTimedSessionMap : public TimedSessionMapImpl<KeyT, ValueT, Mutex> {
	public:
        ThreadSafeTimedSessionMap(){}
	};
}
