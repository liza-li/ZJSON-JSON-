/*
	ʵ��allocator
	1�������ڴ������CtrAllocator��ʵ�������ڴ��Malloc�����������ڴ��Realloc�������ͷ��ڴ��Free����
	2��DOMĬ�Ϸ�����MemoryAllocator����ʹ���û�����������ʹ�õ�ǰ�ڴ�飬�ڴ�������������ڴ�
    ���ܣ�
        �������ܣ�Malloc������Realloc������Free����
        ��չ���ܣ� Clear()��Capacity()��Size() 
*/

#include <iostream>
#include <assert.h>

using namespace std;

#ifndef ALLOCATOR_DEFAULT_CHUNK_CAPACITY
#define ALLOCATOR_DEFAULT_CHUNK_CAPACITY (64 * 1024)
#endif

//��Щ������Ҫ�ϸ�����ݶ��롣Ĭ��ֵΪ8�ֽڡ�
//��xת��Ϊ���ڵ���x����С��8�ı���
#ifndef ZJSON_ALIGN
#define ZJSON_ALIGN(x) (((x) + static_cast<size_t>(7u)) & ~static_cast<size_t>(7u))
#endif

class CrtAllocator {
public:
	static const bool kNeedFree = true;
	void* Malloc(size_t size) {
		if (size) {
			return std::malloc(size);//��С��Ϊ0������ռ�
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
	
    //�����ڴ��Ĺ��캯��
	MemoryAllocator(size_t chunkSize = kDefaultChunkCapacity, BaseAllocator* baseAllocator = 0) :
		chunkHead_(0), chunk_capacity_(chunkSize), userBuffer_(0), baseAllocator_(baseAllocator), ownBaseAllocator_(0)
	{
	}
    //�����û��������Ĺ��캯��������ʹ���û������������û�������ʹ����󲻻��ͷš�
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

    // ���������ڴ���������,�������������ֽڣ���
    size_t Capacity()  {
        size_t capacity = 0;
        for (ChunkHeader* c = chunkHead_; c ; c = c->next)
            capacity += c->capacity;
        return capacity;
    }

   //���������ڴ��,���������ֽ�������
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

        // ����³ߴ�С��ԭ�ߴ磬��������
        if (oSize >= newSize)
            return oPtr;

        //����������һ�����䲢�����㹻�Ŀռ䣬ֻ����չ��
        if (oPtr == reinterpret_cast<char*>(chunkHead_) + ZJSON_ALIGN(sizeof(ChunkHeader)) + chunkHead_->size - oSize) {
            size_t increment = static_cast<size_t>(newSize - oSize);
            if (chunkHead_->size + increment <= chunkHead_->capacity) {
                chunkHead_->size += increment;
                return oPtr;
            }
        }

        // ���·�����̣�����͸����ڴ棬���ͷ�ԭʼ��������
        if (void* newBuffer = Malloc(newSize)) {
            if (oSize)
                std::memcpy(newBuffer, oPtr, oSize);
            return newBuffer;
        }
        else
            return NULL;
    }

    //�ͷ��ڴ��
    static void Free(void* ptr) { (void)ptr; } 


private:

    MemoryAllocator(const MemoryAllocator& rhs) 
    
    MemoryAllocator& operator=(const MemoryAllocator& rhs)

     //�������ڴ��
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

    //Ĭ������Ϊ��64*1024��
	static const int kDefaultChunkCapacity = ALLOCATOR_DEFAULT_CHUNK_CAPACITY;

	struct ChunkHeader {
		size_t capacity;    //!< ����������ֽڣ���������ͷ������
		size_t size;        //!< ��ǰ������ڴ��С���ֽڣ���
		ChunkHeader* next;  //!< �����б��е���һ���顣
	};

	ChunkHeader* chunkHead_;    //!< �������б��ͷ��ֻ��ͷ������ڷ��䡣
	size_t chunk_capacity_;     //!< �����ʱ�����С������
	void* userBuffer_;          //!< �û��ṩ�Ļ�����.
	BaseAllocator* baseAllocator_;  //!< ���ڷ����ڴ��Ļ�����������
	BaseAllocator* ownBaseAllocator_;
};