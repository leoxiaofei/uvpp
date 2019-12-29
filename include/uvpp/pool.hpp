#ifndef _UVPP_POOL_H__
#define _UVPP_POOL_H__

#include <stack>

namespace uvpp
{

template <int N, class Tag>
class Pool
{
	enum { HEADSIZE = sizeof(size_t) };
public:
	static char* malloc(size_t size, size_t* allsize = 0)
	{
		char* ret = 0;

		int n = calcN(size);
		if (n < N)
		{
			std::stack<char *>& pool = m_pools[n];
			if (!pool.empty())
			{
				ret = pool.top();
				pool.pop();
			}
			else
			{
				size_t asize = clacSize(n);
				assert(size <= asize);
				ret = Pool::alloc(asize);
				if (allsize)
				{
					*allsize = asize;
				}
			}
		}
		else
		{
			ret = Pool::alloc(size);
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
		int n = calcN(*reinterpret_cast<size_t*>(src));
		if (n < N)
		{
			std::stack<char *>& pool = m_pools[n];
			pool.push(p);
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
			std::stack<char *>& pool = m_pools[ix];
			while (pool.empty())
			{
				delete[] (pool.top() - HEADSIZE);
				pool.pop();
			}
		}
	}

protected:
	static char* alloc(const size_t& size)
	{
		char* p = new char[size + HEADSIZE];

		*reinterpret_cast<size_t*>(p) = size;
		
		return p + HEADSIZE;
	}

	static inline int calcN(const size_t& size)
	{
		int n = 0;
		int tmp = size >> 10;
		for (int ix = 1; ; ix = ix << 1)
		{
			if (ix >= tmp)
			{
				break;
			}
			++n;
		}
		return n;
	}

	static inline size_t clacSize(int n)
	{
		size_t asize = 1 << n << 10;
		return asize;
	}

private:
	static std::stack<char*> m_pools[N];
};

template <int N, class Tag>
std::stack<char*> uvpp::Pool<N, Tag>::m_pools[N];

typedef Pool<10, class Tag> DPool;
}

#endif // POOL_H__
