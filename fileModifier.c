#include <linux/limits.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>

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

void count_c_files(const char* dirpath, int *count)
{
  struct stat dirstat;
  struct dirent *ent;
  DIR *dir;

  if((stat(dirpath, &dirstat)) < 0)
  {
    perror("fstat");
    return;
  }

  if((dir = opendir(dirpath)) != NULL)
  {
    while((ent = readdir(dir)) != NULL)
    {
      const char *extension = strchr(ent->d_name, '.');

      if(ent->d_type == DT_REG)
      {
        if((!extension) || (extension == ent->d_name))
          return;
        else 
        {
          if(strcmp(extension, ".c") == 0)
          {
            printf("%s\n", ent->d_name);
            (*count)++;
          }
        }
      }
      else if(ent->d_type == DT_DIR)
      {
        if(strcmp(ent->d_name,".") != 0 && strcmp(ent->d_name, "..") != 0)
        {
          char subdirpath[PATH_MAX];
          snprintf(subdirpath, PATH_MAX, "%s/%s", dirpath, ent->d_name);
          count_c_files(subdirpath, count);
        }
      }
    }
                
    closedir(dir);
  }
  else 
  {
    perror("Cannot open dir!\n");
    exit(1);
  }
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
          
          printf("User:\n"); 
          printf((filestat.st_mode & S_IRUSR) ? "Read - yes\n" : "Read - no \n");
          printf((filestat.st_mode & S_IWUSR) ? "Write - yes\n" : "Write - no\n");
          printf((filestat.st_mode & S_IXUSR) ? "Exec - yes\n" : "Exec - no\n");
          printf("Group:\n");
          printf((filestat.st_mode & S_IRGRP) ? "Read - yes\n" : "Read - no\n");
          printf((filestat.st_mode & S_IWGRP) ? "Write - yes\n" : "Write - no\n");
          printf((filestat.st_mode & S_IXGRP) ? "Exec - yes\n" : "Exec - no\n");
          printf("Other:\n");
          printf((filestat.st_mode & S_IROTH) ? "Read - yes\n" : "Read - no\n");
          printf((filestat.st_mode & S_IWOTH) ? "Write - yes\n" : "Write - no\n");
          printf((filestat.st_mode & S_IXOTH) ? "Exec - yes\n" : "Exec - no\n");          
          break;
        }
      case 'l':
        {
          char *linkname;
          printf("Enter the name of the symbolic link you want to create:\n");
          linkname = read_stdin();
          
          if(unlink(linkname) < 0 && errno != ENOENT)
          {
            perror("unlink");
            return;
          }

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

void sl_menu(char *linkpath)
{
  char *input;
  struct stat linkstat;
  struct stat targetstat;
  printf("Symbolic Link menu:\n\n");

  printf("-n Link name\n");
  printf("-l Delete link\n");
  printf("-d Link size\n");
  printf("-t Target size\n");
  printf("-a Acces rights\n");
  printf("-q Exit\n");
  
  while(1)
  {
    printf("Choose an optionn:\n");
    input = read_stdin();
    
    for(int i = 1; input[i] != '\0'; i++)
    {
      if (input[i] == 'q') 
      {
        printf("Goodbye!\n");
        return;
      }
      
      switch(input[i]) {
        case 'n':
          {
            char *buff;
            char *linkname = linkpath;
            
            if(linkname[0] == '/') // if the given path is the root directory it will start with a '/'
              linkname++;
            
            while((buff = strchr(linkname, '/')) != NULL)
              linkname = buff + 1;
            printf("File name: %s\n", linkname);

            break;
          }
        case 'l':
          {
            if(unlink(linkpath) < 0)
            {
              perror("fstat");
              break;
            }

            printf("Deleted symbolic link '%s'\n", linkpath);
            break;
          }
        case 'd':
          {
            if((lstat(linkpath, &linkstat)) < 0)
            {
              perror("fstat");
              return;
            }

            printf("Link size: %ld\n", linkstat.st_size);
            break;
          }
        case 't':
          {
            if((stat(linkpath, &targetstat)) < 0)
            {
              perror("fstat");
              return;
            }

            printf("Target size: %ld\n", targetstat.st_size);
            break;
          }
        case 'a':
          {
            if((lstat(linkpath, &linkstat)) < 0)
            {
              perror("fstat");
              return;
            }
            printf("User\n"); 
            printf((linkstat.st_mode & S_IRUSR) ? "Read - yes\n" : "Read - no\n");
            printf((linkstat.st_mode & S_IWUSR) ? "Write - yes\n" : "Write - no\n");
            printf((linkstat.st_mode & S_IXUSR) ? "Execute - yes\n" : "Execute - no\n");
            printf("Group\n");
            printf((linkstat.st_mode & S_IRGRP) ? "Read - yes\n" : "Read - no\n");
            printf((linkstat.st_mode & S_IWGRP) ? "Write - yes\n" : "Write - no\n");
            printf((linkstat.st_mode & S_IXGRP) ? "Execute - yes\n" : "Execute - no\n");
            printf("Other\n");
            printf((linkstat.st_mode & S_IROTH) ? "Read - yes\n" : "Read - no\n");
            printf((linkstat.st_mode & S_IWOTH) ? "Write - yes\n" : "Write - no\n");
            printf((linkstat.st_mode & S_IXOTH) ? "Execute - yes\n" : "Execute - no\n");
                        
            break;
          }
        default:
          printf("Invalid option!\n");
          break;
      }
    }
  }
}

void dir_menu(char *dirpath)
{
  char *input;
  DIR *dir;
  struct stat dirstat;
  struct dirent *ent;
  
  printf("Directory menu:\n\n");
  printf("-n Directory name\n");
  printf("-d Directory size\n");
  printf("-a Acces rights\n");
  printf("-c Total number of files that end with the '.c' extension\n");
  printf("-q Quit!\n");

  while(1)
  {
    printf("Choose an option:\n");
    input = read_stdin();

    for(int i = 1; input[i] != '\0'; i++)
    {
      if(input[i] == 'q')
      {
        printf("Goodbye!\n");
        return;
      }

      switch(input[i])
      {
        case 'n':
          {
            char *buff;
            char *dirname = dirpath;
            if(dirname[strlen(dirname) - 1] == '/')
              dirname[strlen(dirname) - 1] = '\0';

            if(dirname[0] == '/') // if the given path is the root directory it will start with a '/'
              dirname++;
            
            while((buff = strchr(dirname, '/')) != NULL )
              dirname = buff + 1;
            printf("Directory name: %s\n", dirname);

            break;
          }
        case 'd':
          {
            if((stat(dirpath, &dirstat)) < 0)
            {
              perror("fstat");
              return;
            }

            printf("Directory size: %ld\n", dirstat.st_size);
            break;
          }
        case 'a':
          {
            if((stat(dirpath, &dirstat)) < 0)
            {
              perror("fstat");
              return;
            }

            printf("User:\n"); 
            printf((dirstat.st_mode & S_IRUSR) ? "Read - yes\n" : "Read - no \n");
            printf((dirstat.st_mode & S_IWUSR) ? "Write - yes\n" : "Write - no\n");
            printf((dirstat.st_mode & S_IXUSR) ? "Exec - yes\n" : "Exec - no\n");
            printf("Group:\n");
            printf((dirstat.st_mode & S_IRGRP) ? "Read - yes\n" : "Read - no\n");
            printf((dirstat.st_mode & S_IWGRP) ? "Write - yes\n" : "Write - no\n");
            printf((dirstat.st_mode & S_IXGRP) ? "Exec - yes\n" : "Exec - no\n");
            printf("Other:\n");
            printf((dirstat.st_mode & S_IROTH) ? "Read - yes\n" : "Read - no\n");
            printf((dirstat.st_mode & S_IWOTH) ? "Write - yes\n" : "Write - no\n");
            printf((dirstat.st_mode & S_IXOTH) ? "Exec - yes\n" : "Exec - no\n");
            break;
          }
        case 'c':
          {
            int count = 0;
            count_c_files(dirpath, &count);
            printf("%d\n", count);
          }
      }
    }
  }
}

int main(int argc, char **argv)
{
  struct stat filestat;
  pid_t pid, pid1, pid2;
  pid_t pid1_id, ppid1_id;
  for(int i = 1; i < argc; i++)
  {
    if(lstat(argv[i], &filestat) < 0)
    {
      printf("Error: Cannot lstat the file %s\n", argv[i]);
      continue;
    }
      if((pid1 = fork()) < 0)
      {
        printf("Failed to create 'pid1' child process!\n");
        exit(1);
      }   
      if(pid1 == 0)
      {
        pid1_id = getpid();
        ppid1_id = getppid();
        printf("This is a child process: PPID: %d, PID: %d\n", ppid1_id, pid1_id);
        char* arguments[] = {"bash", "compile_c.sh", argv[i], "f1.txt", NULL};   
        printf("Executing script 'compile_c.sh'\n\n");
        if(execv("/usr/bin/bash", arguments) == -1)
        {
          perror("execv");
          exit(EXIT_FAILURE);
        }
        exit(0);
      }
      // else 
      // {
      //   wait(NULL);            
      // }
      //
    if((pid = fork()) < 0)
    {
      printf("Failed to create child process!\n");
      exit(1);
    }

    if(pid == 0)
    {
      if(S_ISREG(filestat.st_mode))
      {
        char *buff;
        char *filename = argv[i];
     
        if(filename[0] == '/') // if the given path is the root directory it will start with a '/'
          filename++;
            
        while((buff = strchr(filename, '/')) != NULL)
          filename = buff + 1;
        printf("%s - Regular file\n\n", filename);
        
        // if(filename[strlen(filename) - 2] == '.' && filename[strlen(filename) - 1] == 'c')
        // {
        //   if((pid1 = fork()) < 0)
        //   {
        //     printf("Failed to create 'pid1' child process!\n");
        //     exit(1);
        //   }
        //   
        //   if(pid1 == 0)
        //   {
        //     char* arguments[] = {"bash", "compile_c.sh", argv[i], "f1.txt", NULL};
        //   
        //     printf("This is a .c file. Executing script 'compile_c.sh'\n\n");
        //     if(execv("/usr/bin/bash", arguments) == -1)
        //     {
        //       perror("execv");
        //       exit(EXIT_FAILURE);
        //     }
        //     exit(0);
        //   }
        //   else 
        //   {
        //     wait(NULL);            
        //     printf("Done! Sending you to regular file menu!\n\n");
        //   }
        // }
        //
        rf_menu(argv[i]);
      }

      if(S_ISLNK(filestat.st_mode))
      {
        char *buff;
        char *linkname = argv[i];
            
        if(linkname[0] == '/') // if the given path is the root directory it will start with a '/'
          linkname++;
            
        while((buff = strchr(linkname, '/')) != NULL)
          linkname = buff + 1;
        printf("%s - Link\n\n", linkname);

        sl_menu(argv[i]);
      }

      if(S_ISDIR(filestat.st_mode))
      {
        char *buff;
        char *dirname = argv[i];

        if(dirname[strlen(dirname) - 1] == '/')
          dirname[strlen(dirname) - 1] = '\0';

        if(dirname[0] == '/') // if the given path is the root directory it will start with a '/'
          dirname++;
            
        while((buff = strchr(dirname, '/')) != NULL )
          dirname = buff + 1;
        printf("%s - Directory\n\n", dirname);
        
        if((pid2 = fork()) < 0)
        {
          printf("Failed to create 'pid2' child process\n");
          exit(1);
        }
        
        if(pid2 == 0)
        {
          char* dirname_file = (char *) malloc(sizeof(char) * (strlen(dirname) + 10));
          strcpy(dirname_file, dirname);
          strcat(dirname_file, "_file.txt");
          char* arguments[] = {"touch", dirname_file, NULL};

          printf("Creating a text file with the name '%s_file.txt'\n", dirname);

          if(execv("/usr/bin/touch", arguments) == -1)
          {
            perror("execv");
            exit(EXIT_FAILURE);
          }
          exit(0);
        }
        else 
        {
          wait(NULL);
          printf("Done! Sending you to directory menu.\n\n");
        }

        dir_menu(argv[i]);
      }
      exit(0);
    }
 wait(&pid1_id); 

      wait(NULL);    
     }
  return 0;
}
