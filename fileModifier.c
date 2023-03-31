#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static char * read_stdin(void)
{
  size_t cap = 4096, len = 0;
  char *buffer = malloc(cap * sizeof(char));
  int c;

  while((c = fgetc(stdin)) != '\n' && !feof(stdin))
  {
    buffer[len] = c;

    if(++len == cap)
      buffer = realloc(buffer, (cap *= 2) * sizeof(char));
  }

  buffer = realloc(buffer, (len + 1) * sizeof(char));
  buffer[len] = '\0';

  return buffer;
}

void rf_menu(char *filepath)
{
  char *input;
  struct stat filestat;
  printf("Regular File Menu:\n\n");

  printf("-n File name\n");
  printf("-d File size\n");
  printf("-h Number of hardlinks\n");
  printf("-m Time of last modification\n");
  printf("-a Acces rights\n");
  printf("-l Create a symbolic link\n");
  printf("-q Exit\n");
  while(1)
  {
    printf("Chose an option:\n");
  
    input = read_stdin();

    for(int i = 1; input[i] != '\0'; i++)
    {
      if (input[i] == 'q') 
      {
        printf("Goodbye!\n");
        return;
      }

      switch (input[i]) {
       case 'n':
          {
            char *buff;
            char *filename = filepath;
            
            if(filename[0] == '/') // if the given path is the root directory it will start with a ''
              filename++;
            
            while((buff = strchr(filename, '/')) != NULL)
              filename = buff + 1;
            printf("File name: %s\n", filename);
            break;
          }
       case 'd':
          {
            if((stat(filepath, &filestat)) < 0)
            {
              perror("fstat");
              return;
            }

            printf("File size: %ld\n", filestat.st_size);
            break;
          }
      case 'h':
        { 
          if((stat(filepath, &filestat)) < 0)
          {
            perror("fstat");
            return;
          }
          
          printf("Number of hard links: %ld\n", filestat.st_nlink);
          break;
        }
      case 'm':
        {
          if((stat(filepath, &filestat)) < 0)
          {
            perror("fstat");
            return;
          }

          printf("Time of last modification: %s", ctime(&filestat.st_mtime));
          break;
        }
      case 'a':
        { 
          if((stat(filepath, &filestat)) < 0)
          {
            perror("fstat");
            return;
          }
          
          printf((filestat.st_mode & S_IRUSR) ? "r" : "-");
          printf((filestat.st_mode & S_IWUSR) ? "w" : "-");
          printf((filestat.st_mode & S_IXUSR) ? "x" : "-");
          printf((filestat.st_mode & S_IRGRP) ? "r" : "-");
          printf((filestat.st_mode & S_IWGRP) ? "w" : "-");
          printf((filestat.st_mode & S_IXGRP) ? "x" : "-");
          printf((filestat.st_mode & S_IROTH) ? "r" : "-");
          printf((filestat.st_mode & S_IWOTH) ? "w" : "-");
          printf((filestat.st_mode & S_IXOTH) ? "x\n" : "-\n");
          break;
        }
      case 'l':
        {
          char *linkname;
          printf("Enter the name of the symbolic link you want to create:\n");
          linkname = read_stdin();
          
          if(symlink(filepath, linkname) < 0)
          {
            perror("fstat");
            return;
          }

          printf("You created the symbolic link '%s' for file '%s'\n", linkname, filepath);
          break;
        }
      default:
        printf("Invalid choice!\n");
        break;
      }  
    }
  }
}
// void sl_menu(char *filename
// {
//   char input;
//
//   do {
//     printf("Symbolic Link Menu:\n\n");
//
//     printf("-n Link name:\n");
//     printf("-l ")
//   }
// }
//
int main(int argc, char **argv)
{
  struct stat filestat;

  for(int i = 1; i < argc; i++)
  {
    if(lstat(argv[i], &filestat) < 0)
    {
      printf("Error: Cannot lstat the file %s\n", argv[i]);
      continue;
    }

    if(S_ISREG(filestat.st_mode))
    {
      rf_menu(argv[i]);
    }

    if(S_ISLNK(filestat.st_mode))
    {
      printf("Link: %s", argv[i]);
      //sl_menu(argv[i]);
    }
    if(S_ISDIR(filestat.st_mode))
    {
      printf("Directory: %s", argv[i]);
    }

  }

  return 0;
}
