int main (int argc, char * argv [])
  {
    static char foo [29] = "Multics rulez, UNIX Droolz.  ";
    ioa_$ (& foo);
    return sizeof (foo);
  }
