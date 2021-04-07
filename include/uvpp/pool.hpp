#ifndef _UVPP_POOL_H__
#define _UVPP_POOL_H__

#include <stack>
#include <atomic>

namespace uvpp
{

template <int N, class Tag>
class Pool
{
	enum { HEADSIZE = sizeof(size_t) };
public:
	static char* malloc(size_t size, size_t* allsize = 0)
	{
		char* ret = nullptr;

		int n = calcN(size + HEADSIZE);
		if (n < N)
		{
			ret = popSta(n);

			if(!ret)
			{
				size_t asize = Pool::calcSize(n);
				assert(size <= asize);
				ret = Pool::alloc(asize);
				if (allsize)
				{
					*allsize = asize - HEADSIZE;
				}
			}
		}
		else
		{
			ret = Pool::alloc(size + HEADSIZE);
			if (allsize)
			{
				*allsize = size;
			}
		}

		return ret;
	}

	static void free(char* p)
	{
		char* src = p - HEADSIZE;
		int n = Pool::calcN(*reinterpret_cast<size_t*>(src));
		if (n < N)
		{
			pushSta(n, p);
		}
		else
		{
			delete[] src;
		}
	}

	static void release()
	{
		for (int ix = 0; ix != N; ++ix)
		{
			clearSta(ix);
		}
	}

protected:
	static char* alloc(const size_t& size)
	{
		char* p = new char[size];

		*reinterpret_cast<size_t*>(p) = size;
		
		return p + HEADSIZE;
	}

	static inline int calcN(const size_t& size)
	{
		int n = 0;
		size_t tmp = size >> 10;
		if (tmp)
		{
			n += (size & 1023) ? 1 : 0;

			for (size_t ix = 1; ix < tmp; ix = ix << 1)
			{
				++n;
			}
		}
		return n;
	}

	static inline size_t calcSize(int n)
	{
		size_t asize = 1 << n << 10;
		return asize;
	}

	static inline void pushSta(int n, char* p)
	{
		std::stack<char *>& pool = m_pools[n];
		std::atomic_flag& amf = m_amf[n];

		while (amf.test_and_set());

		pool.push(p);

		amf.clear();
	}

	static inline char* popSta(int n)
	{
		char* ret = nullptr;

		std::stack<char *>& pool = m_pools[n];
		std::atomic_flag& amf = m_amf[n];

		while (amf.test_and_set());

		if (!pool.empty())
		{
			ret = pool.top();
			pool.pop();
		}

		amf.clear();

		return ret;
	}

	static inline void clearSta(int n)
	{
		std::stack<char *>& pool = m_pools[n];
		std::atomic_flag& amf = m_amf[n];

		while (amf.test_and_set());

		while (pool.empty())
		{
			delete[](pool.top() - HEADSIZE);
			pool.pop();
		}

		amf.clear();
	}

private:
	static std::atomic_flag m_amf[N];
	static std::stack<char*> m_pools[N];
};

template <int N, class Tag>
std::atomic_flag uvpp::Pool<N, Tag>::m_amf[N] = { ATOMIC_FLAG_INIT };

template <int N, class Tag>
std::stack<char*> uvpp::Pool<N, Tag>::m_pools[N];

typedef Pool<10, class Tag> DPool;
}

#endif // POOL_H__
