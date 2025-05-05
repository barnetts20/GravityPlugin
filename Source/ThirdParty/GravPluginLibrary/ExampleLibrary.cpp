#if defined _WIN32 || defined _WIN64
    #include <Windows.h>

    #define EXAMPLELIBRARY_EXPORT __declspec(dllexport)
#else
    #include <stdio.h>
#endif

#ifndef EXAMPLELIBRARY_EXPORT
    #define EXAMPLELIBRARY_EXPORT
#endif

EXAMPLELIBRARY_EXPORT void ExampleLibraryFunction()
{

}