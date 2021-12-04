typedef struct link{
  int pid;
  char status[20]; 
  char linkName[1024]; 
  struct link * next;
}links;

void checklinks(int argc, char ** argv);
void checkNorm(char * fileOrUrl); 
void getFile(char * fileOrUrl); 
void pushIt(char * linkname, char * status);
void printout(void); 
char * checkP(char * fileOrUrl); 
void Parallel(void);
void distructor(void); 
