#include <iostream>
#include <list>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

struct allocation{ //provided allocation struct from pdf
std::size_t size; 
void *space; 
};

std::list<allocation*> occupiedChunks; //linked list of pointers to occupied chunks
std::list<allocation*> freeChunks; //linked list of pointers to free chunks
std::string strategy = "bestfit"; //strategy definition because I wrote a single program taking strategy as an input initially instead of 2 separate ones

allocation * firstFit(std::size_t chunkSize){ //first fit algorithm implementation
    allocation * nullChunk = nullptr; //initialise a null chunk to return a null pointer for future checking
    for (allocation * chunk : freeChunks){ //loop over every chunk in the list of free chunks
        if (chunk->size >= chunkSize){ // if the size of the chunk is greater of equal to the desired size
            return chunk; //return that chunk
        }
    }
    return nullChunk; //otherwise if no valid chunks are found return the null pointer
}

allocation * bestFit(std::size_t chunkSize){ //best fit algorithm implementation
    std::size_t sizeDiff = SIZE_MAX; //initialise the difference in size between the desired chunk size and the found chunk size to be the max possible
    allocation * bestChunk = nullptr; //initialise the best chunk found to be a null pointer for future checking
    for (allocation * chunk : freeChunks){ //loop over every chunk in the list of free chunks
        if ((chunk->size >= chunkSize) && (chunk->size - chunkSize) < sizeDiff){ //check if the size of the found chunk is greater than or equal to the provided chunk size and the difference between the 2 is smaller than the lowest found so far
            sizeDiff = chunk->size - chunkSize; //if checks pass then set the size difference variable to the current difference
            bestChunk = chunk; //set the best chunk found to the current chunk
        }
    }
    return bestChunk; //return the best chunk found
}

void * alloc(std::size_t chunkSize){ //allocation algorithm
    if(chunkSize < 32){ //chunk size validation as per spec
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

    allocation * chunk; //this is the test variable to check the return from the firstfit or bestfit algorithms
    if(strategy == "firstfit"){ //legacy code from when the algorithm was parsed as a command line argument
        chunk = firstFit(chunkSize); //run a firstfit search
    }else if(strategy == "bestfit"){ //legacy code from when the algorithm was parsed as a command line argument
        chunk = bestFit(chunkSize); //run a bestfit search
    }else{ //legacy code from when the algorithm was parsed as a command line argument
        std::cerr << "Error: strategy value not valid" << std::endl; //error message
        return nullptr; //return nullptr for error checking
    }

    if(chunk){ //check if the output of the search found
        freeChunks.remove(chunk); //remove the found chunk from the list of free chunks
        occupiedChunks.push_back(chunk); //add the chunk to the list of occupied chunks
        return chunk->space; //return the memory space of the chunk
    }


    void * memAlloc = sbrk(chunkSize); //if no chunk was found allocate enough memory to create it
    if (memAlloc == (void*)-1){ //check if the memory was successfully allocated
        std::cerr << "Error: failed to allocate " << chunkSize << " bytes of memory" << std::endl; //error message
        return nullptr; //return null pointer for error checking
    }

    allocation * newChunk = new allocation(); //create a new allocation object (considering this allocates heap memory I understand if this is not allowed but that wasn't specified in the pdf)
    newChunk->size = chunkSize; //set the size of the new chunk to be the provided chunk size
    newChunk->space = memAlloc; //set the space of the new chunk to be the recently allocated memory using sbrk
    occupiedChunks.push_back(newChunk); //add the created chunk pointer to the list of occupied chunk pointers
    return newChunk->space; //return the space of the chunk
};

void dealloc(void * chunk){ //dealloc algorithm
    for (std::list<allocation*>::iterator search = occupiedChunks.begin(); search != occupiedChunks.end(); search++){ //iterate over the list of occupied chunks 
        allocation * element = *search; //dereference search to get a memory allocation pointer (as search is a pointer to a pointer)
        if(element->space == chunk){ //check if the current chunk's space is the desired chunk space
            freeChunks.push_back(element); //if it is add that chunk to the list of free chunks
            occupiedChunks.erase(search); //remove the chunk from the list of occupied chunks
            return; //exit early to avoid searching any more
        }
    }
    throw std::runtime_error("No chunk found"); //if no chunk is found, throw an error as per the spec
};




int main(int argc, char* argv[]){ //main function
    if(argc != 2){ //check the number of provided command line arguments
        std::cerr << "Please enter arguments in the form ./<strategy> <datafile>" << std::endl; //tell the user the correct way to run the program
        return 1; //return a 1 as an error occured
    }


    std::ifstream datafile(argv[1]); //create an input stream with the datafile as a source
    if(!datafile.is_open()){  //check if the input stream was unable to be created successfully
        std::cerr << "Error: Could not open file " << argv[1] << std::endl; //give an error message
        return 1; //return 1 as an error occurred
    }


    std::string line; //line to be read from the input file
    void * recentMem; //most recent memory allocated (as dealloc testing only wants to deallocate the most recent chunk of memory)
    void * allocTest; //test for the output of the alloc algorithm
    while(std::getline(datafile, line)){ //loop over every line in the datafile
        std::istringstream iss(line); //get the line from the input stream as a string stream
        std::string command; //create a temp variable to store the command read from the line
        iss >> command; //push the line from the string stream to the command variable

        if(command == "alloc:"){ //check if the command is to allocate memory
            std::size_t size; //get a variable to store the provided memorys ize
            iss >> size; // get the size from the string stream
            allocTest = alloc(size); //allocate the memory of that size
            if (allocTest == nullptr){ //if the return from the alloc function is a nullptr
                std::cerr << "Error: Memory failed to allocate" << std::endl; //give an error messaage
                datafile.close(); //close the data file
                return 1; //return 1 as an error occurred
            }
        }else if(command == "dealloc"){ //check if the command is to deallocate memory
            dealloc(recentMem); //run the dealloc on the most recently allocated chunk of memory
        }else{ //check if the command is anything else
            std::cerr << "Error: Invalid command in datafile, skipping" << std::endl; //give an error message but continue to run the program as no data was altered
        }
        recentMem = occupiedChunks.back()->space; //set the most recently allocated chunk of memory to be the end of the occupied chunks list
    }

    std::cout << "Occupied Chunks:" << std::endl; //print out the occupied chunks one by one
    for (allocation * chunk : occupiedChunks){ //loop over every chunk in the occupied chunks list
        std::cout << "Address: " << chunk->space << ", Size: " << chunk->size << std::endl; //print out the chunk space and the size of the chunk
    }

    std::cout << "Free Chunks:" << std::endl; //print out the free chunks one by one
    for (allocation * chunk : freeChunks){ //loop over every chunk in the free chunks list
        std::cout << "Address: " << chunk->space << ", Size: " << chunk->size << std::endl; //print out the chunk space and the size of thec hunk
    }

    datafile.close(); //close the input file

    return 0; //return 0 as programm executed successfully
}