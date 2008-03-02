#include <cstdio>

class File_ptr {
    FILE* f;
public:
    File_ptr (const char* fname, const char* mode)
       {  if (! (f = fopen (fname, mode)))
             printf ("Kann \"%s\" nicht öffnen\n", fname);
       }
   ~File_ptr () 	{ printf("Schließe\n"); if (f) fclose(f); }
   FILE* operator()() 	{ return f; }
};


int main ()
{
  {
    File_ptr fp ("datei", "ro");
  }
    printf ("danacb\n");
}

