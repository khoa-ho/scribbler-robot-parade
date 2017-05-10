#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <MyroC.h>
#include <time.h>
#include <eSpeakPackage.h>

#define MAX_LINE 100
#define STR_LEN 20

//===================================================================
//Helper functions
/** Initializes eSpeak and connects the robot */
void initializeRobot(void) {
  rConnect("/dev/rfcomm0");
  eSpeakConnect();
  rSetForwardnessTxt("scribbler-forward");
}

/** Cleans up scribbler, eSpeak, and closes file */
void quit(FILE *fp, FILE *log) {
  rStop();
  rDisconnect();
  eSpeakDisconnect();
  fclose(fp);
  if(log) { fclose(log); }
}

void rDitty(void){

}

void rSong(void){

}

/** Removes the last character in a string if it is a newline */
char *rmChar(char *str) {
  if(str[strlen(str) - 1] == '\n') {
    str[strlen(str) - 1] = '\0';
  }
  return str;
}

void logit(FILE *fp, char *arg){
  time_t curtime;
  int left, right, mid, avg;

  if(fp) {
    left = rGetLightTxt("left", 3);
    mid = rGetLightTxt("middle", 3);
    right = rGetLightTxt("right", 3);
    avg = (left + right + mid) / 3;
    time(&curtime);
    fprintf(fp, "%s  %s (%d, %d, %d)\n", rmChar(ctime(&curtime)), arg, left, right, avg);
  }
}

/** Checks whether the input stream contains a specific command */
bool isCmd(char str[], char cmd[]) {
  return (strncmp(str, cmd, strlen(cmd)) == 0);
}


//===================================================================
bool isType(char *str, char type) {
  int i = 0;
  
  if (*str == '\0') {
    printf("No argument given. Enter a proper argument\n");
    return false;
  } else if (type == 'd') {
    while (str[i] != '\0') {
      if (!isdigit(str[i]) && str[i] != '\n') {
        // If character is not a numeral positive
        printf("%s not an integer\n", str);
        return false;
      }
      i++;
    }
  } else if (type == 'f') {
    while (str[i] != '\0') {
      if (!isdigit(str[i]) && str[i] != '.' && str[i] != '\n') {
        // If character is not a numeral or decimal
        printf("%s not an integer or float\n", str);
        return false;
      }
      i++;
    }
  }
  return true;
}

int main(int argc, char **argv) {
  FILE *fp, *log = NULL;
  
  if (isCmd(argv[1], "-log")) {
    log = fopen(argv[2], "w");
    fp = fopen(argv[3],"r");
    if(!fp) {
      perror("File opening failed");
      return EXIT_FAILURE;
    }
  } else {
    fp = fopen(argv[1],"r");
    if(!fp) {
      perror("File opening failed");
      return EXIT_FAILURE;
    }
  }
    
  // Step 1
  initializeRobot();

  // Step 2, 3, 4
  printf("Press 'Enter' to start: ");
  while (getchar() != '\n') {}
  rMotors(0.25, 0.25);
  printf("Press 'Enter' to execute file: ");
  while (getchar() != '\n') {}
  rStop();

  // Step 5
  char *token;
  char buf[MAX_LINE];
  char *tokens[3];
  
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    token = strtok(buf, " ");
    int i = 0;
    while((token != NULL) && (i < 3)) {
      tokens[i++] = rmChar(token);
      token = strtok(NULL, " ");
    }
    
    if (isCmd(tokens[0], "forward")) {
      if(isType(tokens[1], 'f')){
        rForward(1.0, atof(tokens[1]));
        logit(log, tokens[0]);
      } else {
        quit(fp, log);
        return 0;
      }
    } else if (isCmd(tokens[0], "turn")) {
      if(isType(tokens[1], 'd')){
        if(isCmd(tokens[2], "left")) {
          rTurnLeft(1.0, atof(tokens[1]) / 45);
          logit(log, tokens[0]);
        } else if(isCmd(tokens[2], "right")) {
          rTurnRight(1.0, atof(tokens[1]) / 45);
          logit(log, tokens[0]);
        } else {
          printf("Not given correct direction\n");
          quit(fp, log);
          return 0;
        }
      } else {
        quit(fp, log);
        return 0;
      }
    } else if (isCmd(tokens[0], "spin")) {
      if(isCmd(tokens[1], "left")) {
        rTurnLeft(1.0, 4.0);
        logit(log, tokens[0]);
      } else if(isCmd(tokens[1], "right")) {
        rTurnRight(1.0, 4.0);
        logit(log, tokens[0]);
      } else {
        printf("Not given correct direction\n");
        quit(fp, log);
        return 0;
      }
    } else if (isCmd(tokens[0], "beep")) {
      rBeep(1.0, 698);
      logit(log, tokens[0]);
    } else if (isCmd(tokens[0], "ditty")) {
      rDitty();
      logit(log, tokens[0]);
    } else if (isCmd(tokens[0], "song")) {
      rSong();
      logit(log, tokens[0]);
    } else {
      printf("Invalid command\n");
      quit(fp, log);
      return 0;
    }
  }
  if (feof(fp)) { 
    printf("End of file reached\n");
  }
  
  // Step 6
  rForward(1.0,2.0);
  quit(fp, log);
  
  return 0;
}
  
