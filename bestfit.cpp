#include <iostream>
#include <list>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>
#include <exception>

struct allocation{ 
std::size_t size; 
void *space; 
};

std::list<allocation*> occupiedChunks;
std::list<allocation*> freeChunks;
std::string strategy = "bestfit";

allocation * firstFit(std::size_t chunkSize){
    allocation * nullChunk = nullptr;
    for (allocation * chunk : freeChunks){
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
    allocation * chunk;
    if(strategy == "firstfit"){
        chunk = firstFit(chunkSize);
    }else if(strategy == "bestfit"){ 
        chunk = bestFit(chunkSize);
    }else{
        std::cerr << "Error: strategy value not valid" << std::endl;
        return nullptr;
    }

    if(chunk){
        freeChunks.remove(chunk);
        occupiedChunks.push_back(chunk);
        return chunk->space;
    }


    void * memAlloc = sbrk(chunkSize);
    if (memAlloc == (void*)-1){
        std::cerr << "Error: failed to allocate " << chunkSize << " bytes of memory" << std::endl;
        return nullptr;
    }

    allocation * newChunk = new allocation();
    newChunk->size = chunkSize;
    newChunk->space = memAlloc;
    occupiedChunks.push_back(newChunk);
    return newChunk->space;
};

void dealloc(void * chunk){
    for (std::list<allocation*>::iterator search = occupiedChunks.begin(); search != occupiedChunks.end(); search++){
        allocation * element = *search;
        if(element->space == chunk){
            freeChunks.push_back(element);
            occupiedChunks.erase(search);
            return;
        }
    }
    throw std::runtime_error("No chunk found");
};




int main(int argc, char* argv[]){
    if(argc != 2){
        std::cerr << "Please enter arguments in the form ./<strategy> <datafile>" << std::endl;
        return 1;
    }


    std::ifstream datafile(argv[1]);
    if(!datafile.is_open()){
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }


    std::string line;
    void * recentMem;
    void * allocTest;
    while(std::getline(datafile, line)){
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if(command == "alloc:"){
            std::size_t size;
            iss >> size;
            allocTest = alloc(size);
            if (allocTest == nullptr){
                std::cerr << "Error: Memory failed to allocate" << std::endl;
                datafile.close();
                return 1;
            }
        }else if(command == "dealloc"){
            dealloc(recentMem);
        }else{
            std::cerr << "Error: Invalid command in datafile, skipping" << std::endl;
        }
        recentMem = occupiedChunks.back()->space;
    }

    std::cout << "Occupied Chunks:" << std::endl;
    for (allocation *chunk : occupiedChunks) {
        std::cout << "Address: " << chunk->space << ", Size: " << chunk->size << std::endl;
    }

    std::cout << "Free Chunks:" << std::endl;
    for (allocation *chunk : freeChunks) {
        std::cout << "Address: " << chunk->space << ", Size: " << chunk->size << std::endl;
    }

    datafile.close();

    return 0;
}