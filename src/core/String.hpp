#pragma once
#include <stdio.h>
#include <string.h>

#include "Types.hpp"
#include "List.hpp"

#define MAXSTINGSIZE (2000)

class String
{
public:
    String()
    {
        List::Init(&data, 1, sizeof(char));
        data[0] = '\0';
    }

    String(const char *s)
    {
        if (s == nullptr)
        {
            List::Init(&data, 1, sizeof(char));
            data[0] = '\0';
        }
        else
        {
            unsigned int length = strlen(s) + 1;
            List::Init(&data, length, sizeof(char));
            strcpy(data, s);
        }
    }

    String(const String &_string)
    {
        const char *s = _string.CString();
        List::Init(&data, (unsigned int)(strlen(s) + 1), sizeof(char));
        strcpy(data, s);
    }

    String(String &&_string) noexcept
    {
        data = _string.data;
        _string.data = nullptr;
    }

    ~String()
    {
        if (data != nullptr)
        {
            List::Free(&data);
            data = nullptr;
        }
    }

    u32 Count()
    {
        return List::GetCount(&data) - 1;
    }

    const char *CString() const
    {
        return data != nullptr ? data : "";
    }

    operator const char *() const { return (const char *)data; }
    operator bool() const { return data[0] == '\0'; }

    String &operator=(const char *cString)
    {
        if (this->data == cString)
            return *this;

        // Free existing data if it exists
        if (this->data != nullptr)
        {
            //List::Free(&this->data);
        }

        // Initialize new data
        unsigned int newLength = strlen(cString) + 1;
        for(int i = 0; i < newLength+1; i++)
            List::Add(&this->data, &cString[i]);
        //List::Init(&this->data, newLength, sizeof(char));
        //strcpy(this->data, cString);

        return *this;
    }

    String &operator=(const String &_string)
    {
        // printf("o= %p\n", this->data);
        if (this == &_string)
            return *this; // Check for self-assignment

        // Free the existing data
        List::Free(&this->data);

        // Allocate new memory and copy the data
        unsigned int newLength = strlen(_string.CString()) + 1;
        List::Init(&this->data, newLength, sizeof(char));
        strcpy(this->data, _string.CString());

        return *this;
    }

    
    String &operator+(const char *cString)
    {
        if (this->data == cString)
            return *this;

        for (int s = 0; s < MAXSTINGSIZE; s++)
        {
            if (this->data[s] == '\0')
            {
                for (int i = 0; i < MAXSTINGSIZE; i++)
                {
                    if (cString[i] == '\0')
                    {
                        memcpy(&(this->data[s]), cString, i);
                        break;
                    }
                }
                break;
            }
        }

        return *this;
    }

    String &operator+=(const char *cString)
    {
        if (this->data == cString)
            return *this;

        for (int s = 0; s < MAXSTINGSIZE; s++)
        {
            if (this->data[s] == '\0')
            {
                for (int i = 0; i < MAXSTINGSIZE; i++)
                {
                    if (cString[i] == '\0')
                    {
                        memcpy(&(this->data[s]), cString, i);
                        break;
                    }
                }
                break;
            }
        }

        return *this;
    }

    /*String& operator+(const u32 _number)
    {
        char str[100];
        sprintf(str, "%d", _number);
        *this += (const char *)&str;
        return *this;
    }

    String& operator+=(const u32 _number)
    {
        char str[100];
        sprintf(str, "%d", _number);
        *this += (const char *)&str;
        return *this;
    }

    String operator+(size_t n) const
    {
        if (n >= MAXSTINGSIZE) {
            return String("");
        }
        return String(*this + n);
    }*/

private:
    char *data = nullptr;
};

inline String ToString(u32 _number)
{
    char str[100];
    sprintf(str, "%d", _number);
    return String((const char *)&str);
}
