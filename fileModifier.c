#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

char *get_name_from_path(char *path) {
  char *buff;
  char *name = path;

  // if the path ends with a '/'.
  // only used for directories
  if (name[strlen(name) - 1] == '/')
    name[strlen(name) - 1] = '\0';

  // if the given path is the root directory it
  // will start with a '/'
  if (name[0] == '/')
    name++;

  while ((buff = strchr(name, '/')) != NULL)
    name = buff + 1;

  return name;
}

static char *read_stdin(void) {
  size_t cap = 4096, len = 0;
  char *buffer = malloc(cap * sizeof(char));
  int c;

  while ((c = fgetc(stdin)) != '\n' && !feof(stdin)) {
    buffer[len] = c;

    if (++len == cap)
      buffer = realloc(buffer, (cap *= 2) * sizeof(char));
  }

  buffer = realloc(buffer, (len + 1) * sizeof(char));
  buffer[len] = '\0';

  return buffer;
}

void count_c_files(const char *dirpath, int *count) {
  struct stat dirstat;
  struct dirent *ent;
  DIR *dir;

  if ((stat(dirpath, &dirstat)) < 0) {
    perror("fstat");
    return;
  }

  if ((dir = opendir(dirpath)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      const char *extension = strchr(ent->d_name, '.');

      if (ent->d_type == DT_REG) {
        if ((!extension) || (extension == ent->d_name))
          return;
        else {
          if (strcmp(extension, ".c") == 0) {
            // printf("%s\n", ent->d_name);
            (*count)++;
          }
        }
      } else if (ent->d_type == DT_DIR) {
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
          char subdirpath[PATH_MAX];
          snprintf(subdirpath, PATH_MAX, "%s/%s", dirpath, ent->d_name);
          count_c_files(subdirpath, count);
        }
      }
    }
    closedir(dir);
  } else {
    perror("Cannot open dir!\n");
    exit(1);
  }
}

void rf_menu(char *filepath) {
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

  while (1) {
    printf("Chose an option:\n");

    input = read_stdin();

    for (int i = 1; input[i] != '\0'; i++) {
      if (input[i] == 'q') {
        printf("Goodbye!\n");
        return;
      }

      switch (input[i]) {
      case 'n': {
        printf("File name: %s\n", get_name_from_path(filepath));
        break;
      }
      case 'd': {
        if ((stat(filepath, &filestat)) < 0) {
          perror("fstat");
          return;
        }

        printf("File size: %ld\n", filestat.st_size);
        break;
      }
      case 'h': {
        if ((stat(filepath, &filestat)) < 0) {
          perror("fstat");
          return;
        }

        printf("Number of hard links: %ld\n", filestat.st_nlink);
        break;
      }
      case 'm': {
        if ((stat(filepath, &filestat)) < 0) {
          perror("fstat");
          return;
        }

        printf("Time of last modification: %s", ctime(&filestat.st_mtime));
        break;
      }
      case 'a': {
        if ((stat(filepath, &filestat)) < 0) {
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
      case 'l': {
        char *linkname;
        printf("Enter the name of the symbolic link you want to create:\n");
        linkname = read_stdin();

        if (unlink(linkname) < 0 && errno != ENOENT) {
          perror("unlink");
          return;
        }

        if (symlink(filepath, linkname) < 0) {
          perror("fstat");
          return;
        }

        printf("You created the symbolic link '%s' for file '%s'\n", linkname,
               filepath);
        break;
      }
      default:
        printf("Invalid option!\n");
        break;
      }
    }
  }
}

void sl_menu(char *linkpath) {
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

  while (1) {
    printf("Choose an optionn:\n");
    input = read_stdin();

    for (int i = 1; input[i] != '\0'; i++) {
      if (input[i] == 'q') {
        printf("Goodbye!\n");
        return;
      }

      switch (input[i]) {
      case 'n': {
        printf("Link name: %s\n", get_name_from_path(linkpath));
        break;
      }
      case 'l': {
        if (unlink(linkpath) < 0) {
          perror("fstat");
          break;
        }

        printf("Deleted symbolic link '%s'\n", linkpath);
        break;
      }
      case 'd': {
        if ((lstat(linkpath, &linkstat)) < 0) {
          perror("fstat");
          return;
        }

        printf("Link size: %ld\n", linkstat.st_size);
        break;
      }
      case 't': {
        if ((stat(linkpath, &targetstat)) < 0) {
          perror("fstat");
          return;
        }

        printf("Target size: %ld\n", targetstat.st_size);
        break;
      }
      case 'a': {
        if ((lstat(linkpath, &linkstat)) < 0) {
          perror("fstat");
          return;
        }
        printf("User\n");
        printf((linkstat.st_mode & S_IRUSR) ? "Read - yes\n" : "Read - no\n");
        printf((linkstat.st_mode & S_IWUSR) ? "Write - yes\n" : "Write - no\n");
        printf((linkstat.st_mode & S_IXUSR) ? "Execute - yes\n"
                                            : "Execute - no\n");
        printf("Group\n");
        printf((linkstat.st_mode & S_IRGRP) ? "Read - yes\n" : "Read - no\n");
        printf((linkstat.st_mode & S_IWGRP) ? "Write - yes\n" : "Write - no\n");
        printf((linkstat.st_mode & S_IXGRP) ? "Execute - yes\n"
                                            : "Execute - no\n");
        printf("Other\n");
        printf((linkstat.st_mode & S_IROTH) ? "Read - yes\n" : "Read - no\n");
        printf((linkstat.st_mode & S_IWOTH) ? "Write - yes\n" : "Write - no\n");
        printf((linkstat.st_mode & S_IXOTH) ? "Execute - yes\n"
                                            : "Execute - no\n");

        break;
      }
      default:
        printf("Invalid option!\n");
        break;
      }
    }
  }
}

void dir_menu(char *dirpath) {
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

  while (1) {
    printf("Choose an option:\n");
    input = read_stdin();

    for (int i = 1; input[i] != '\0'; i++) {
      if (input[i] == 'q') {
        printf("Goodbye!\n");
        return;
      }

      switch (input[i]) {
      case 'n': {
        printf("Directory name: %s\n", get_name_from_path(dirpath));
        break;
      }
      case 'd': {
        if ((stat(dirpath, &dirstat)) < 0) {
          perror("fstat");
          return;
        }

        printf("Directory size: %ld\n", dirstat.st_size);
        break;
      }
      case 'a': {
        if ((stat(dirpath, &dirstat)) < 0) {
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
      case 'c': {
        int count = 0;
        count_c_files(dirpath, &count);
        printf("Total numbers of C files: %d\n", count);
        break;
      }
      default:
        printf("Invalid option!\n");
        break;
      }
    }
  }
}

void count_errors_and_warnings(char *file, int pipe_fd) {
  char *arguments[] = {"bash", "compile_c.sh", file, "f1.txt", NULL};
  
  dup2(pipe_fd, STDOUT_FILENO);
  close(pipe_fd);
  printf("Executing script 'compile_c.sh'\n\n");
  fflush(stdout);
    
  if (execv("/usr/bin/bash", arguments) == -1) {
    perror("execv");
    exit(EXIT_FAILURE);
  }
}


void create_file_for_directory(char *directory, int pipe_fd) {
  char *dirname = get_name_from_path(directory);
  char *dirname_file = (char *)malloc(sizeof(char) * (strlen(dirname) + 10));

  strcpy(dirname_file, dirname);
  strcat(dirname_file, "_file.txt");
  char *arguments[] = {"touch", dirname_file, NULL};

  dup2(pipe_fd, STDOUT_FILENO);
  close(pipe_fd);
  printf("Creating a text file with the name '%s_file.txt'\n", dirname);
  fflush(stdout);

  if (execv("/usr/bin/touch", arguments) == -1) {
    perror("execv");
    exit(EXIT_FAILURE);
  }
}

int compute_score(int errors, int warnings) {
  int score;

  if (errors == 0 && warnings == 0)
    score = 10;
  else if (errors >= 1)
    score = 1;
  else if (errors == 0 && warnings > 10)
    score = 2;
  else if (errors == 0 && warnings <= 10)
    score = 2 + 8 * (10 - warnings) / 10;

  return score;
}

void print_no_of_lines(char *file, int pipe_fd) {
  char *arguments[] = {"grep", "-c", "^", file, NULL};
  
  dup2(pipe_fd, STDOUT_FILENO);
  close(pipe_fd);
  printf("Number of lines: ");
  fflush(stdout);
 
  if(execv("/usr/bin/grep", arguments) == -1) {
    perror("execv");
    exit(EXIT_FAILURE);
  }
}

void change_permission(char *link, int pipe_fd) {
  char *arguments[] = {"chmod", "-v", "760", link, NULL};
  
  dup2(pipe_fd, STDOUT_FILENO);
  close(pipe_fd);
  printf("Changing permissions to 'rwxrw----'\n");
  fflush(stdout);

  if(execv("/usr/bin/chmod", arguments) == -1) {
    perror("execv");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv) {
  struct stat filestat;
  pid_t pid1, pid2;
  int fd1[2], fd2[2], score_fd[2];
  char buff1[4096], buff2[4096], score_buff[4096];
  int status1, status2;

  for (int i = 1; i < argc; i++) {
    if (lstat(argv[i], &filestat) < 0) {
      printf("Error: Cannot lstat the file %s\n", argv[i]);
      continue;
    }
    
    if (pipe(fd1) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }
    
    if(pipe(fd2) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
    }
    
    if ((pid1 = fork()) < 0) {
      printf("Failed to create the first child process!\n");
      exit(1);
    }

    if (pid1 == 0) {
        if (S_ISREG(filestat.st_mode)) {
        printf("%s - Regular file\n\n", get_name_from_path(argv[i]));
        rf_menu(argv[i]);
      }

      if (S_ISLNK(filestat.st_mode)) {
        printf("%s - Link\n\n", get_name_from_path(argv[i]));
        sl_menu(argv[i]);
      }

      if (S_ISDIR(filestat.st_mode)) {
        printf("%s - Directory\n\n", get_name_from_path(argv[i]));
        dir_menu(argv[i]);
      }
      exit(0);
    } 
    else if (pid1 > 0) {
      if ((pid2 = fork()) < 0) {
        printf("Failed to create the second child process!\n");
        exit(1);
      }
      
      if(pid2 == 0) {
        if(S_ISREG(filestat.st_mode)) {
          char *name = get_name_from_path(argv[i]);
          char *extension = strchr(name, '.');
 
          if (strcmp(extension, ".c") == 0) {
            close(fd1[0]); // close the read end of the pipe of child process
            count_errors_and_warnings(argv[i], fd1[1]);
          }
          else {
            close(fd2[0]);
            print_no_of_lines(argv[i], fd2[1]);
          }
        }

        if(S_ISDIR(filestat.st_mode)) {
          close(fd2[0]);
          create_file_for_directory(argv[i], fd2[1]);
        }

        if(S_ISLNK(filestat.st_mode)) {
          close(fd2[0]);
          change_permission(argv[i], fd2[1]); 
        }
        exit(0);
      }
      else if(pid2 > 0) {
        waitpid(pid1, &status1, 0);
        waitpid(pid2, &status2, 0);

        close(fd1[1]);
        close(fd2[1]);
       
        int n1, n2;
        n1 = read(fd1[0], buff1, sizeof(score_buff));
        buff1[n1] = '\0';
        n2 = read(fd2[0], buff2, sizeof(buff2));
        buff2[n2] = '\0';

        if(S_ISREG(filestat.st_mode)) {
          char *name = get_name_from_path(argv[i]);
          char *extension = strchr(name, '.');
          FILE *file = fopen("grades.txt", "a+");

          if (strcmp(extension, ".c") == 0) {
            write(STDOUT_FILENO, buff1, n1);
            int errors, warnings;
            sscanf(buff1, "Executing script 'compile_c.sh'\n\nErrors: %d Warnings: %d", &errors, &warnings);
            int score = compute_score(errors, warnings);  
            
            fprintf(file, "%s:%d\n", name, score);
            printf("The score was printed in the file 'grades.txt'\n");
            
            fclose(file);
          }
        }

        write(STDOUT_FILENO, buff2, n2);
                       
        printf("The process with PID %d has ended with the exit code %d\n", pid1, status1);
        printf("The process with PID %d has ended with the exit code %d\n", pid2, status2);
      }
    }
  }
  return 0;
}
