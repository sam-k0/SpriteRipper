/*
    SpriteRipper
    A simple tool to extract PNG images from GameMaker Studio 1.4 game binaries.
    Author: samk0
*/
// Common includes
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#define LINUX 1 // Switch this for Linux or Windows
#if LINUX == 1 // Linux includes
#include <unistd.h>
#include <limits.h>
#include <cerrno>
#include <stdlib.h>
#else // Windows, not sure if these are even needed
#include <stdio.h>
#include <array>
#include <fstream>
#endif

// The sections to test against
std::vector<unsigned char> BEGIN_SQ = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
std::vector<unsigned char> END_SQ = { 0x49, 0x45, 0x4e ,0x44, 0xae, 0x42, 0x60, 0x82 };


void uni_fopen(FILE** fileptr, std::string path, std::string mode)
{
    #if LINUX == 0
    fopen_s(fileptr, path.c_str(), mode.c_str());
    #else
    *fileptr = fopen(path.c_str(), mode.c_str());
    #endif
}


std::string getCurrentDir() // Returns EXE directory
{
    char cCurrentPath[FILENAME_MAX]; // get working directory into buffer
    #if LINUX == 0
    if (!_getcwd(cCurrentPath, sizeof(cCurrentPath)))
        exit(-1);
    #else
    if (getcwd(cCurrentPath, sizeof(cCurrentPath)) == NULL)
        exit(-1);
    #endif
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; // not really required

    char* s = cCurrentPath; // save path from buffer into currentpath chararr
    return std::string(s);
}

unsigned char* convertToUCArray(std::vector<unsigned char>* fvec, long filelen)
{
    unsigned char* pbuffer = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file
    for (int i = 0; i < filelen; i++) // change to vecsize
    {
        pbuffer[i] = fvec->at(i);
    }
    return pbuffer;
}

std::vector<unsigned char>* convertUCArrayToVec(long filelen, unsigned char* buffer)
{
    // translate this to vector
    std::vector<unsigned char>* fvec = new std::vector<unsigned char>();

    for (int i = 0; i < filelen; i++)
    {
        unsigned char tc = buffer[i];
        fvec->push_back(tc);
    }
    return fvec;
}

//Reads the entire game binary file into a byte vector
std::vector<unsigned char>* gameBinaryToBytevec(std::string path, long* filelen, FILE* fileptr)
{
    // Local scope variable defs
    unsigned char* buffer;
    // Passed-out variable defs
    std::vector<unsigned char>* fvec;

    // Reading file
    uni_fopen(&fileptr, path, "rb");
    if (errno != 0)
    {
        printf("Could not find file! Error code: %d\n", errno);
        exit(-99);
    }

    fseek(fileptr, 0, SEEK_END);
    *filelen = ftell(fileptr);
    rewind(fileptr);

    // Read file to buffer
    buffer = (unsigned char*)malloc(*filelen * sizeof(unsigned char)); // Enough memory for the file
    fread(buffer, sizeof(unsigned char), *filelen, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

    // translate this to vector
    fvec = convertUCArrayToVec(*filelen, buffer);
    
    free(buffer);

    return fvec;
}

// Used to read a PNG file to byte vector
std::vector<unsigned char>* spriteSheetToBytevec(std::string path)
{
    // Local scope variable defs
    FILE* fileptr;
    long filelen = 0;
    unsigned char* buffer;
    // Passed-out variable defs
    std::vector<unsigned char>* fvec;

    // Reading file
    uni_fopen(&fileptr, path, "rb");
    if (errno != 0)
    {
        printf("Could not find file! Error code: %d\n", errno);
        exit(-99);
    }

    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);

    // Read file to buffer
    buffer = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file
    fread(buffer, sizeof(unsigned char), filelen, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

    // translate this to vector
    fvec = convertUCArrayToVec(filelen, buffer);
    free(buffer);

    return fvec;
}



int main(int argc, char* argv[])
{
    // Runtime variable definitions   
    errno = 0;              // error status
    FILE* fileptr = NULL;   // Fileptr to the Game binary file
    unsigned char* buffer;  // Buffer to hold the game binary read bytes
    long filelen;           // The length of the game binary file in BYTES
    std::string path;       // The filepath of the game binary


    if (argc == 2)
    {
        printf("Opened by drag-and-drop!\n");
        path = std::string(argv[1]);
    }
    else
    { 
        // User input filename
        std::string targetfname = "";
        printf("Please input the name of the file to extract from in the current directory: (Example: data.win)\n");
        #if LINUX == 0
            std::cin >> targetfname;
        #else
            std::getline(std::cin, targetfname);
        #endif

        path = getCurrentDir() + "\\" + targetfname;
    }
    
    // Read the entire game file into a vector
    std::vector<unsigned char>* fvec = gameBinaryToBytevec(path.c_str(), &filelen, fileptr);
    printf("Copied %d bytes to process.\n", fvec->size());
    // Process the read data for sections
    int beginsq = 0;
    int endsq = 0;
    bool found = false;
    int imgcnt = 0;

    std::vector<unsigned char> slice;

    for (int i = 0; i < fvec->size() - 8; i += 1)
    {
        slice.clear();

        for (int j = 0; j < 8; j++)
        {
            slice.push_back(fvec->at(i + j)); // copy to slice
        }

        if (!found) // looking for begin section
        {
            auto it = std::search(slice.begin(), slice.end(), BEGIN_SQ.begin(), BEGIN_SQ.end()); // search for seq
            if (it != slice.end()) // found
            {
                found = true;
                beginsq = i;
                printf("->Begin PNG sequence at byte %d\n", beginsq);
            }
        }

        if (found) // looking for end
        {        
            auto it = std::search(slice.begin(), slice.end(), END_SQ.begin(), END_SQ.end()); // search for seq
            if (it != slice.end()) // found end            
            {
                found = false;                
                endsq = i + 8;
                printf("->End PNG sequence at byte %d\n", endsq);


                // get printvec
                std::vector<unsigned char> printvec; // The vector to add the stuff to
                unsigned char* printbuf;        // The buffer to be printed then
                printbuf=(unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file

                for (int yy = beginsq; yy < endsq; yy++) // adding to the vector somehow worked easier than adding to the buffer
                {
                    printvec.push_back(fvec->at(yy));
                }

                // Converting the vec back to array                                   
                printbuf = convertToUCArray(&printvec, printvec.size());
                imgcnt++; // The counter of how many images have been already saved
                std::string savefilename = "assetrip" + std::to_string(imgcnt) + ".png";
                // Printing file..
                uni_fopen(&fileptr, savefilename, "wb");
                fwrite(printbuf, sizeof(unsigned char), (endsq - beginsq), fileptr);
                fclose(fileptr); // Close the file
                free(printbuf);
            }
        }
    }

    // Clean up
    delete fvec;
    printf("Finished! %d images extracted.\n", imgcnt);
    #if LINUX == 0
    while (!kbhit());
    #endif

    return 0;
}