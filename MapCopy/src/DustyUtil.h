/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is DustyUtil, a set of utility functions and classes.
 * 
 * The Initial Developer of the Original Code is James Dustin Reichwein.
 * Portions created by James Dustin Reichwein are
 * Copyright (C) 2000, James Dustin Reichwein.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 */

////////////////////////////////////////////////////////////////////////////////
// Header File: DustyUtil.h
//
// Description: Dusty's Utility functions.
//
// Written By:  Dusty Reichwein
//
// Version:     1.0
//
// Created:     December 8, 1997
//
// Modification History:
// Dec/11/1997  JDR   Last modification under the "LDisk" project.
// Sep/03/2000  JDR   Moved from LDisk to project to a generic utility file
//                    Made smart pointer destroy on assignment. Added
//                    equality, array operators. Added prototypes for
//                    string case functions. Promoted to version 1.0
//                    Renamed to: DustyUtil
// Sep/13/2000  JDR   Released under MPL
// Feb/10/2005  JDR   Changes for mingw compiler, removed unncessary array 
//                    operators.
// Feb/13/2005  JDR   Added debug output utility class and methods.
////////////////////////////////////////////////////////////////////////////////

#ifndef DUSTYUTIL_H_
#define DUSTYUTIL_H_

#include <stddef.h>
#include <sstream>
#include <string>
using namespace std;

namespace DustyUtil
{

    // Functions that aren't standard in ANSI C++

    // Convert case of string
    string convert_to_lower(string& s);
    string convert_to_upper(string& s);

    // Create a copy of the string of a new case
    string copy_to_lower(const string s);
    string copy_to_upper(const string s);

    // Determine if a file exists
    bool fileExists(const string filename);

    // A smart pointer class, that destroys its contents with delete
    // The optional argument makes sure that objects constructed with new[] are
    // deleted with delete[])
    template <class T, bool isArray=false>
    class SmartPointer
    {
        public:
        // Construct with a pointer
        SmartPointer(T *ptr)
        {
            pointer=ptr;
        }

        // construct without a pointer
        SmartPointer()
        {
            pointer=NULL;
        }

        // Release control of a pointer
        // sets this pointer to NULL without deleting it,
        // and returns the old pointer.
        T* releaseControl()
        {
            T* temp = pointer;
            pointer = NULL;
            return temp;
        }


        // Assign to a new pointer
        SmartPointer& operator=(T *ptr)
        {
            destroy();
            pointer=ptr;
            return *this;
        }

        // Destroy the contents
        void destroy()
        {
            if (pointer!=NULL)
            {
                if (isArray) delete[] pointer;
                else delete pointer;
            }
        }


        // Destroy, using delete to destroy the contents
        ~SmartPointer()
        {
            destroy();
        }

        // type conversion to regular boring pointer
        operator T*()
        {
            return pointer;
        }

        operator const T*() const
        {
            return pointer;
        }

        // Simulate pointer operations
        T& operator*()
        {
            return *pointer;
        }
        const T& operator*() const
        {
            return *pointer;
        }

        T** operator&()
        {
            return &pointer;
        }

        T* operator->()
        {
            return pointer;
        }
        const T* operator->() const
        {
            return pointer;
        }

        // Check to see if the interface pointer is NULL
        bool isNull() const
        {
            return (pointer==NULL);
        }

        // Retrieve the pointer
        T* get()
        {
            return pointer;
        }

        const T* get() const
        {
            return pointer;
        }

        // Comparison
        bool operator == (SmartPointer<T,isArray> sp) const
        {
            return (pointer == sp.pointer);
        }

        bool operator == (T* p) const
        {
            return (pointer == p);
        }

        protected:

        T *pointer;
    };

    // Utility methods for output that's enabled/disabled by a global verbose
    // settings.  This has all static members and cannot be instantiated.
    // Future enhancements could be to support multiple output streams
    // (aka log files and stdout), multiple levels of logging (debug vs error)
    // perhaps with different levels going to different streams.
    class LogOutput
    {
        public:
        static void setOutputStream(ostream& outputStream)
        { stream = &outputStream; }

        static void setEnabled(bool b)         
        { enabled = b; }

        static ostream& log()
        {
            if (enabled) return *stream;
            else return nullStream;
        }

        private:

        static ostream* stream;
        static ostream nullStream;
        static bool enabled;
    };

    // This is a "null" stream buf that reads nothing and writes nothing
    class nullBuf : public streambuf
    {
        public:
            nullBuf() { }
    };
}
#endif

