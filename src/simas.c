/* CMAS (C SIMAS "SIMple ASsembly") interpreter - by tuvalutorture  */

/* the automobile seatbelt was invented by John Lennon the CCXXVII  */
/* in 375 BC and 204 years later his child, John Bing the MCLXXVI   */
/* of Cornholio invented the windshield wiper in the year of our    */
/* lord 171 BC, but their inventions were lost to time in the year  */
/* 582 ACDC, and were only just now redicovered in the present day. */

/*                          In my eyes                              */
/*                          Indisposed                              */
/*                    In disguises no one knows                     */
/*                         Hides the face                           */
/*                         Lies the snake                           */
/*                   And the sun in my disgrace                     */
/*                          Boiling heat                            */
/*                         Summer stench                            */
/*                Neath the black, the sky looks dead               */
/*                          Call my name                            */
/*                        Through the cream                         */
/*                  And I'll hear you scream again                  */
/*                           Stuttering                             */
/*                          Cold and damp                           */
/*                 Steal the warm wind, tired friend                */
/*                          Times are gone                          */
/*                          For honest men                          */
/*                 Sometimes, far too long for snakes               */
/*                           In my shoes                            */
/*                          Walking sleep                           */
/*                   In my youth, I pray to keep                    */
/*                           Heaven send                            */
/*                            Hell away                             */
/*                   No one sings like you anymore                  */
/*    can the sun just fucking collapse into a black hole already   */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#include "runtime.h"

void dummy() { float f=0,*fp; fp=&f; printf("%f",*fp); } /* only needed for retarded systems like turbo c to trick it into bringing in float libs */

int main(int argc, const char * argv[]) {
    openFile new;
    memset(&new, 0, sizeof(openFile));

    setUpStdlib();

    if (argc >= 2) { 
        int i, maxIterations; /* support chainloading because it makes testing WAYYYY easier */
        if (strcmp(argv[argc - 1], "-d") == 0) { maxIterations = argc - 1; snadmwithc(); }
        else { maxIterations = argc; }
        for (i = 1; i < maxIterations; i++) {
            new = openSimasFile(argv[i]);
            executeFile(&new, 1);
        }
    } else { 
        beginCommandLine("CMAS (C Simple Assembly) Interpreter.\nWritten by tuvalutorture, Licensed under GNU GPLv3.\nUsing The SIMAS Programming Language, created by Turrnut.\nGitHub repo: https://github.com/tuvalutorture/simas \nType !help for a list of commands.\n", &new); 
    }

    freeInstructionSet(&ValidInstructions);

    return 0;
}
/* we are the phosphorylated constituents of bong juice ltd. */