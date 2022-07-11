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
        cout << "Could not find file!" << endl;
        exit(-99);
    }
}

int main(int argc, char* argv[])
{
    
    errno = 0;
    FILE* fileptr;
    unsigned char* buffer;
    long filelen;

    // The sections to test against
    std::vector<unsigned char> beginSq = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
    std::vector<unsigned char> endSq = { 0x49, 0x45, 0x4e ,0x44, 0xae, 0x42, 0x60, 0x82 };

    string path;
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
    


    cout << "-----" << endl;
    // Reading file
    fopen_s(&fileptr,path.c_str(), "rb");

    exitOnFileError();

    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);
    
    cout <<"File is " << filelen << " bytes in size."<< endl;
    // Read file to buffer
    buffer = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file
    fread(buffer, sizeof(unsigned char), filelen, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file
    
    // translate this to vector
    std::vector<unsigned char>* fvec = convertUCArrayToVec(filelen, buffer);
    
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