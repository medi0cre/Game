#include <Arena.h>

bool ArenaInit(Arena* _Arena_, size_t Size)
{
    if (!_Arena_)
    {
        fprintf(stderr, "Null arena cannot be initialized\n");
        return false;
    }

    if (Size > ArenaMax)
    {
        fprintf(stderr, "Requested arena size is too big\n");
        return false;
    }

    _Arena_->Start = (unsigned char*)calloc(1, Size);
    if (!_Arena_->Start)
    {
        fprintf(stderr, "Failed to initialize arena\n");
        return false;
    }

    _Arena_->Current = _Arena_->Start;
    _Arena_->Snapshot = _Arena_->Start;
    _Arena_->Size = Size;
    return true;
}

void* ArenaAlloc(Arena* _Arena_, size_t Size, size_t Alignment)
{
    if (!_Arena_ || !_Arena_->Start)
    {
        fprintf(stderr, "Null arena\n");
        return NULL;
    }

    if (Alignment == 0 || (Alignment & (Alignment - 1)) != 0)
    {
        fprintf(stderr, "Alignment must be a non-zero power of two\n");
        return NULL;
    }

    if (Size > _Arena_->Size)
    {
        fprintf(stderr, "Requested allocation was too big\n");
        return NULL;
    }

    unsigned char* AlignedMemory = (unsigned char*) (((uint64_t)_Arena_->Current + Alignment - 1) & ~(Alignment - 1));
    if (AlignedMemory > _Arena_->Start + _Arena_->Size - Size)
    {
        fprintf(stderr, "Arena out of memory\n");
        return NULL;
    }

    _Arena_->Current = AlignedMemory + Size;
    return AlignedMemory;
}

bool ArenaSnapshot(Arena* _Arena_)
{
    if (!_Arena_ || !_Arena_->Start)
    {
        fprintf(stderr, "Null arena\n");
        return false;
    }

    _Arena_->Snapshot = _Arena_->Current;
    return true;
}

bool ArenaReset(Arena* _Arena_)
{
    if (!_Arena_ || !_Arena_->Start)
    {
        fprintf(stderr, "Null arena\n");
        return false;
    }

    _Arena_->Current = _Arena_->Start;
    return true;
}

bool ArenaResetToSnapshot(Arena* _Arena_)
{
    if (!_Arena_ || !_Arena_->Start)
    {
        fprintf(stderr, "Null arena\n");
        return false;
    }

    _Arena_->Current = _Arena_->Snapshot;
    return true;
}

bool ArenaFree(Arena* _Arena_)
{
    if (!_Arena_ || !_Arena_->Start)
    {
        fprintf(stderr, "Null arena\n");
        return false;
    }

    free(_Arena_->Start);
    _Arena_->Start = NULL;
    _Arena_->Current = NULL;
    _Arena_->Snapshot = NULL;
    _Arena_->Size = 0;
    return true;
}
