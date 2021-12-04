#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "checklinks.h"
#define Pat "<[^>]*[href|HREF]=[\"]?(http[s]?:[^?\"#][^ \"]*[^ \"/])[\"]?[^>]*>"
links * head = NULL; 
bool parallel; 
void checklinks(int argc, char ** argv){
int ch; 
 char * url, * fileName; 
  while ((ch = getopt(argc, argv, "f:ph")) != -1) {
    switch (ch) {
    case 'f':
      fileName = optarg;
      getFile(fileName);     
      break;
    case 'p':
      assert(argc > 2); // if p is pass make sure has something to run
      parallel = true; 
      break;
    case 'h':
      printf("[-f file name]\n");
      printf("[-p parallel run]\n");
      printf("[-h help]\n");
      break;
    default: 
      fprintf(stdout, "Unknown input -h for help commands"); 
      exit(EXIT_FAILURE); 
    }
  }
    if(optind == argc -1){
      url = malloc(sizeof(char) *  (1024 + strlen(argv[optind])));
      sprintf(url, "wget --no-cache --delete-after -q -O file.txt %s", argv[optind]); 
      fileName = "file.txt"; 
      if(system(url) != 0){
        fprintf(stdout, "Unable to retrieve url: %s\n", argv[optind]);  
        exit(EXIT_FAILURE); 
      }  
      getFile(fileName);
      remove(fileName); // Removes file made for test url
      free(url); 
    }
  printout(); 
  distructor(); 
}
void getFile(char * fileOrUrl){ 
  int retval; 
  regex_t regex;  
  char line[1024] = {0}; 
  retval = regcomp(&regex, Pat, REG_ICASE|REG_EXTENDED);
  if(retval){
    perror("Could not compile regular expression.\n");
    exit(EXIT_FAILURE); 
  } 
  FILE *fp = fopen(fileOrUrl, "r"); 
  if(!fp){
    perror(fileOrUrl);   
    exit(EXIT_FAILURE); 
  }
 while ((fgets(line, 1024, fp)) != NULL)
    {
        line[strlen(line)-1] = '\0';
        if ((retval = regexec(&regex, line, 0, NULL, 0)) == 0){ 
            checkNorm(line); 
        }   
    }
    regfree(&regex);
    fclose(fp); 
    if(parallel == true){
      Parallel(); 
    }
}
void checkNorm(char * fileOrUrl){
  size_t maxGroups = 4;
  regex_t regexCompiled;
  regmatch_t groupArray[maxGroups];

  if (regcomp(&regexCompiled, Pat, REG_EXTENDED))
    {
      perror("Could not compile regular expression.\n");
      exit(EXIT_FAILURE); 
    };

  if (regexec(&regexCompiled, fileOrUrl, maxGroups, groupArray, 0) == 0)
    {
      int g = 0;
      char buff[1024] = {0},
       newImpro[strlen(fileOrUrl)],
       sourceCopy[strlen(fileOrUrl) + 1];
      while(g < maxGroups){
         if (groupArray[g].rm_so == (size_t)-1){  
          strcpy(sourceCopy, fileOrUrl);
          sourceCopy[groupArray[g-1].rm_eo] = 0;
          strcpy(newImpro, sourceCopy + groupArray[g-1].rm_so); 
          sprintf(buff, "wget  --spider -q --delete-after -T10 -t1 '%s'", newImpro);  
          if(parallel == true){
            pushIt(newImpro, ""); 
          }else{
            if(system(buff) == 0){
              pushIt(newImpro, "okay");  
            }else{
              pushIt(newImpro, "error");  
            }
          }
          break; 
         }
        g++; 
      }
    }
  regfree(&regexCompiled);
}
void pushIt(char * linkname, char * status){
  links * newPtr = malloc(sizeof(links)); 
  strcpy(newPtr->status, status); 
  strcpy(newPtr->linkName, linkname); 
  newPtr->pid = 0; 
  newPtr->next = NULL;
  if(head == NULL){
    head = newPtr; 
  }else if(strcmp(newPtr->linkName, head->linkName) == 0){
    free(newPtr); 
    return; 
  }
  else if(strcmp(newPtr->linkName, head->linkName) < 0){
    newPtr->next = head;
    head = newPtr; 
  }else{
    links * curr = head; 
    while(curr->next != NULL && (strcmp(newPtr->linkName, curr->next->linkName) > 0 || strcmp(newPtr->linkName, curr->next->linkName) == 0)){
      if(strcmp(newPtr->linkName, curr->next->linkName) == 0){ 
        free(newPtr); 
        return; 
      }
      curr = curr->next;
    }
    newPtr->next = curr->next;
    curr->next = newPtr;
  }
}
void printout(){
  links * curr = head;
  while(curr != NULL){
    printf("%s %s\n", curr->status, curr->linkName); 
    curr = curr->next; 
  }
}
void Parallel(){
  links * curr = head; 
  while(curr != NULL){
    curr->pid = fork(); 
    if(curr->pid == 0){ 
      execl("/usr/bin/wget", "wget", "--spider", "-q", "--delete-after", "-T10", "-t1", curr->linkName, (char *)NULL); 
      perror(curr->linkName); 
      exit(EXIT_FAILURE); 
    }
    curr = curr->next; 
  }
  links * cur = head; 
  while(cur != NULL){
    int stat; 
    waitpid(cur->pid, &stat, 0);
    if(stat == 0) 
      strcpy(cur->status, "okay");
    else 
       strcpy(cur->status, "error");
    cur = cur->next; 
  }
}
void distructor(){
  links * curr = head; 
  while(curr != NULL){
    curr = curr->next; 
    free(head);
    head = curr;  
  }
}
