/*
	实现allocator
	1）基础内存分配器CtrAllocator须实现申请内存块Malloc（），调整内存块Realloc（），释放内存块Free（）
	2）DOM默认分配器MemoryAllocator，先使用用户缓冲区，再使用当前内存块，内存块满后申请新内存
    功能：
        基础功能：Malloc（）；Realloc（）；Free（）
        扩展功能： Clear()；Capacity()；Size() 
*/

#include <iostream>
#include <assert.h>

using namespace std;

#ifndef ALLOCATOR_DEFAULT_CHUNK_CAPACITY
#define ALLOCATOR_DEFAULT_CHUNK_CAPACITY (64 * 1024)
#endif

//有些机器需要严格的数据对齐。默认值为8字节。
//把x转化为大于等于x的最小的8的倍数
#ifndef ZJSON_ALIGN
#define ZJSON_ALIGN(x) (((x) + static_cast<size_t>(7u)) & ~static_cast<size_t>(7u))
#endif

class CrtAllocator {
public:
	static const bool kNeedFree = true;
	void* Malloc(size_t size) {
		if (size) {
			return std::malloc(size);//大小不为0，申请空间
		}
		else
			return NULL;
	}

	void* Realloc(void* oPtr, size_t osize, size_t newsize)
	{
		//(void)osize;
		if (newsize==0)
		{
			free(oPtr);
			return NULL;
		}
		return std::realloc(oPtr, newsize);
	}

	void Free(void* ptr)
	{
		 std::free(ptr);
	}

};

template <typename BaseAllocator = CrtAllocator>
class MemoryAllocator {
public:
	static const bool kNeedFree = false;
	
    //具有内存块的构造函数
	MemoryAllocator(size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) :
		chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(0), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
	{
	}
    //具有用户缓冲区的构造函数，首先使用用户缓冲区，且用户缓冲区使用完后不会释放。
	MemoryAllocator(void* buffer, size_t size, size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) :
		chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(buffer), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
	{
		assert(buffer != 0);
		assert(size > sizeof(ChunkHeader));
		chunkHead_ = reinterpret_cast<ChunkHeader*>(buffer);
		chunkHead_->capacity = size - sizeof(ChunkHeader);
		chunkHead_->size = 0;
		chunkHead_->next = 0;
	}
	
    ~MemoryAllocator() {
        Clear();
        delete(ownBaseAllocator_);
    }

    void Clear() {
        while (chunkHead_ && chunkHead_ != userBuffer_) {
            ChunkHeader* next = chunkHead_->next;
            baseAllocator_->Free(chunkHead_);
            chunkHead_ = next;
        }
        if (chunkHead_ && chunkHead_ == userBuffer_)
            chunkHead_->size = 0; 
    }

    // 计算分配的内存块的总容量,返回总容量（字节）。
    size_t Capacity()  {
        size_t capacity = 0;
        for (ChunkHeader* c = chunkHead_; c ; c = c->next)
            capacity += c->capacity;
        return capacity;
    }

   //计算分配的内存块,返回已用字节总数。
    size_t Size() const {
        size_t size = 0;
        for (ChunkHeader* c = chunkHead_; c != 0; c = c->next)
            size += c->size;
        return size;
    }

    void* Malloc(size_t size) {
        if (!size)
            return NULL;

        size = ZJSON_ALIGN(size);
        if (chunkHead_ == 0 || chunkHead_->size + size > chunkHead_->capacity)
            if (!AddChunk(chunk_capacity_ > size ? chunk_capacity_ : size))
                return NULL;

        void* buffer = reinterpret_cast<char*>(chunkHead_) + ZJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size;
        chunkHead_->size += size;
        return buffer;
    }

    void* Realloc(void* oPtr, size_t oSize, size_t newSize) {
        if (oPtr == 0)
            return Malloc(newSize);

        if (newSize == 0)
            return NULL;

        oSize = ZJSON_ALIGN(originalSize);
        newSize = ZJSON_ALIGN(newSize);

        // 如果新尺寸小于原尺寸，请勿收缩
        if (oSize >= newSize)
            return oPtr;

        //如果它是最后一个分配并且有足够的空间，只需扩展它
        if (oPtr == reinterpret_cast<char*>(chunkHead_) + ZJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size - oSize) {
            size_t increment = static_cast<size_t>(newSize - oSize);
            if (chunkHead_->size + increment <= chunkHead_->capacity) {
                chunkHead_->size += increment;
                return oPtr;
            }
        }

        // 重新分配进程：分配和复制内存，不释放原始缓冲区。
        if (void* newBuffer = Malloc(newSize)) {
            if (oSize)
                std::memcpy(newBuffer, oPtr, oSize);
            return newBuffer;
        }
        else
            return NULL;
    }

    //释放内存块
    static void Free(void* ptr) { (void)ptr; } 


private:

    MemoryAllocator(const MemoryAllocator& rhs) 
    
    MemoryAllocator& operator=(const MemoryAllocator& rhs)

     //创建新内存块
    bool AddChunk(size_t capacity) {
        if (!baseAllocator_)
            ownBaseAllocator_ = baseAllocator_ = new(BaseAllocator)();
        if (ChunkHeader* chunk = reinterpret_cast<ChunkHeader*>(baseAllocator_->Malloc(ZJSON_ALIGN(sizeof(ChunkHeader)) + capacity))) {
            chunk->capacity = capacity;
            chunk->size = 0;
            chunk->next = chunkHead_;
            chunkHead_ = chunk;
            return true;
        }
        else
            return false;
    }

    //默认容量为（64*1024）
	static const int kDefaultChunkCapacity = ALLOCATOR_DEFAULT_CHUNK_CAPACITY;

	struct ChunkHeader {
		size_t capacity;    //!< 块的容量（字节）（不包括头本身）。
		size_t size;        //!< 当前分配的内存大小（字节）。
		ChunkHeader* next;  //!< 链接列表中的下一个块。
	};

	ChunkHeader* chunkHead_;    //!< 块链接列表的头。只有头块服务于分配。
	size_t chunk_capacity_;     //!< 分配块时块的最小容量。
	void* userBuffer_;          //!< 用户提供的缓冲区.
	BaseAllocator* baseAllocator_;  //!< 用于分配内存块的基本分配器。
	BaseAllocator* ownBaseAllocator_;
};