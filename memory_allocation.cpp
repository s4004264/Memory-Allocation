#include <iostream>
#include <list>
#include <cstdlib>
#include <unistd.h>

struct allocation{ 
std::size_t size; 
void *space; 
};

std::list<allocation*> occupiedChunks;
std::list<allocation*> freeChunks;
std::string strategy;

allocation * firstFit(std::size_t chunkSize){
    allocation * nullChunk = nullptr;
    for (allocation * chunk : freeChunks){
        std::cout << chunk;
        if (chunk->size >= chunkSize){
            return chunk;
        }
    }
    return nullChunk;
}

allocation * bestFit(std::size_t chunkSize){
    std::size_t sizeDiff = SIZE_MAX;
    allocation * bestChunk = nullptr;
    for (allocation * chunk : freeChunks){
        if ((chunk->size >= chunkSize) && (chunk->size - chunkSize) < sizeDiff){
            sizeDiff = chunk->size - chunkSize;
            bestChunk = chunk;
        }
    }
    return bestChunk;
}

void * alloc(std::size_t chunkSize){
    std::cout << chunkSize;
    if(chunkSize < 32){
        chunkSize = 32;
    }else if(chunkSize < 64){
        chunkSize = 64;
    }else if(chunkSize < 128){
        chunkSize = 128;
    }else if(chunkSize < 256){
        chunkSize = 256;
    }else{
        chunkSize = 512;
    }
    std::cout << chunkSize;
    allocation * chunk;
    if(strategy == "firstfit"){
        chunk = firstFit(chunkSize);
    }else if(strategy == "bestfit"){ 
        chunk = bestFit(chunkSize);
    }else{
        std::cerr << "strategy value not valid" << std::endl;
        return nullptr;
    }

    if(chunk){
        freeChunks.remove(chunk);
        occupiedChunks.push_back(chunk);
        return chunk->space;
    }


    void * memAlloc = sbrk(chunkSize);
    if (memAlloc == (void*)-1){
        std::cerr << "failed to allocate " << chunkSize << " bytes of memory" << std::endl;
        return nullptr;
    }

    allocation newChunk;
    newChunk.size = chunkSize;
    newChunk.space = memAlloc;
    occupiedChunks.push_back(&newChunk);
    return newChunk.space;
};

void dealloc(void * chunk){

};




int main(int argc, char* argv[]){
    if(argc != 3){
        std::cerr << "Please enter arguments in the form ./memory_allocation <strategy> <datafile>" << std::endl;
        return 1;
    }

    strategy = argv[1];
    if((strategy != "firstfit") && (strategy != "bestfit")){
        std::cout << "you entered " << strategy << std::endl;
        std::cerr << "please enter only 'firstfit' or 'bestfit' as the strategy" << std::endl;
        return 1;
    }
    std::cout << strategy << std::endl;
    void * num1 = alloc(32);
    void * num2 = alloc(64);
    void * num3 = alloc(128);
    void * num4 = alloc(256);
    void * num5 = alloc(512);

    return 0;
}