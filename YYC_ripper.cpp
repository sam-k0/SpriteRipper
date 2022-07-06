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

#define _CRT_SECURE_NO_WARNINGS

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

std::vector<unsigned char> read_bin(std::string filename)
{
    std::ifstream file(filename, std::ios::binary);
    file.unsetf(std::ios::skipws);
    std::streampos file_size;
    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> vec;
    vec.reserve(file_size);
    vec.insert(vec.begin(), std::istream_iterator<unsigned char>(file), std::istream_iterator<unsigned char>());
    return vec;
}

std::string vectostr(std::vector<unsigned char> vec)
{
    std::string buf;
    for (int i = 0; i < vec.size(); i++)
    {
        buf += vec.at(i);
    }
    return buf;
}

void writeFile(vector<unsigned char> *printvec)
{
    ofstream wf(getCurrentDir() + "\\img" + std::to_string(99) + ".win", ios::out, ios::binary);
    //wf.unsetf(ios::skipws);

    for (int xx = 0; xx < printvec->size(); xx++)
    {
        wf << printvec->at(xx);

        //cout << printvec->at(xx);
    }

    wf.close();
}

int cppmain()
{
   
    cout << "Hello World!\n";

    string path = getCurrentDir() + "\\data.win";
    cout << path << endl;

    std::vector <unsigned char> bytes = read_bin(path);

    writeFile(&bytes);

    return -88;

    std::vector<unsigned char> slice;

    // outbuf
    std::vector<unsigned char>* printvec = new std::vector<unsigned char>();
    

    std::vector<unsigned char> beginSq = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
    std::vector<unsigned char> endSq = { 0x49, 0x45, 0x4e ,0x44, 0xae, 0x42, 0x60, 0x82 };

    int beginsq = 0;
    int endsq = 0;
    bool found = false;
    int imgcnt = 0;

    //std::copy(bytes.begin(), bytes.end(), arr.begin());

    // amogus
    for (int i = 0; i < bytes.size()-8; i+= 1)
    {
        slice.clear();
       
        for (int ii = 0; ii < 8; ii++)
        {
            slice.push_back(bytes.at(i+ii)); // copy to slice
        }


        if (!found) // looking for begin section
        {
            /**auto it = std::search(slice.begin(), slice.end(), beginSq.begin(), beginSq.end()); // search for seq
            if (it != slice.end()) // found
            {             
                found = true;
                beginsq = i;
                cout << "SuS";
            }*/

            if (vectostr(slice) == vectostr(beginSq))
            {
                found = true;
                beginsq = i;
                cout << "begin";
            }
        }

        if (found) // looking for end
        {
            //auto it = std::search(slice.begin(), slice.end(), endSq.begin(), endSq.end()); // search for seq
            //if (it != slice.end()) // found end
            if(vectostr(slice) == vectostr(endSq))
            {
                found = false;
                cout << "End\n";
                endsq = i+8;
               
                for (int yy = beginsq; yy < endsq; yy++)
                {
                    printvec->push_back(bytes.at(yy) );
                }
                imgcnt++;
                // save to file (printarr)
                ofstream wf(getCurrentDir() + "\\img"+ std::to_string(imgcnt)+".png", ios::out, ios::binary );
                wf.unsetf(ios::skipws);

                for (int xx = 0; xx < printvec->size(); xx++)
                {
                    wf << printvec->at(xx);
                    
                    cout << printvec->at(xx);
                }

                wf.close();
                printvec->clear();
                return -99;
            }
       
        }
    }

  
}

int main()
{
    FILE* fileptr;
    unsigned char* buffer;
    long filelen;

    string path = getCurrentDir() + "\\origtest.png";

    fileptr = fopen(path.c_str(), "rb");
    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);

    buffer = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file
    fread(buffer, sizeof(unsigned char), filelen, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file
    
    // translate this to vector
    std::vector<unsigned char>* fvec = new std::vector<unsigned char>();

    for (int i = 0; i < filelen; i++)
    {
        unsigned char tc = buffer[i];
        fvec->push_back(tc);
    }

    cout << fvec->size();



    // vec to array
    unsigned char* pbuffer = (unsigned char*)malloc(filelen * sizeof(unsigned char)); // Enough memory for the file
    for (int i = 0; i < filelen; i++) // change to vecsize
    {
        pbuffer[i] = fvec->at(i);
    }

    fileptr = fopen("mogus.png", "wb");
    fwrite(pbuffer, sizeof(unsigned char), filelen, fileptr);
    fclose(fileptr); // Close the file

    /**fileptr = fopen("mogus.png", "wb");
    fwrite(buffer, sizeof(unsigned char), filelen, fileptr);
    fclose(fileptr); // Close the file*/

    return 0;
}