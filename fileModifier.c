#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void rf_menu(char *filname)
{
  char input;

  do {
    printf("Regular File Menu:\n\n");

    printf("-n File name\n");
    printf("-d File size\n");
    printf("-h Number of hardlinks\n");
    printf("-m Time of last modification\n");
    printf("-a Acces rights\n");
    printf("-l Create a symbolic link\n");

    switch (input) {
      case 'n':
        // do n
        break; 
      case 'd':
        //do d
        break;
      case 'h':
        //do h
        break;
      case 'm':
        // do m
        break;
      case 'a':
        // do a
        break;
      case 'l':
        // do l
        break;
      case 'q':
        printf("Goodbye!\n");
        break;
      default:
        printf("Invalid choice!\n");
        break;
    }
  }while (input != 'q');
}

// void sl_menu(char *filename)
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
    if(lstat(argv[i]), &filestat) < 0)
    {
      printf("Error: Cannot lstat the file %s\n", argv[i]);
      continue;
    }

    if(S_ISREG(filstat.st_mode))
    {
      rf_menu(argv[i]);
    }

    if(S_ISLINK(filestat.st_mode))
    {
      sl_menu(argv[i]);
    }
    if(S_ISDIR(filestat.st_mode))
    {
      printf("Directory: %s", argv[i]);
    }

  }

  return 0;
}
