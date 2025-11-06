/*-----------------------------------------------
Program: Food Web Analyzer
    Builds and analyzes a predator prey "food web".
    Supports interactive expansion, supplementation and extinction,
    and reports apex predators, producers, "tastiest food", heights,
    and vore types. Designed to be memory safe and readable.

Course: CS 211, Fall 2025, UIC
Author: Eldin Nokic
-------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/*
Struct: Org
Purpose:            
    Represents a species in the food web.
Fields:
    name - fixed size name
    prey - dynamic array of indices of organisms this org eats
    numprey - number of valid entries in prey[]
*/

typedef struct Org_struct {
    char name[20];
    int* prey; //dynamic array of indices  
    int numPrey;
} Org;


/*
Function: addOrgToWeb
Purpose: 
    Appends a new organism to the heap-allocated web array
Parameters: 
    pWeb - address of the Org* array pointer
    pNumOrgs - address of the current size
    newName - C string with the new organism's name
Returns:
    void
*/
void addOrgToWeb(Org** pWeb, int* pNumOrgs, char* newOrgName) {
    
    int oldSize = *pNumOrgs;
    int newSize = oldSize + 1;

    Org* newArray = (Org*)malloc(newSize* sizeof(Org));

    for (int i = 0; i < oldSize; i++) {
        newArray[i] = (*pWeb)[i];
    }
    strcpy(newArray[oldSize].name, newOrgName);
    newArray[oldSize].prey = NULL;
    newArray[oldSize].numPrey = 0;

    if (*pWeb != NULL) {
        free(*pWeb);
    }
    *pWeb = newArray;
    *pNumOrgs = newSize;
}

/*
Function addRelationToWeb
Purpose: 
    Add a predator->prey edge, avoiding duplicated and invalid pairs
Parameters:
    web - array of Org
    numOrgs - number of organisms
    predInd - predator index
    preyInd - prey index
Returns: 
    true if relation is added, falee if its invalid or a duplicate
*/
bool addRelationToWeb(Org* web, int numOrgs, int predInd, int preyInd) {
    if ((predInd < 0) || (predInd > numOrgs - 1) || (preyInd < 0) || (preyInd > numOrgs - 1) || (predInd == preyInd)) {
        printf("Invalid predator and/or prey index. No relation added to the food web.\n");
        return false;
    }
    
    Org* pred = &web[predInd];

    for (int i = 0; i < pred->numPrey; i++) {
        if (pred->prey[i] == preyInd) {
            printf("Duplicate predator/prey relation. No relation added to the food web.\n");
            return false;
        }
    }

    int n = pred->numPrey;
    int* newArray = (int*)malloc((n+1)*sizeof(int));

    for (int i = 0; i < n; i++) {
        newArray[i] = pred->prey[i];
    }
    newArray[n] = preyInd;

    if (pred->prey != NULL) {
        free(pred->prey);
    }

    pred->prey = newArray;
    pred->numPrey = n + 1;

    return true;

}

/*
Function: removeOrgFromWeb
Purpose: 
    Remove an organism by index, free its prey[], shrink web and update
    all remaining prey indices
Parameters:
    pWeb - address of Org* array
    pNumOrgs - address of size
    index - extinction index
Returns: 
    true if removed, false if invalid or nothing to remove
*/
bool removeOrgFromWeb(Org** pWeb, int* pNumOrgs, int index) {

    if (*pWeb == NULL || *pNumOrgs == 0) {
        return false;
    }

    if (index < 0 || index >= *pNumOrgs) {
        printf("Invalid extinction index. No organism removed from the food web.\n");
        return false;
    }

    Org* web = *pWeb;
    int n = *pNumOrgs;

    if (n == 1) {
        if (web[0].prey != NULL) {
            free(web[0].prey);
        }
        free(web);
        *pWeb = NULL;
        *pNumOrgs = 0;
        return true;
    }

    if (web[index].prey != NULL) {
            free(web[index].prey);
            web[index].prey = NULL;
            web[index].numPrey = 0;
        }
    
    Org* newWeb = (Org*)malloc(sizeof(Org) * (n-1));
    if (newWeb == NULL) {
        return false;
    }

    for (int i = 0; i < index; i++) {
        newWeb[i] = web[i];
    }
    for (int i = index + 1; i < n; i++) {
        newWeb[i - 1] = web[i];
    }

    free(web);

    *pWeb = newWeb;
    *pNumOrgs = n - 1;
    web = *pWeb;
    n = *pNumOrgs;

    for (int i = 0; i < n; i++) {
        int oldCount = web[i].numPrey;
        int* oldArr = web[i].prey;

        if (!oldCount || oldArr == NULL) {
            continue;
        }

        bool extinct = false;
        bool greater = false;

        for (int j = 0; j < oldCount; j++) {
            if (oldArr[j] == index) {
                extinct = true;
            }
            else if (oldArr[j] > index) {
                greater = true;
            }
        }

        if (!extinct && greater) {
            for (int j = 0; j < oldCount; j++){
                if (oldArr[j] > index){
                    --oldArr[j];
                }
            }
            continue;
        }

        if (!extinct && !greater) {
            continue;
        }

        int newCount;
        if (extinct) {
            newCount = oldCount - 1;
        }
        else {
            newCount = oldCount;
        }

        if (newCount == 0) {
            free(oldArr);
            web[i].prey = NULL;
            web[i].numPrey = 0;
            continue;
        }

        int* newArr = (int*)malloc(sizeof(int)*newCount);
        if (newArr == NULL) {
            continue;
        }

        int c = 0;
        for (int j = 0; j < oldCount; j++) {
            int ind = oldArr[j];
            if (ind == index) {
                continue;
            }
            if (ind > index) {
                ind -= 1;
            }
            newArr[c++] = ind;
        }
        free(oldArr);
        web[i].prey = newArr;
        web[i].numPrey = newCount;
    }
    return true;
}

/*
Function: freeWeb
Purpose: 
    Free all prey arrays and then the web array itself
Parameters:
    web - array
    numOrgs - length
Returns:
    void
*/
void freeWeb(Org* web, int numOrgs) {
    if (web == NULL) {
        return;
    }
    for (int i = 0; i < numOrgs; i++) {
        if (web[i].prey != NULL) {
            free(web[i].prey);
        }
    }
    free(web);
}

/*
Function: printWeb
Purpose: 
    Human readable printout of the food web
Parameters:
    web, numOrgs
Returns: 
    void
*/
void printWeb(Org* web, int numOrgs) {
    for (int i = 0; i < numOrgs; i++) {
        printf("  (%d) %s", i, web[i].name);
        if (web[i].numPrey > 0) {
            printf(" eats ");
            for (int j = 0; j < web[i].numPrey; j++) {
                int preyInd = web[i].prey[j];
                printf("%s", web[preyInd].name);
                if (j < web[i].numPrey - 1) {
                    printf(", ");
                }
            }
        }
        printf("\n");
    }
    printf("\n");
}

/*
Function: eatenByCounts
Purpose: 
    Count, for each organism, how many predators eat it.
Parameters:
    web, numOrgs
Returns:
    an int array of size numOrgs, caller frees it
*/
int* eatenByCounts(Org* web, int numOrgs) {

    int* eatenBy = (int*)malloc(sizeof(int)*numOrgs);

    for (int i = 0; i < numOrgs; i++) {
        eatenBy[i] = 0;
    }

    for (int i = 0; i < numOrgs; i++) {
        for (int j = 0; j < web[i].numPrey; j++) {
            int preyInd = web[i].prey[j];
            eatenBy[preyInd]++;
        }
    }
    return eatenBy;
}

/*
Function: calculateHeights
Purpose: 
    Layer height = 0 for producers, otherwise 1 + max(prey heights)
    Iteratively relax heights until covnergence
Parameters:
    web, numOrgs
Returns:
    an int array of size numOrgs, caller frees it
*/
int* calculateHeights(Org* web, int numOrgs) {
    int* height = (int*)malloc(sizeof(int)*numOrgs);

    for (int i = 0; i < numOrgs; i++) {
        height[i] = 0;
    }

    int change = 1;
    while (change) {
        change = 0;
        for (int i = 0; i < numOrgs; i++) {
            int newHeight = 0;
            if (web[i].numPrey > 0) {
                int maxPreyHeight = 0;
                for (int j = 0; j < web[i].numPrey; j++) {
                    int preyInd = web[i].prey[j];
                    if (height[preyInd] > maxPreyHeight) {
                        maxPreyHeight = height[preyInd];
                    }
                }
                newHeight = maxPreyHeight + 1;
            }
            if (newHeight != height[i]) {
                height[i] = newHeight;
                change = 1;
            }
        }
    } 
    return height;
}

/*
Function: printHeights
Purpose:
    Print each organism's height 
Parameters:
    web, numOrgs, height
Returns: 
    void
*/
void printHeights(Org* web, int numOrgs, int* height) {
    for (int i = 0; i < numOrgs; i++) {
        printf("  %s: %d\n", web[i].name, height[i]);
    }
}

/*
Function: printVoreTypes
Purpose: 
    Classify and print producers, Herbivores, Omnivores, Carnivores
Parameters:
    web, numOrgs
Returns: 
    void
*/
void printVoreTypes(Org* web, int numOrgs) {
    int* producer = (int*)malloc(sizeof(int)*numOrgs);
    for (int i = 0; i < numOrgs; i++) {
        if (web[i].numPrey == 0) {
            producer[i] = 1;
        }
        else {
            producer[i] = 0;
        }
    }
    printf("  Producers:\n");
    for (int i = 0; i < numOrgs; i++) {
        if (producer[i]) {
            printf("    %s\n", web[i].name);
        }
    }

    printf("  Herbivores:\n");
    for (int i = 0; i < numOrgs; i++) {
        if (producer[i]) {
            continue;
        }
        int eatsP = 0, eatsNP = 0;
        for (int j = 0; j < web[i].numPrey; j++) {
            int preyInd = web[i].prey[j];
            if (producer[preyInd]) {
                eatsP = 1;
            }
            else {
                eatsNP = 1;
            }
        }
        if (eatsP && !eatsNP) {
            printf("    %s\n", web[i].name);
        }
    }

    printf("  Omnivores:\n"); 
    for (int i = 0; i < numOrgs; i++) {
        if (producer[i]) {
            continue;
        }
        int eatsP = 0, eatsNP = 0;
        for (int j = 0; j < web[i].numPrey; j++) {
            int preyInd = web[i].prey[j];
            if (producer[preyInd]) {
                eatsP = 1;
            }
            else {
                eatsNP = 1;
            }
        }
        if (eatsP && eatsNP) {
            printf("    %s\n", web[i].name);
        }
    }

    printf("  Carnivores:\n");
    for (int i = 0; i < numOrgs; i++) {
        if (producer[i]) {
            continue;
        }
        int eatsP = 0, eatsNP = 0;
        for (int j = 0; j < web[i].numPrey; j++) {
            int preyInd = web[i].prey[j];
            if (producer[preyInd]) {
                eatsP = 1;
            }
            else {
                eatsNP = 1;
            }
        }
        if (!eatsP && eatsNP) {
            printf("    %s\n", web[i].name);
        }
    }
    printf("\n");
    free(producer);
}

/*
Function: displayAll
Purpose: 
    Print all analyses (web, apex, producers, flexible eaters, 
    tastiest food, heights, and vore types.) Adds UPDATES prefix when modified = true
Parameters:
    web, numOrgs, modified
Returns: 
    void
*/
void displayAll(Org* web, int numOrgs, bool modified) {

    if (modified) printf("UPDATED ");
    printf("Food Web Predators & Prey:\n");
    printWeb(web,numOrgs); 

    int* eatenBy = eatenByCounts(web, numOrgs);

    if (modified) printf("UPDATED ");
    printf("Apex Predators:\n");
    for (int i = 0; i < numOrgs; i++) {
        if (eatenBy[i] == 0) {
            printf("  %s\n", web[i].name);
        }
    }
    printf("\n");

    if (modified) printf("UPDATED ");
    printf("Producers:\n");
    for (int i = 0; i < numOrgs; i++) {
        if (web[i].numPrey == 0) {
            printf("  %s\n", web[i].name);
        }
    }
    printf("\n");

    if (modified) printf("UPDATED ");
    printf("Most Flexible Eaters:\n");
    int maxPrey = 0;
    for (int i = 0; i < numOrgs; i++) {
        if (web[i].numPrey > maxPrey) {
            maxPrey = web[i].numPrey;
        }
    }
    for (int i = 0; i < numOrgs; i++) {
        if (web[i].numPrey == maxPrey) {
            printf("  %s\n", web[i].name);
        }
    }
    printf("\n");

    if (modified) printf("UPDATED ");
    printf("Tastiest Food:\n");
    int maxEaten = 0;
    for (int i = 0; i < numOrgs; i++) {
        if (eatenBy[i] > maxEaten) {
            maxEaten = eatenBy[i];
        }
    }
    for (int i = 0; i < numOrgs; i++) {
        if (eatenBy[i] == maxEaten) {
            printf("  %s\n", web[i].name);
        }
    }
    printf("\n");
    free(eatenBy);

    if (modified) printf("UPDATED ");
    printf("Food Web Heights:\n");
    int* heights = calculateHeights(web, numOrgs);
    printHeights(web, numOrgs, heights);
    free(heights);
    printf("\n");

    if (modified) printf("UPDATED ");
    printf("Vore Types:\n");
    printVoreTypes(web, numOrgs);
}

/*
Funciton: setModes
Purpose: 
    parse -b, -d, -q flags from argv; each at most once
Parameters:
    argc, argv, pBasicMode, pDebugMode, pQuietMode
Returns: 
    true on success, false if any invalid/duplicate flag.
*/
bool setModes(int argc, char* argv[], bool* pBasicMode, bool* pDebugMode, bool* pQuietMode) {

    int bCount = 0, dCount = 0, qCount = 0;

    *pDebugMode = false;
    *pQuietMode = false;
    *pBasicMode = false;

    for (int i = 1; i < argc; i++) {

        char* s = argv[i];

        if (strncmp(s,"-d",2) == 0) {
            if (dCount == 1) {
                return false;
            }
            else {
                dCount++;
                *pDebugMode = true;
            }
        }
        else if (strncmp(s,"-b",2) == 0) {
            if (bCount == 1) {
                return false;
            }
            else {
                bCount++;
                *pBasicMode = true;
            }
        }
        else if (strncmp(s,"-q",2) == 0) {
            if (qCount == 1) {
                return false;
            }
            else {
                qCount++;
                *pQuietMode = true;
            }
        }
        else {
            return false;
        }
    }
    
    return true; 
}

void printONorOFF(bool mode) {
    if (mode) {
        printf("ON\n");
    } else {
        printf("OFF\n");
    }
}

/*
Function: main
Purpose: 
    Orchhestrates program flow: settings, initial build,
    relations input, reports, and interactive modifications.
Parameters:
    argc, argv
Returns:
    0 on sucess, 1 on CLI parse error
*/
int main(int argc, char* argv[]) {  
    
    // Initializes all modes to false
    bool basicMode = false; // -b (read only)
    bool debugMode = false; // -d print extra after each change
    bool quietMode = false; // -q suppress user prompts (program output is still printed)

    // Parse -b/-d/-q at most once each
    if (!setModes(argc, argv, &basicMode, &debugMode, &quietMode)) { // If input is invalid (Duplicates or invalid arguments, no flags is still valid)
        printf("Invalid command-line argument. Terminating program...\n");
        return 1;
    }

    // Prints which modes are ON/OFF
    printf("Program Settings:\n");
    printf("  basic mode = ");
    printONorOFF(basicMode); // Function that prints ON or OFF
    printf("  debug mode = ");
    printONorOFF(debugMode);
    printf("  quiet mode = ");
    printONorOFF(quietMode);
    printf("\n");

    // Intial state for the food web
    int numOrgs = 0; // Initializes number of organisms to 0
    printf("Welcome to the Food Web Application\n\n");
    printf("--------------------------------\n\n");

    Org* web = NULL; // Initializes the web array to a null pointer

    printf("Building the initial food web...\n");
    
    if (!quietMode) printf("Enter the name for an organism in the web (or enter DONE): "); // Only prints when not in quiet mode
    char tempName[20] = ""; // Declares and initializes name with an empty string
    scanf("%s",tempName); // Scans name into tempName
    if (!quietMode) printf("\n");
    while (strcmp(tempName,"DONE") != 0) { // Runs loops until user enters DONE
        addOrgToWeb(&web,&numOrgs,tempName); // Calls function that will add the organism to the web
        if (debugMode) { // Prints some extra info after each change 
            printf("DEBUG MODE - added an organism:\n");
            printWeb(web,numOrgs);
            printf("\n");
        }
        if (!quietMode) printf("Enter the name for an organism in the web (or enter DONE): ");
        scanf("%s",tempName); 
        if (!quietMode) printf("\n");
    }
    if (!quietMode) printf("\n");

    // Asks the user to enter the needed info about predator/prey relations. 
    // Read predator/prey index pairs, loops continues only while the pair is valid and pred!=prey
    if (!quietMode) printf("Enter the pair of indices for a predator/prey relation.\n");
    if (!quietMode) printf("Enter any invalid index when done (-1 2, 0 -9, 3 3, etc.).\n");
    if (!quietMode) printf("The format is <predator index> <prey index>: ");
        
    int predInd, preyInd; // Declares the indices for predator and prey
    scanf("%d %d",&predInd, &preyInd); // Reads in users indices for predator and prey
    if (!quietMode) printf("\n");

    while (predInd >= 0 && preyInd >= 0 && predInd < numOrgs &&  preyInd < numOrgs && predInd != preyInd) { // Loops while the indices are valid
        addRelationToWeb(web,numOrgs,predInd,preyInd); // Calls function that will add the new relation to the web
        if (debugMode) { // Prints extra info if in debug mode
            printf("DEBUG MODE - added a relation:\n");
            printWeb(web,numOrgs);
            printf("\n");
        }
        if (!quietMode) printf("Enter the pair of indices for a predator/prey relation.\n");
        if (!quietMode) printf("Enter any invalid index when done (-1 2, 0 -9, 3 3, etc.).\n");
        if (!quietMode) printf("The format is <predator index> <prey index>: ");
        
        scanf("%d %d",&predInd, &preyInd);  
        if (!quietMode) printf("\n");
    }
    printf("\n");

    printf("--------------------------------\n\n");
    printf("Initial food web complete.\n");
    printf("Displaying characteristics for the initial food web...\n");
    
    displayAll(web,numOrgs,false); // Calls function that displays all the analyses of the food web

    // Interactive modification menu
    if (!basicMode) { // Only runs if the user is not in the basic mode (the user can modify the web)
        printf("--------------------------------\n\n");
        printf("Modifying the food web...\n\n");
        char opt = '?';

        while (opt != 'q') { // Runs loop until user enters q for quitting
            if (!quietMode) { 
                printf("Web modification options:\n");
                printf("   o = add a new organism (expansion)\n");
                printf("   r = add a new predator/prey relation (supplementation)\n");
                printf("   x = remove an organism (extinction)\n");
                printf("   p = print the updated food web\n");
                printf("   d = display ALL characteristics for the updated food web\n");
                printf("   q = quit\n");
                printf("Enter a character (o, r, x, p, d, or q): ");
            }
            scanf(" %c", &opt); // Reads users choice from the menu
            if (!quietMode) printf("\n\n");

            if (opt == 'o') { // Grows heap array web by 1, frees old array after copy and initializes new Org
                char newName[20];
                if (!quietMode) printf("EXPANSION - enter the name for the new organism: ");
                scanf("%s",newName); // Reads in the name of the new organism
                if (!quietMode) printf("\n");
                printf("Species Expansion: %s\n", newName);
                addOrgToWeb(&web,&numOrgs,newName); // Adds it to the web
                printf("\n");

                if (debugMode) {
                    printf("DEBUG MODE - added an organism:\n");
                    printWeb(web,numOrgs);
                    printf("\n");
                }

            } else if (opt == 'x') { // Removes organism if user enters 'x'
                int extInd; // Index of organism that will be removed
                if (!quietMode) printf("EXTINCTION - enter the index for the extinct organism: ");
                scanf("%d",&extInd); // Reads in the index of the organism that the user wants to be removed
                if (!quietMode) printf("\n");
                if (extInd >= 0 && extInd < numOrgs) { // Checks if index is valid
                    printf("Species Extinction: %s\n", web[extInd].name);
                    removeOrgFromWeb(&web,&numOrgs,extInd); // Calls the function that removes the organism from the web
                } else {
                    printf("Invalid index for species extinction\n"); // Printed if the index is invalid
                }
                printf("\n");
                
                if (debugMode) {
                    printf("DEBUG MODE - removed an organism:\n");
                    printWeb(web,numOrgs);
                    printf("\n");
                }

            } else if (opt == 'r') { // Adds a new pred/prey relation if user enters 'r'
                if (!quietMode) printf("SUPPLEMENTATION - enter the pair of indices for the new predator/prey relation.\n");
                if (!quietMode) printf("The format is <predator index> <prey index>: ");
                scanf("%d %d",&predInd, &preyInd); // Reads in indices of predator and prey
                if (!quietMode) printf("\n");

                if (addRelationToWeb(web,numOrgs,predInd,preyInd)) { // Checks if the new relation is valid and adds it to the web
                    printf("New Food Source: %s eats %s\n", web[predInd].name, web[preyInd].name); // Prints the new relation
                };
                printf("\n");
                if (debugMode) {
                    printf("DEBUG MODE - added a relation:\n");
                    printWeb(web,numOrgs);
                    printf("\n");
                }

            } else if (opt == 'p') { // Prints the updated food if user enters 'p', print only, no memory changes
                printf("UPDATED Food Web Predators & Prey:\n");
                printWeb(web,numOrgs); // Calls the function that prints the web
                printf("\n");
                
            } else if (opt == 'd') { // Displays all the characteristics for the updated food web if user enters 'd'
                // Full analysis
                printf("Displaying characteristics for the UPDATED food web...\n\n");
                displayAll(web,numOrgs,true); // Calls the function that displays it all

            }
            printf("--------------------------------\n\n");
        
        }
        
    }

    freeWeb(web,numOrgs); // Frees the memory that was allocated for the web and prey to avoid leaks



    return 0;
}

