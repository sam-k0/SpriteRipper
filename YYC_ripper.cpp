// YYC_ripper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "yyg.h"
#include <iostream>
#include <direct.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <array>
#include <string>
#include <stdio.h>
#include <conio.h>


using namespace std;

// The sections to test against
std::vector<unsigned char> beginSq = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
std::vector<unsigned char> endSq = { 0x49, 0x45, 0x4e ,0x44, 0xae, 0x42, 0x60, 0x82 };


std::string getCurrentDir() // Returns EXE directory
{
    char cCurrentPath[FILENAME_MAX]; // get working directory into buffer
    if (!_getcwd(cCurrentPath, sizeof(cCurrentPath)))
        exit(-1);
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; // not really required

    char* s = cCurrentPath; // save path from buffer into currentpath chararr
    return std::string(s);
}

unsigned char* convertToUCArray(vector<unsigned char>* fvec, long filelen)
{
    unsigned char* pbuffer = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file
    for (int i = 0; i < filelen; i++) // change to vecsize
    {
        pbuffer[i] = fvec->at(i);
    }
    return pbuffer;
}

vector<unsigned char>* convertUCArrayToVec(long filelen, unsigned char* buffer)
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

void exitOnFileError()
{
    if (errno != 0)
    {
        std::cout << "Could not find file!" << endl;
        exit(-99);
    }
}

//Reads the entire game binary file into a byte vector
vector<unsigned char>* gameBinaryToBytevec(std::string path, long* filelen, FILE* fileptr)
{
    // Local scope variable defs
    //FILE* fileptr;
    //long filelen = 0;
    unsigned char* buffer;
    // Passed-out variable defs
    std::vector<unsigned char>* fvec;

    // Reading file
    fopen_s(&fileptr, path.c_str(), "rb");

    exitOnFileError();

    fseek(fileptr, 0, SEEK_END);
    *filelen = ftell(fileptr);
    rewind(fileptr);

    // Read file to buffer
    buffer = (unsigned char*)malloc(*filelen * sizeof(unsigned char)); // Enough memory for the file
    fread(buffer, sizeof(unsigned char), *filelen, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

    // translate this to vector
    fvec = convertUCArrayToVec(*filelen, buffer);

    // Free the buffer??
    // will it break the copied values?
    free(buffer);

    return fvec;
}

// Used to read a PNG file to byte vector
vector<unsigned char>* spriteSheetToBytevec(std::string path)
{
    // Local scope variable defs
    FILE* fileptr;
    long filelen = 0;
    unsigned char* buffer;
    // Passed-out variable defs
    std::vector<unsigned char>* fvec;

    // Reading file
    fopen_s(&fileptr, path.c_str(), "rb");

    exitOnFileError();

    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);

    // Read file to buffer
    buffer = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file
    fread(buffer, sizeof(unsigned char), filelen, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file

    // translate this to vector
    fvec = convertUCArrayToVec(filelen, buffer);

    // Free the buffer??
    // will it break the copied values? Doesnt seem like it
    free(buffer);

    return fvec;
}

// Returns as integer: start of the first PNG sequence in the vector
long findPNGstart(vector<unsigned char>* vect)
{
    long pngstart = -1;
    vector<unsigned char> slice;
    bool found = false;

    for (int i = 0; i < vect->size() - 8; i += 1)
    {
        slice.clear();

        for (int ii = 0; ii < 8; ii++)
        {
            slice.push_back(vect->at(i + ii)); // copy to slice
        }

        if (!found) // looking for begin section
        {
            auto it = std::search(slice.begin(), slice.end(), beginSq.begin(), beginSq.end()); // search for seq
            if (it != slice.end()) // found
            {
                found = true;
                pngstart = i;                
            }
        }
    }

    return pngstart;
}

long findPNGend(vector<unsigned char>* vect) 
{
    long pngend = -1;
    vector<unsigned char> slice;
    bool found = false;

    for (int i = 0; i < vect->size() - 7; i += 1)
    {
        slice.clear();

        for (int ii = 0; ii < 8; ii++)
        {
            slice.push_back(vect->at(i + ii)); // copy to slice
        }

        if (!found) // looking for begin section
        {
            auto it = std::search(slice.begin(), slice.end(), endSq.begin(), endSq.end()); // search for seq
            if (it != slice.end()) // found
            {
                found = true;
                pngend = i + 8;
            }
        }
    }

    return pngend;
}

void writePatchedBinary(vector<unsigned char>* vect)
{
    unsigned char* printbuf;        // The buffer to be printed then
    long filelen = vect->size();
    FILE* fptr = NULL;

    cout << "Filelen: " << vect->size() << endl;
    printbuf = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file

    // Converting the vec back to array lols

    printbuf = convertToUCArray(vect, vect->size());
    // save to file (printarr)

    string savefilename = "patch.win";
    // Printing file..
    fopen_s(&fptr, savefilename.c_str(), "wb");
    fwrite(printbuf, sizeof(unsigned char), filelen, fptr);
    fclose(fptr); // Close the file
    free(printbuf);
}

// This will return the gamebin vec with the spritesheet vec injected at the png section
void injectSpritesheet(std::vector<unsigned char>* gameBinaryByteVec, std::vector<unsigned char>* spriteSheetByteVec)
{
    // TODO: Start von alte bin -> start bis png in datei kopieren, dann den neuen vec injecten, dann png end bis bin end adden
    // vector.insert??
    long PNG_START_GAME = findPNGstart(gameBinaryByteVec);
    long PNG_END_GAME = findPNGend(gameBinaryByteVec);
    long GAME_VEC_LEN = gameBinaryByteVec->size();
    long GAME_SPRITE_LEN = PNG_END_GAME - PNG_START_GAME;

    long PNG_START_PATCH = findPNGstart(spriteSheetByteVec);
    long PNG_END_PATCH = findPNGend(spriteSheetByteVec);
    long PATCH_VEC_LEN = spriteSheetByteVec->size();
    long PATCH_SPRITE_LEN = PNG_END_PATCH - PNG_START_PATCH;

    long GAME_TO_PATCH_DIFF = GAME_SPRITE_LEN - PATCH_SPRITE_LEN;

    
    // padding TODO: Fix padding automatically. try injecting pre-compiled gamemaker sprites
    int paddingBytes = 0;
    for (int i = 0; i < paddingBytes; i++)
    {
        spriteSheetByteVec->push_back(0x00);
    }

    vector<unsigned char> AudioSection(gameBinaryByteVec->begin() + PNG_END_GAME, gameBinaryByteVec->end());
    
    // Erase old PNG sector
    gameBinaryByteVec->erase(gameBinaryByteVec->begin() + PNG_START_GAME, gameBinaryByteVec->begin() + PNG_END_GAME);
    
    // Write the new png sector after the original one
    gameBinaryByteVec->insert(gameBinaryByteVec->begin() + PNG_START_GAME, spriteSheetByteVec->begin() + PNG_START_PATCH, spriteSheetByteVec->begin() + PNG_END_PATCH+paddingBytes);

    cout << "pre audio len: " << gameBinaryByteVec->size() << endl;


    //gameBinaryByteVec->insert(gameBinaryByteVec->end(), AudioSection.begin(), AudioSection.end());
    
    cout << "post : " << gameBinaryByteVec->size() << endl;
//    gameBinaryByteVec->insert(gameBinaryByteVec->end(), AudioSection.begin(), AudioSection.end());
 
    writePatchedBinary(gameBinaryByteVec);
}

int main(int argc, char* argv[])
{
    long tfilelen = 0;
    FILE* tfileptr = NULL;
    // Test
    cout << "Started" << endl;
    std::string pathSheetPNG = getCurrentDir() + "\\"+ "AssetPatcherGMX\\" + "patch.png"; // The full path to the to be patched png file
    std::string pathGameBin = getCurrentDir() + "\\" + "AssetPatcherGMX\\" + "data_original2.win";

    cout << "Loading hex vecs" << endl;
    auto pngVec = spriteSheetToBytevec(pathSheetPNG);
    auto binVec = gameBinaryToBytevec(pathGameBin, &tfilelen, tfileptr);


    

    cout << "Injecting spritesheet" << endl;
    injectSpritesheet(binVec, pngVec);

    cout << "Write patched bin" << endl;
    writePatchedBinary(binVec);

    cout << "Press any key to close." << endl;
    while (!_kbhit());

    return 0;
    // Runtime variable definitions   
    errno = 0;              // error status
    FILE* fileptr = NULL;   // Fileptr to the Game binary file
    unsigned char* buffer;  // Buffer to hold the game binary read bytes
    long filelen;           // The length of the game binary file in BYTES
    string path;            // The filepath of the game binary


    if (argc == 2)
    {
        cout << "Opened by drag-and-drop!" << endl;
        cout << argv[1] << endl;
        path = string(argv[1]);
    }
    else
    { 
        // User input filename
        string targetfname = "";
        cout << "Please input the name of the file to extract from in the current directory: (Example: data.win)" << endl;
        cin >> targetfname;

        // default name
        if (targetfname == " ")
        {
            targetfname = "data.win";
        }

        path = getCurrentDir() + "\\" + targetfname;
    }
    
    // Read the entire game file into a vector
    std::vector<unsigned char>* fvec = gameBinaryToBytevec(path.c_str(), &filelen, fileptr);
    
    cout << "Copied "<<fvec->size()<<" bytes to process."<<endl;
    cout << "-----" << endl;
    // Process the read data for sections
    int beginsq = 0;
    int endsq = 0;
    bool found = false;
    int imgcnt = 0;

    vector<unsigned char> slice;

    for (int i = 0; i < fvec->size() - 8; i += 1)
    {
        slice.clear();

        for (int ii = 0; ii < 8; ii++)
        {
            slice.push_back(fvec->at(i + ii)); // copy to slice
        }

        if (!found) // looking for begin section
        {
            auto it = std::search(slice.begin(), slice.end(), beginSq.begin(), beginSq.end()); // search for seq
            if (it != slice.end()) // found
            {
                found = true;
                beginsq = i;
                cout << "->Begin PNG sequence at byte "<< beginsq << endl;
            }
        }

        if (found) // looking for end
        {        
            auto it = std::search(slice.begin(), slice.end(), endSq.begin(), endSq.end()); // search for seq
            if (it != slice.end()) // found end            
            {
                found = false;                
                endsq = i + 8;
                cout << "->End PNG sequence at byte " <<endsq<< endl;
                cout << "-----" << endl;

                // get printvec
                vector<unsigned char> printvec; // The vector to add the stuff to
                unsigned char* printbuf;        // The buffer to be printed then
                printbuf=(unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file

                for (int yy = beginsq; yy < endsq; yy++) // adding to the vector somehow worked easier than adding to the buffer
                {
                    printvec.push_back(fvec->at(yy));
                }

                // Converting the vec back to array lols
                                                               
                printbuf = convertToUCArray(&printvec, printvec.size());
                // save to file (printarr)

                imgcnt++; // The counter of how many images have been already saved
                string savefilename = "assetrip" + std::to_string(imgcnt) + ".png";
                // Printing file..
                fopen_s(&fileptr,savefilename.c_str(), "wb");
                fwrite(printbuf, sizeof(unsigned char), (endsq - beginsq), fileptr);
                fclose(fileptr); // Close the file
                free(printbuf);
            }
        }
    }

    // Clean up
    delete fvec;
    cout << "Done! Extracted "<< imgcnt <<" assets" << endl;
    cout << "Press any key to close." << endl;
    while (!_kbhit());

    return 0;
}