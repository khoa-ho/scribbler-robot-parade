#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <MyroC.h>
#include <time.h>
#include <eSpeakPackage.h>


// ============================================================================
/** Defines input string length */
#define STR_LEN 20

#define MAX_LINE 100

/** Defines musical notes */
#define B6 1975.53
#define G6 1567.98
#define C7 2093.00
#define D7flat 2217.46
#define D7 2349.32
#define E7 2637.02
#define REST 1.0

/** Defines note lengths */
#define BEATS_PER_MIN (600)
#define SECONDS_PER_BEAT (60.0/BEATS_PER_MIN)
double dur1() {return SECONDS_PER_BEAT / 2.0;}
double dur4() {return SECONDS_PER_BEAT / 2.0;}
double dur8() {return SECONDS_PER_BEAT * 4.0;}

// ============================================================================
// Helper functions
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
  if (log) { fclose(log); }
}

/** Plays a tune for 1 second */ 
void rDitty(void){
  rBeep (dur8(), D7flat);
  rBeep (dur8(), G6);
  rBeep (dur4(), C7);
}

/** Plays a tune for 3 seconds */
void rSong(void){
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), B6);
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), C7);
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), D7flat);
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), D7);
  rBeep (dur1(), E7);
  rBeep (dur1(), G6);
  rBeep (dur1(), D7);
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), B6);
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur8(), REST); 
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), B6);
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), C7);
  rBeep (dur8(), REST); 
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), B6);
  rBeep (dur1(), G6);
  rBeep (dur1(), G6);
  rBeep (dur1(), C7);
  rBeep (dur4(), G6);
  rBeep (dur4(), D7flat);
  rBeep (dur4(), G6);
}

/** Removes the last character in a string if it is a newline */
char *rmChar(char *str) {
  if (str[strlen(str) - 1] == '\n') {
    str[strlen(str) - 1] = '\0';
  }
  return str;
}

/** Logs the time, the command, and the values of IR sensors */
void logIt(FILE *fp, char *arg) {
  time_t curtime;
  int left, right, mid, avg;

  if (fp) {
    left = rGetLightTxt("left", 3);
    mid = rGetLightTxt("middle", 3);
    right = rGetLightTxt("right", 3);
    avg = (left + right + mid) / 3;
    time(&curtime);
    fprintf(fp, "%s %s (%d, %d, %d)\n", rmChar(ctime(&curtime)), arg, left, right, avg);
  }
}

/** Checks whether the input stream contains a specific command */
bool isCmd(char str[], char cmd[]) {
  return (strncmp(str, cmd, strlen(cmd)) == 0);
}

// ============================================================================
/** Checks whether the input is of correct type */
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

// ============================================================================
// Main function
int main(int argc, char **argv) {
  // Open files
  FILE *fp, *log = NULL;
  char *token, *tokens[3], buf[MAX_LINE];
  
  if (isCmd(argv[1], "-log")) {
    log = fopen(argv[2], "w");
    fp = fopen(argv[3],"r");
    if (!fp) {
      perror("File opening failed");
      return EXIT_FAILURE;
    }
  } else {
    fp = fopen(argv[1],"r");
    if (!fp) {
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
  
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    token = strtok(buf, " ");
    int i = 0;
    while((token != NULL) && (i < 3)) {
      tokens[i++] = rmChar(token);
      token = strtok(NULL, " ");
    }
    
    if (isCmd(tokens[0], "forward")) {
      if (isType(tokens[1], 'f')){
        rForward(1.0, atof(tokens[1]));
        logIt(log, tokens[0]);
      } else {
        quit(fp, log);
        return 0;
      }
    } else if (isCmd(tokens[0], "turn")) {
      if (isType(tokens[1], 'd')){
        if (isCmd(tokens[2], "left")) {
          rTurnLeft(1.0, atof(tokens[1]) * 2 / 225);
          logIt(log, tokens[0]);
        } else if (isCmd(tokens[2], "right")) {
          rTurnRight(1.0, atof(tokens[1]) * 2 / 225);
          logIt(log, tokens[0]);
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
      if (isCmd(tokens[1], "left")) {
        rTurnLeft(1.0, 3.2);
        logIt(log, tokens[0]);
      } else if (isCmd(tokens[1], "right")) {
        rTurnRight(1.0, 3.2);
        logIt(log, tokens[0]);
      } else {
        printf("Not given correct direction\n");
        quit(fp, log);
        return 0;
      }
    } else if (isCmd(tokens[0], "beep")) {
      rBeep(1.0, 698);
      logIt(log, tokens[0]);
    } else if (isCmd(tokens[0], "ditty")) {
      rDitty();
      logIt(log, tokens[0]);
    } else if (isCmd(tokens[0], "song")) {
      rSong();
      logIt(log, tokens[0]);
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
