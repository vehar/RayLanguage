#include <RThreadPool.h>

#ifndef RAY_EMBEDDED

#include <RClassTable.h>

struct RThreadPool {
    size_t classId;

    REnumerateDelegate enumerator;
    RThreadFunction    delegateFunction;
    RArray            *threads;
};

constructor(RThreadPool)) {
    object = allocator(RThreadPool);
    if(object != nil) {
        object->threads = makeRArray();
        if(object->threads != nil) {
            object->threads->destructorDelegate = getRFree();
            object->classId = registerClassOnce(toString(RThreadPool));
        } else {
            deallocator(object);
            object = nil;
        }
    }
    return object;
}

destructor(RThreadPool) {
    deleter(object->threads, RArray);
}

printer(RThreadPool) {
    RPrintf("%s object %p {", toString(RThreadPool), object);
    $(object->threads, p(RArray)));
    RPrintf("} end of %s\n", toString(RThreadPool));
}

method(void, setDelegateFunction, RThreadPool), RThreadFunction delegateFunction) {
    object->delegateFunction = delegateFunction;
}

method(RThreadFunction, delegateFunction, RThreadPool)) {
    return object->delegateFunction;
}

method(void, addWithArg, RThreadPool), pointer argumentForNewWorker) {
    RThread *newOne = allocator(RThread);
    if(newOne != nil) {
//        RPrintf("new worker %p\n", newOne);
        RThreadCreate(newOne, nil, object->delegateFunction, argumentForNewWorker);
//        RPrintf("malloc ptr %p, RTrueMalloc %p, is equals = %d\n", RMallocPtr, RTrueMalloc, RMallocPtr == RTrueMalloc);
//        assert(RMallocPtr != RTrueMalloc);
        $(object->threads, m(addObject, RArray)), newOne);

    } elseError( RError("RThreadPool. Add with arg bad worker allocation.", object) );
}

method(void, addWorker,  RThreadPool), RThread *worker) {
    $(object->threads, m(addObject, RArray)), worker);
}

rbool joinThreadCheck(pointer context, pointer thread, size_t iterator) {
    RThreadJoin(thread);
    return yes;
}

method(void, join, RThreadPool)) {
    object->enumerator.virtualEnumerator = joinThreadCheck;
    $(object->threads, m(enumerate, RArray)), &object->enumerator, yes);
}

#endif