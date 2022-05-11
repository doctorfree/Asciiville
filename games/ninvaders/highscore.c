/**
 * nInvaders - a space invaders clone for ncurses
 * Copyright (C) 2002-2003 Dettus
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * homepage: http://ninvaders.sourceforge.net
 * mailto: ninvaders-devel@lists.sourceforge.net
 *
 */

#include "highscore.h"
#include <stdio.h>
#include <string.h>

/**************************************************************************************************
 *
 * private constants, variables, structures, declarations, macros
 *
 **************************************************************************************************/

#define HIGHSCORE_FILE "highscore" /* filename of highscore             */
#define HIGHSCORE_ID                                                           \
  "nInvaders Highscore"         /* header for highscore file         */
#define HIGHSCORE_VERSION "0.1" /* version number for highscore file */

HighScore highscore; /* todo: not a global variable! */

/**************************************************************************************************
 *
 * getStandardHighScore
 *
 * returns a HighScore object initialized with standard values
 *
 **************************************************************************************************/
HighScore getStandardHighScore() {

  HighScore hs;
  HighScoreEntry *hs_b, *hs_n, *hs_e;
  int n;

  hs_b = hs.beginner;
  hs_n = hs.normal;
  hs_e = hs.expert;

  for (n = MAX_HIGHSCORE_ENTRIES; n > 0; n--) {

    hs_b->score = n * 1000;
    strcpy(hs_b->name, "hollinge");
    hs_b++;

    hs_n->score = n * 1000;
    strcpy(hs_n->name, "sen_hoss");
    hs_n++;

    hs_e->score = n * 1000;
    strcpy(hs_e->name, "segoh");
    hs_e++;
  }

  return hs;
}

/**************************************************************************************************
 *
 * fput_HighScoreData
 *
 * writes highscore entries to file
 *
 **************************************************************************************************/
static void fput_HighScoreData(HighScoreEntry *hs_e, FILE *fp) {
  int n;
  for (n = 0; n < MAX_HIGHSCORE_ENTRIES; n++) {
    fprintf(fp, "%i %s\n", hs_e->score, hs_e->name);
    hs_e++;
  }
  fputs("\n", fp);
}

/**************************************************************************************************
 *
 * writeHighScore
 *
 * writes highscore to file
 * return value
 *     1 if successful
 *     0 if error occurred
 *
 **************************************************************************************************/
int writeHighScore(HighScore hs) {

  FILE *fp_HighScore;

  if ((fp_HighScore = fopen(HIGHSCORE_FILE, "w")) != NULL) {

    /* write header */
    fprintf(fp_HighScore, "%s\nv%s\n\n", HIGHSCORE_ID, HIGHSCORE_VERSION);

    /* write data */
    fputs("beginner\n", fp_HighScore);
    fput_HighScoreData(hs.beginner, fp_HighScore);
    fputs("normal\n", fp_HighScore);
    fput_HighScoreData(hs.normal, fp_HighScore);
    fputs("expert\n", fp_HighScore);
    fput_HighScoreData(hs.expert, fp_HighScore);

    /* close file */
    fclose(fp_HighScore);

  } else {

    puts("(EE) writeHighScore: cannot open highscore file for writing");
    return 0;
  }

  return 1;
}

/**************************************************************************************************
 *
 * fget_HighScoreData
 *
 * reads highscore data from file
 *
 **************************************************************************************************/
static void fget_HighScoreData(HighScoreEntry *hs_e, FILE *fp) {
  int n;
  for (n = 0; n < MAX_HIGHSCORE_ENTRIES; n++) {
    fscanf(fp, "%i %s\n", &hs_e->score, hs_e->name);
    hs_e++;
  }
  fscanf(fp, "\n");
}

/**************************************************************************************************
 *
 * readHighScore
 *
 * reads highscore from file
 *
 **************************************************************************************************/
HighScore readHighScore() {

  FILE *fp_HighScore;
  HighScore hs;
  char hs_id[sizeof(HIGHSCORE_ID)];
  char hs_version[sizeof(HIGHSCORE_VERSION)];

  HighScore standard = getStandardHighScore();

  if ((fp_HighScore = fopen(HIGHSCORE_FILE, "r")) != NULL) {

    /* read header */
    fscanf(fp_HighScore, "%[^\n]\nv%[^\n]\n\n", hs_id, hs_version);

    if (strcmp(hs_id, HIGHSCORE_ID) != 0) {
      puts("(EE) readHighScore: highscore file has wrong header");
      fclose(fp_HighScore);
      return standard;
    }

    if (strcmp(hs_version, HIGHSCORE_VERSION) != 0) {
      puts("(EE) readHighScore: highscore file has wrong version number");
      fclose(fp_HighScore);
      return standard;
    }

    /* read data */
    fscanf(fp_HighScore, "beginner\n");
    fget_HighScoreData(hs.beginner, fp_HighScore);
    fscanf(fp_HighScore, "normal\n");
    fget_HighScoreData(hs.normal, fp_HighScore);
    fscanf(fp_HighScore, "expert\n");
    fget_HighScoreData(hs.expert, fp_HighScore);

    /* close file */
    fclose(fp_HighScore);

  } else {

    puts("(EE) readHighScore: cannot open highscore file for reading");
    return standard;
  }

  return hs;
}

/**
 * adds entry to highscore
 */
void addEntry(char *name, int score, int hsType) {
  int i = 0;
  int j;

  if (hsType == 0) {

    while (highscore.beginner[i].score >= score) {
      i++;
    }

    for (j = MAX_HIGHSCORE_ENTRIES - 1; j > i; j--) {
      highscore.beginner[j] = highscore.beginner[j - 1];
    }

    highscore.beginner[i].score = score;
    strcpy(highscore.beginner[i].name, name);

  } else if (hsType == 1) {

    while (highscore.normal[i].score >= score) {
      i++;
    }

    for (j = MAX_HIGHSCORE_ENTRIES - 1; j > i; j--) {
      highscore.normal[j] = highscore.normal[j - 1];
    }

    highscore.normal[i].score = score;
    strcpy(highscore.normal[i].name, name);

  } else if (hsType == 2) {

    while (highscore.expert[i].score >= score) {
      i++;
    }

    for (j = MAX_HIGHSCORE_ENTRIES - 1; j > i; j--) {
      highscore.expert[j] = highscore.expert[j - 1];
    }

    highscore.expert[i].score = score;
    strcpy(highscore.expert[i].name, name);
  }
}

/* void displayHighScore(HighScore hs){ */

/* 	int n; */

/* 	puts (hs.identifier); */
/* 	printf (" v. "); */
/* 	puts (hs.version); */

/* 	puts ("beginner"); */
/* 	for (n=0; n<MAX_NUMBER_HIGHSCORE_ENTRIES; n++) { */
/* 		printf ("  %2.7d ", hs.beginner[n].score); */
/* 		puts (hs.beginner[n].name); */
/* 	} */
/* 	puts ("normal"); */
/* 	for (n=0; n<MAX_NUMBER_HIGHSCORE_ENTRIES; n++) { */
/* 		printf ("  %2.7d ", hs.normal[n].score); */
/* 		puts (hs.normal[n].name); */
/* 	} */
/* 	puts ("expert");	 */
/* 	for (n=0; n<MAX_NUMBER_HIGHSCORE_ENTRIES; n++) { */
/* 		printf ("  %2.7d ", hs.expert[n].score); */
/* 		puts (hs.expert[n].name); */
/* 	} */

/* } */

/* int main(int argc, char **argv) */
/* { */
/* 	HighScore hs; */
/* 	HighScore hsRead; */

/* 	hs = getStandardHighScore(); */
/* 	writeHighScore(hs); */

/* 	hsRead = readHighScore(); */
/* 	displayHighScore (hsRead); */

/* 	return 0; */
/* } */
